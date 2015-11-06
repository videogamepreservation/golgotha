/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "sound_man.hh"
#include "loaders/wav_load.hh"
#include "memory/malloc.hh"
#include "math/random.hh"
#include "objs/sfx_obj.hh"
#include "music/stream.hh"
#include "file/file.hh"
#include "time/profile.hh"
#include "string/string.hh"
#include "sound/sfx_id.hh"
#include "lisp/lisp.hh"
#include "lisp/li_init.hh"
#include "lisp/li_dialog.hh"
#include "lisp/li_class.hh"
#include "status/status.hh"

g1_sound_manager_class g1_sound_man;

i4_stream_wav_player *g1_music_stream=0;
static int music_count_down=0;


g1_sound_manager_class::g1_sound_manager_class() 
{
  sfx_obj_list=0; 
  loop_current_song=i4_F;
}

void g1_sound_manager_class::next_song()
{
  if (g1_music_stream)
  {
    delete g1_music_stream;
    g1_music_stream=0;
  }
}

void g1_sound_manager_class::pause()
{
  for (g1_sfx_obj_class *s=sfx_obj_list; s; s=s->next_sfx)
    if (s->stream)
      s->stream->pause();

  if (g1_music_stream)
    g1_music_stream->pause();
}

void g1_sound_manager_class::unpause()
{
  for (g1_sfx_obj_class *s=sfx_obj_list; s; s=s->next_sfx)
    if (s->stream)
      s->stream->unpause();

  if (g1_music_stream)
    g1_music_stream->unpause();
}

inline float dist_comp(i4_3d_vector &a, i4_3d_vector &b)
{
  return (((a.x-b.x)*(a.x-b.x)) + ((a.y-b.y)*(a.y-b.y)) + ((a.z-b.z)*(a.z-b.z)));
}

static int get_music_volume()
{
  li_object *music=li_get_value("music");
  if (music)
  {
    li_class *c=li_class::get(music,0);
    return li_get_int(c->value("volume"),0);
  }
  else return 0;
}

static void set_music_volume(int vol)
{
  
  li_object *music=li_get_value("music");
  if (music)
  {
    li_class *c=li_class::get(music,0);

    if (vol>I4_SOUND_VOLUME_LEVELS-1)
      vol=I4_SOUND_VOLUME_LEVELS-1;
    if (vol<0) vol=0;

    if (g1_music_stream)
      g1_music_stream->set_volume(vol);

    c->set("volume", new li_int(vol));
  }
}



static void restart_song(char *newfn)
{
  if (g1_music_stream)
  {
    delete g1_music_stream;
    g1_music_stream=0;
  }
  
  char fn[100];
  sprintf(fn, "%s/%s", s1_get_sfx_path(), newfn);

  if (i4_sound_man==&i4_null_sound)
    music_count_down=100;
  else
  {

    i4_file_class *fp=i4_open(fn, I4_SUPPORT_ASYNC | I4_READ);
    if (fp)
    {    
      g1_music_stream=new i4_stream_wav_player(fp, 256*1024, i4_F, i4_T);
      if (g1_music_stream->load_failed())
      {
        delete g1_music_stream;
        g1_music_stream=0;
      }
      else 
        set_music_volume(get_music_volume());

    }
    else 
      music_count_down=100;  // wait 10 seconds before trying again
  }
}



li_object *g1_set_song(li_object *o, li_environment *env)
{
  li_class *oldv=li_class::get(li_second(o,env),env);
  li_class *newv=li_class::get(li_first(o,env),env);

  char *oldfn=li_get_string(oldv->value("songs"),env);
  char *newfn=li_get_string(newv->value("songs"),env);


  if (strcmp(oldfn,newfn))
    restart_song(newfn);
      

  if (g1_music_stream)
  {
    int vol=li_get_int(newv->value("volume"),env);
    g1_music_stream->set_volume(vol);
  }


  return 0;
}

li_object *g1_edit_music(li_object *o, li_environment *env)
{
  li_create_dialog("Music",
                   li_get_value("music"),
                   0,
                   g1_set_song);
  return 0;
}

li_object *g1_music_up(li_object *o, li_environment *env)
{
  set_music_volume(get_music_volume()+8);
  return 0;
}


li_object *g1_music_down(li_object *o, li_environment *env)
{
  set_music_volume(get_music_volume()-8);
  return 0;
}

li_automatic_add_function(g1_edit_music, "edit_music");
li_automatic_add_function(g1_music_up, "music_up");
li_automatic_add_function(g1_music_down, "music_down");

