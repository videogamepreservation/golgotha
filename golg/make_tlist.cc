/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include <stdio.h>
#include "saver_id.hh"
#include "status/status.hh"

#include "lisp/lisp.hh"
#include "file/file.hh"
#include "memory/array.hh"
#include "loaders/dir_load.hh"
#include "error/alert.hh"


static void g1_add_to_list(i4_array<i4_str *> &t_arr, i4_str *tname)
{
  if (tname->null())
    i4_error("added null name");
      
  int found=0;
  for (int j=0; !found && j<t_arr.size(); j++)
    if ( (*t_arr[j])==(*tname))
      found=1;

  if (!found)
    t_arr.add(tname);
  else
    delete tname;  
}

static void g1_add_to_list(i4_array<i4_str *> &t_arr, 
                           li_object *fmt, li_object *name)
{
  char buf[200];
  sprintf(buf, li_string::get(fmt,0)->value(), li_string::get(name,0)->value());

  g1_add_to_list(t_arr, new i4_str(buf));
}



void g1_make_texture_list_for_model(const i4_const_str model_name,
                                    i4_array<i4_str *> &t_arr)
{
  i4_file_class *fp=i4_open(model_name);

  if (fp)
  {
    i4_loader_class *gfp=i4_open_save_file(fp);
    if (gfp)
    {
      if (gfp->goto_section(G1_SECTION_MODEL_TEXTURE_NAMES))
      {
        w16 quads=gfp->read_16();
        for (int i=0; i<quads; i++)
        {
          i4_str *tname=gfp->read_counted_str();
          if (tname->null())
          {
            i4_alert("Model %S has a face with no name!", 100, &model_name);
            delete tname;
          }
          else
            g1_add_to_list(t_arr, tname);
        }
      }
      delete gfp;
    }
  }
}

static i4_array<i4_str *> *g1_current_tnames=0;
static i4_array<i4_str *> *g1_current_model_names=0;
static int g1_current_t_tiles=0;


li_object *g1_get_models_texture(li_object *o, li_environment *env)
{
  li_object *fmt=li_eval(li_car(o,env),env);

  for (o=li_cdr(o,env); o; o=li_cdr(o,env))  // skip path at start
    g1_add_to_list(*g1_current_model_names, fmt, li_car(o,env));

  return 0;
}


li_object *g1_get_tile_textures(li_object *o, li_environment *env)
{
  li_object *fmt=li_eval(li_car(o,env),env);

  char buf[200];

  for (o=li_cdr(o,env); o; o=li_cdr(o,env))    // skip path at start
  {
    char *name;
    if (li_car(o,env)->type()==LI_STRING)
      name=li_string::get(li_eval(li_car(o,env),env),env)->value();
    else
      name=li_string::get(li_eval(li_car(li_car(o,env),env),env),env)->value(); 

    sprintf(buf, li_string::get(fmt,env)->value(), name);

    g1_current_tnames->add(new i4_str(buf));
    g1_current_t_tiles++;
  }


  return 0;
}


li_object *g1_sky_textures(li_object *o, li_environment *env)
{
  li_object *fmt=li_eval(li_car(o,env),env);

  char buf[200], name1[200], name2[200];

  for (o=li_cdr(o,env); o; o=li_cdr(o,env))    // skip path at start
  {
    char *name;
    if (li_car(o,env)->type()==LI_STRING)
      name=li_string::get(li_eval(li_car(o,env),env),env)->value();
    else
      name=li_string::get(li_eval(li_car(li_car(o,env),env),env),env)->value(); 

    sprintf(buf, li_string::get(fmt,env)->value(), name);
    sprintf(name1, "%s1", buf);
    sprintf(name2, "%s2", buf);

    g1_current_tnames->add(new i4_str(buf));
    g1_current_tnames->add(new i4_str(name1));
    g1_current_tnames->add(new i4_str(name2));
    g1_current_t_tiles++;
  }


  return 0;
}


static li_symbol *s_model_name=0, *s_mini_object=0;

li_object *g1_get_building_textures(li_object *o, li_environment *env)
{
  li_object *fmt=li_get_value("building_format", env);

  for (; o; o=li_cdr(o,env))
    g1_add_to_list(*g1_current_model_names, fmt, li_car(o,env));

  return 0;
}


li_object *g1_get_object_textures(li_object *o, li_environment *env)
{
  li_object *fmt=li_get_value("object_format", env);

  for (li_object *p=li_cdr(o,env); p; p=li_cdr(p,env)) 
  {
    li_symbol *sym=li_symbol::get(li_car(li_car(p,env),env),env);

    if (sym==li_get_symbol("model_name",s_model_name))
      g1_add_to_list(*g1_current_model_names, fmt, li_car(li_cdr(li_car(p,env),env),env));     
    else if (sym==li_get_symbol("mini_object",s_mini_object))
    {
      li_object *s=li_cdr(li_car(p,env),env);  // s = ("gunport_barrel")
      for (; s; s=li_cdr(s,env))
        if (li_symbol::get(li_car(li_car(s,env),env),env)==
            li_get_symbol("model_name", s_model_name))
          g1_add_to_list(*g1_current_model_names, fmt, li_car(li_cdr(li_car(s,env),env),env));
    }
  }

  return 0;
}


static li_object *g1_ignore(li_object *o, li_environment *env) {  return 0; }


static int defaults_set=0;



void g1_get_load_info(i4_file_class **fp, int t_files,
                      i4_array<i4_str *> &texture_name_array,
                      i4_array<i4_str *> &model_name_array,
                      int &total_tiles,
                      int include_model_textures)
{
  li_environment  *env=new li_environment(0,i4_T);


  li_set_value("texture_format", new li_string("x:/crack/golgotha/textures/%s.tga"));
  li_set_value("building_format", new li_string("x:/crack/golgotha/objects/%s.gmod"));
  li_set_value("object_format", new li_string("x:/crack/golgotha/objects/%s.gmod"));

  
         

  li_add_function("def_class", g1_ignore, env);
  li_add_function("models", g1_get_models_texture, env);
  li_add_function("textures", g1_get_tile_textures, env);
  li_add_function("def_object", g1_get_object_textures, env);
  li_add_function("def_buildings", g1_get_building_textures, env);
  li_add_function("def_skys", g1_sky_textures, env);
  li_add_function("find_models", g1_ignore, env);

  g1_current_tnames = &texture_name_array;
  g1_current_model_names = &model_name_array;
  g1_current_t_tiles=0;

  li_load("scheme/models.scm", env);
  for (int j=0; j<t_files; j++)
    li_load(fp[j], env);


            
  g1_current_model_names=0;
  g1_current_tnames=0;

  if (include_model_textures)
  {
    i4_status_class *stat=i4_create_status(i4gets("scanning_models"));

  
    for (int i=0; i<model_name_array.size(); i++)
    {
      if (stat)
        stat->update((i+1)/(float)model_name_array.size());
      g1_make_texture_list_for_model(*model_name_array[i], texture_name_array);
    }
    if (stat) delete stat;
  }


  total_tiles=g1_current_t_tiles;
}














