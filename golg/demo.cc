/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "file/file.hh"
#include "tick_count.hh"
#include "lisp/lisp.hh"
#include "lisp/li_init.hh"
#include "g1_object.hh"
#include "objs/stank.hh"
#include "player.hh"
#include "objs/field_camera.hh"
#include "human.hh"
#include "saver.hh"
#include "map.hh"
#include "map_man.hh"
#include "level_load.hh"
#include "app/app.hh"
#include "g1_speed.hh"
#include "file/sub_section.hh"

/*

example script:
'((action_mode)   300

  (strategy_mode)
  (set_camera_pos x y) 10

  (action_mode)   
  (wait_till_near_camera "main_base" 10)

  (camera_mode)
  (set_current_camera "main_base1")

  (pan_to_camera "main_base2")
  




(wait 1000)
(

commands

(action_mode)

(strategy_mode)    ; overhead view
(set_camera_pos x y) ; should be called when in stretgy mode to the position in world space

(strategy_toggle)  ; radar map toggle
(wait_till_near_camera "camera_name" min_distance)
(set_current_camera "camera_name")

 */

static int demo_last_tick=0;
static i4_file_class *record_file=0;
static int wait_ticks=0;
static float camera_dist=0;


static void flush_time()
{
  if (record_file && g1_tick_counter-demo_last_tick)
  {
    record_file->printf(" %d\n", g1_tick_counter-demo_last_tick);
    demo_last_tick=g1_tick_counter;
  }
}

static i4_bool copy_file(const i4_const_str &f1, const i4_const_str &f2)
{
  i4_file_class *in=i4_open(f1);
  if (!in)
    return i4_F;

  i4_file_class *out=i4_open(f2, I4_WRITE);
  if (!out)
  {
    delete in;
    return i4_F;
  }
  w8 buf[4096];
  int done=0;
  while (!done)
  {
    int s=in->read(buf, sizeof(buf));
    if (out->write(buf, s)!=s)
      return i4_F;

    if (s!=sizeof(buf))
      done=1;
  }
  delete in;
  delete out;
  return i4_T;
}

static int current_demo_num=0;

static li_object *record_toggle(li_object *o, li_environment *env)
{
  if (record_file)
  {
    flush_time();
    record_file->printf(")\n");
    delete record_file;
    record_file=0;
    g1_human->record_end();



    // find a free demo name we can save to
    current_demo_num=0;
    char demo_name[200], found=1;
    do
    {
      sprintf(demo_name, "demos/demo%02d.level", current_demo_num);

      i4_file_status_struct rs;
      if (i4_get_status(demo_name, rs))
        current_demo_num++;
      else
        found=0;

    } while (found);
    

    i4_file_class *out=i4_open(demo_name, I4_WRITE);
    i4_file_class *in=i4_open("demos/tmp_level.level");

    i4_file_class *sdata[3];
    sdata[0]=i4_open("demos/tmp_script.scm");
    sdata[1]=i4_open("demos/tmp_inputs.gdm");
    sdata[2]=i4_open("demos/tmp_resources.scm");
    
    char *snames[3]={"script", "inputs", "resources"};

    
    i4_insert_sections(in, out, 3, sdata, snames);
    delete in;
    delete out;
    delete sdata[0];
    delete sdata[1];
    delete sdata[2];

    i4_unlink("demos/tmp_script.scm");
    i4_unlink("demos/tmp_level.level");
    i4_unlink("demos/tmp_inputs.gdm");
    i4_unlink("demos/tmp_resources.scm");    
  }
  else if (g1_map_is_loaded())
  {
    i4_mkdir("demos");

    i4_str *res_file=g1_get_res_filnename(g1_get_map()->get_filename());
    if (!copy_file(*res_file, "demos/tmp_resources.scm"))
      return 0;

    i4_file_class *out=i4_open("demos/tmp_level.level", I4_WRITE);
    if (!out)
    {
      i4_warning("Couldn't save initial demo level state!");
      return 0;
    }


    g1_saver_class *save=new g1_saver_class(out);
    g1_get_map()->save(save, G1_MAP_ALL);
    if (save->begin_data_write())
      g1_get_map()->save(save, G1_MAP_ALL);
    delete save;
    g1_load_level("demos/tmp_level.level", 0);
    
    record_file=i4_open("demos/tmp_script.scm", I4_WRITE);

    record_file->printf("(play_script\n"
                        "  (action_mode)\n");
    g1_human->record_start("demos/tmp_inputs.gdm");

    demo_last_tick=g1_tick_counter;
  }

  return 0;
}