// should be calle by main game every tick, check to see if a new narative should be played
void g1_sound_manager_class::poll(i4_bool game_is_running)
{  
  i4_3d_vector listener;
  s1_get_camera_pos(listener);
  for (g1_sfx_obj_class *sfx=sfx_obj_list; sfx; sfx=sfx->next_sfx)
    sfx->dist_from_camera_sqrd=(sfx->x - listener.x) * (sfx->x - listener.x) +
                               (sfx->y - listener.y) * (sfx->y - listener.y) +
                               (sfx->h - listener.z) * (sfx->h - listener.z);


  if (game_is_running)
  {    
    if (!g1_music_stream)  // no more music, go to the next song
    {
      if (music_count_down)  // this prevents trying to open a missing music file quickly
        music_count_down--;
      else
      {
        li_object *music=li_get_value("music");
        if (music)
        {
          li_class *c=li_class::get(music,0);
          li_object *list=li_cdr(li_class_get_property_list(c->type(), li_get_symbol("songs")),0), *o;
          li_object *cur_song=c->value("songs");

          for (o=list; o && li_car(o,0)!=cur_song; o=li_cdr(o,0));

          if (o && !loop_current_song)
            o=li_cdr(o,0);
        
          if (!o)
            o=list;
    
          if (o)    
          {
            cur_song=li_car(o,0);
            c->set("songs", cur_song);
          }
          else 
            cur_song=0;

          if (cur_song)
            restart_song(li_get_string(cur_song,0));

        }
      }
    }

    if (g1_music_stream)
      if (!g1_music_stream->poll())
      {
        delete g1_music_stream;
        g1_music_stream=0;
      }

    for (g1_sfx_obj_class *s=sfx_obj_list; s; s=s->next_sfx)
    {
      if (s->stream)
      {
        float d=s->max_hearable_distance;
        if (s->dist_from_camera_sqrd>d*d || !s->stream->poll())
        {
          delete s->stream;
          s->stream=0;
          s->current_delay=s->restart_delay;
          if (s->random_restart_delay)
            s->current_delay+=(i4_rand()%s->random_restart_delay);          
        }
      }

      if (!s->stream)
      {
        if (s->current_delay)
          s->current_delay--;
        else if (s->dist_from_camera_sqrd<
                 s->max_hearable_distance*s->max_hearable_distance && s->filename)
        {
          if (i4_sound_man!=&i4_null_sound)
          {
            i4_file_class *fp=i4_open(*s->filename, I4_READ | I4_SUPPORT_ASYNC | I4_NO_BUFFER);
            if (!fp)
            {
              i4_filename_struct fs;
              i4_split_path(*s->filename, fs);

              char fn[256];
              sprintf(fn, "%s/ambient/%s.%s", s1_get_sfx_path(), fs.filename, fs.extension);
              fp=i4_open(fn, I4_READ | I4_SUPPORT_ASYNC | I4_NO_BUFFER);
            }

            if (fp)
            {
              i4_bool loop=i4_F;
              if (s->restart_delay==0 && s->random_restart_delay==0)
                loop=i4_T;

              s->stream=new i4_stream_wav_player(fp, 64*1024, loop, i4_T);
              if (s->stream->load_failed())
              {
                delete s->stream;
                s->stream=0;
                s->current_delay=10000;
              }
            }
            else s->current_delay=10000;
          }
        }
      }

      if (s->stream)
      {
        float d=sqrt(s->dist_from_camera_sqrd);
        i4_float volume_scale = 1.f - (d / (float)s->max_hearable_distance);

        sw32 vol = i4_f_to_i(volume_scale * (float)s->max_volume);
        
        s->stream->set_volume(vol);
      }

    }
  } 
}

void g1_sound_manager_class::add_sfx_to_list(g1_sfx_obj_class *sfx)
{
  sfx->next_sfx=sfx_obj_list;
  sfx_obj_list=sfx;
}

void g1_sound_manager_class::remove_sfx_from_list(g1_sfx_obj_class *sfx)
{
  g1_sfx_obj_class *last=0, *p;
  for (p=sfx_obj_list; p && p!=sfx;)
  {
    last=p;
    p=p->next_sfx;
  }
  
  I4_ASSERT(p, "remove_sfx_from_list : not in list");
  if (last)
    last->next_sfx=p->next_sfx;
  else sfx_obj_list=p->next_sfx;
}



li_object *g1_list_sfx(li_object *o, li_environment *env)
{
  i4_file_class *fp=i4_open("sfx_list.txt", I4_WRITE);
  if (fp)
  {
    s1_save_sfx_list(fp);

    for (g1_sfx_obj_class *s=g1_sound_man.sfx_obj_list; s; s=s->next_sfx)
    {
      char fname[256];
      i4_os_string(*s->filename, fname, 256);
    
      fp->printf("filename '%s' : location (%f,%f,%f), global_id=%d\n", 
                 fname, s->x, s->y, s->h, s->global_id);

    }
    delete fp;
  }

  return 0;
}



class g1_sound_man_updater : public i4_idle_class
{  
public:
  virtual void idle() 
  {
    if (g1_music_stream)
      g1_sound_man.poll(i4_T);             // update sound effects (play next narative sfx)
  }


} g1_sound_man_updater_instance;



static li_object *next_song(li_object *o, li_environment *env)
{
  g1_sound_man.next_song();
  return 0;
}


li_automatic_add_function(g1_list_sfx, "list_sfx");
li_automatic_add_function(next_song, "next_song");
