/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "lisp/lisp.hh"
#include "file/file.hh"
#include "memory/array.hh"
#include "tile.hh"
#include "load3d.hh"
#include "g1_render.hh"
#include "r1_api.hh"
#include "tmanage.hh"
#include "map_man.hh"
#include "map.hh"
#include "cwin_man.hh"
#include "make_tlist.hh"
#include "saver.hh"
#include "border_frame.hh"
#include "file/sub_section.hh"
#include <stdlib.h>

li_object *g1_add_textures(li_object *o, li_environment *env)
{
  for (o=li_cdr(o,0); o; o=li_cdr(o,0))
    g1_tile_man.add(li_car(o,0), env);

  return 0;
}


static li_object *g1_ignore(li_object *o, li_environment *env) {  return 0;}

i4_str *g1_get_res_filnename(const i4_const_str &map_filename)
{
  i4_filename_struct fn;
  i4_split_path(map_filename, fn);

  char res_name[256];

  int len=strlen(fn.filename);
  while (len && (fn.filename[len-1]>='0' && fn.filename[len-1]<='9'))
    len--;
  res_name[len]=0;

  if (fn.path[0])
    sprintf(res_name, "%s/%s.scm", fn.path, fn.filename);
  else
    sprintf(res_name, "%s.scm", fn.filename);

  return new i4_str(res_name);
}

void g1_load_res_info(i4_file_class *fp)
{
  li_environment *local_env=new li_environment(0,i4_T);

  i4_array<i4_str *> tlist(512,512), mlist(512,512);

  li_list *scheme_list;
  int t_tiles;

  r1_texture_manager_class *tman=g1_render.r_api->get_tmanager();
  tman->reset();

  i4_file_class *fp_list[1];
  fp_list[0]=fp;

  // get a list of models and textures we need to load for this level
  g1_get_load_info(fp_list,1,  tlist,  mlist, t_tiles, 0);

  if (g1_strategy_screen.get())
    g1_strategy_screen->create_build_buttons();

  g1_tile_man.reset(t_tiles);

  g1_model_list_man.reset( mlist, tman);

  int i;
  for (i=0; i<tlist.size(); i++)
    delete   tlist[i];

  for (i=0; i<mlist.size(); i++)
    delete mlist[i];

  li_add_function("models", g1_ignore, local_env);
  li_add_function("textures", g1_add_textures, local_env);

  li_load("scheme/models.scm", local_env);
  fp->seek(0);
  li_load(fp, local_env);


  g1_tile_man.finished_load();
  tman->load_textures();
  g1_render.install_font();

  tman->keep_resident("lod_vehicles1", 256);
}

i4_bool g1_load_level(const i4_const_str &filename, int reload_textures_and_models,
                      w32 exclude_flags)
{
  int i;
  i4_file_class *fp=i4_open(filename);

  if (!fp) 
    return 0;

  g1_loader_class *load=g1_open_save_file(fp);
  if (!load)
    return 0;

  if (reload_textures_and_models)
  {
    if (g1_map_is_loaded())
      g1_destroy_map();

    w32 off,size;
    i4_file_class *res_file=0;
    if (load->get_section_info("resources", off, size))
      res_file=new i4_sub_section_file(i4_open(filename),off,size);

    if (!res_file)
    {
      i4_str *res_name=g1_get_res_filnename(filename);
      res_file=i4_open(*res_name);
      delete res_name;
    }

    if (res_file)
    {
      g1_load_res_info(res_file);
      delete res_file;
    }
  }
   
  g1_initialize_loaded_objects();

  g1_map_class *map;
  if (g1_map_is_loaded())
    map=g1_get_map();
  else
    map=new g1_map_class(filename);


  w32 ret=map->load(load, G1_MAP_ALL & (~exclude_flags));
  delete load;

  if ((ret & (G1_MAP_CELLS | G1_MAP_VERTS))==0)
  {
    delete map;
    return i4_F;
  }

  g1_set_map(map);

  //(OLI) hack for jc to only calculate LOD textures once
  if (reload_textures_and_models)
    map->init_lod();

  g1_cwin_man->map_changed();

  return i4_T;
}