void g1_demo_script_add(char *command)
{
  if (record_file)
  {
    flush_time();
    record_file->printf("  %s", command);
  }
}



static li_object_pointer demo_script;
static g1_object_class *wait_camera=0;


static li_object *end_demo(li_object *o, li_environment *env)
{
  if (record_file)
    record_toggle(0,0);

  if (demo_script.get())
  {
    demo_script=0;
    g1_human->playback_end();
  }
  return 0;
}

static li_object *play_script(li_object *o, li_environment *env)
{
  demo_script=o;
  wait_ticks=0;
  camera_dist=0;
  wait_camera=0;
  return 0;
}


static li_object *load_level(li_object *o, li_environment *env)
{
  i4_bool load_res=li_get_bool(li_second(o,env),env);
  char *fn=li_get_string(li_first(o,env),env);

  if (!g1_load_level(fn, load_res, 0))
  {
    i4_warning("Couldn't load level file %s", fn);
    return 0;
  }

  return li_true_sym;
}


static li_object *playback_input(li_object *o, li_environment *env)
{  
  char *fn=li_get_string(li_first(o,env),env);
  i4_file_class *fp=i4_open(fn);
  if (!g1_human->playback_start(fp))
  {
    i4_warning("Couldn't load stream");
    return li_nil;
  }
  else
    return li_true_sym;
}

static li_object *play_demo(li_object *o, li_environment *env)
{
  if (o)
    current_demo_num=li_get_int(li_car(o,env),env);
 
  char demo_name[200];
  sprintf(demo_name, "demos/demo%02d.level", current_demo_num);

  if (g1_load_level(demo_name, 1))
  {
    i4_loader_class *lfp=i4_open_save_file(i4_open(demo_name));
    if (lfp)
    {
      w32 off, size;
      if (lfp->get_section_info("inputs", off, size))
        g1_human->playback_start(new i4_sub_section_file(i4_open(demo_name), off, size));


      if (lfp->get_section_info("script", off, size))
      {
        i4_file_class *fp=new i4_sub_section_file(i4_open(demo_name), off,size);
        li_load(fp);
        delete fp;
      }


      delete lfp;
    }
  }

  return 0;
}


i4_bool g1_playing_demo()
{
  if (!g1_map_is_loaded()) 
    return i4_F;

  if (wait_ticks || demo_script.get())
    return i4_T;
  else
    return i4_F;
}

void g1_demo_tick()
{
  if (wait_camera)
  {
    g1_object_class *c=g1_player_man.get_local()->get_commander();
    if (!c)
      wait_camera=0;
    else if ((c->x-wait_camera->x)*(c->x-wait_camera->x)+
        (c->y-wait_camera->y)*(c->y-wait_camera->y)+
        (c->h-wait_camera->h)*(c->h-wait_camera->h) < camera_dist*camera_dist)
      wait_camera=0;
  }             
  else if (wait_ticks)
    wait_ticks--;
  else
  {
    li_object *script_start=demo_script.get();
    li_object *script=script_start;

    while (!wait_camera && !wait_ticks && script)
    {
      
      li_object *o=li_car(script, 0);     
      if (o->type()==LI_INT)
        wait_ticks=li_get_int(o, 0);
      else
        li_eval(o);

      if (script_start!=demo_script.get())    // we loaded another script
        script=0;

      if (script)
         script=li_cdr(script, 0);
    }

    if (script_start==demo_script.get())
      demo_script=script;
  }
}

static li_object *wait_near(li_object *o, li_environment *env)
{
  wait_camera=g1_find_named_camera(li_get_string(li_eval(li_first(o,env),env),env));
  camera_dist=li_get_float(li_eval(li_second(o,env), env),env);

  return 0;
}

static li_object *quit_demo(li_object *o, li_environment *env)
{
  if (g1_playing_demo())
  {
    end_demo(0,0);
    i4_current_app->quit();
  }

  return 0;
}

static li_object *set_game_hz(li_object *o, li_environment *env)
{
  G1_HZ=li_get_int(li_car(o,env),env);
  return 0;
}


li_automatic_add_function(quit_demo, "quit_demo");
li_automatic_add_function(end_demo, "end_demo");
li_automatic_add_function(play_demo, "play_demo");
li_automatic_add_function(record_toggle, "record_toggle");
li_automatic_add_function(wait_near, "wait_till_near_camera");

li_automatic_add_function(set_game_hz, "set_game_hz");
li_automatic_add_function(load_level, "load_level");
li_automatic_add_function(playback_input, "playback_input");
li_automatic_add_function(play_script, "play_script");
