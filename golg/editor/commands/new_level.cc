/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "editor/editor.hh"
#include "editor/pmenu.hh"
#include "gui/create_dialog.hh"
#include "gui/text_input.hh"                                       
#include "remove_man.hh"
#include "m_flow.hh"
#include "app/app.hh"
#include "level_load.hh"
#include "window/colorwin.hh"
#include "file/file_open.hh"
#include "app/app.hh"
#include "light.hh"
#include "tile.hh"
#include "player.hh"
#include "map.hh"
#include "editor/e_res.hh"
#include "saver.hh"
#include "map_vert.hh"
#include "level_load.hh"

void g1_editor_class::open_new_level_window()
{
  enum { MIN_DIM=10, MAX_DIM=150 };
  int uw=g1_map_is_loaded() ? get_map()->width() : 100;
  int uh=g1_map_is_loaded() ? get_map()->height() : 100;

  i4_color_window_class *cw=new i4_color_window_class(500,500, 
                                                      style->color_hint->neutral(), style);
  i4_create_dialog(g1_ges("new_level_dialog"), 
                   cw,
                   style,
                   &new_dialog.name,
                   MIN_DIM, MAX_DIM,  &new_dialog.w, uw,
                   MIN_DIM, MAX_DIM,  &new_dialog.h, uh,
                   this, G1_EDITOR_NEW_OK,
                   this, G1_EDITOR_MODAL_BOX_CANCEL);

  cw->resize_to_fit_children();
  create_modal(cw->width(), cw->height(), "new_title");
  modal_window.get()->add_child(0,0,cw);  
}



g1_map_class *g1_editor_class::create_default_map(int w, int h,
                                                  const i4_const_str &name)
{
  g1_map_class *map=new g1_map_class(i4gets("tmp_savename"));


  map->verts=(g1_map_vertex_class *)i4_malloc((w+1) * (h+1) * 
                                                    sizeof (g1_map_vertex_class),
                                                    "map_verts");

  map->cells=(g1_map_cell_class *)i4_malloc(w * h * sizeof (g1_map_cell_class), "map cells");
  map->w=w;
  map->h=h;

  g1_set_map(map);


  int default_tile_type=g1_tile_man.get_default_tile_type();


  w32 x,y,mw=map->width(),mh=map->height();
  for (x=0; x<mw; x++)
  {
    for (y=0; y<mh; y++)
    {
      g1_map_cell_class *c=map->cell(x,y);
      c->init(default_tile_type, G1_ROTATE_0, i4_F);

      if (x==0 || y==0 || x==mw-1 || y==mh-1)
        c->flags=0;
      else
        c->flags=g1_map_cell_class::IS_GROUND;
    }
  }

  for (x=0; x<mw+1; x++)
    for (y=0; y<mh+1; y++)
      map->vertex(x,y)->init();

  map->current_movie=new g1_movie_flow_class;
  map->current_movie->set_scene(0);


  int old_allow=undo.allow;
  undo.allow=0;
  map->recalc_static_stuff();
  undo.allow=old_allow;

  for (int i=0; i<MAX_VIEWS; i++)
  {
    view_states[i].defaults();
    view_states[i].suggest_camera_mode(G1_EDIT_MODE);

    if (views[i])
      views[i]->view=view_states[i];
  }

  map->set_filename(name);

  g1_lights.defaults();

  g1_player_man.load(0);

  g1_object_type factory_pad=g1_get_object_type("factory_pad");
  if (factory_pad && w>25 && h>25 && G1_MAX_PLAYERS>=4)
  {
    g1_object_class *o;

    o=g1_create_object(factory_pad);   
    if (o)
    {
      o->x=o->lx=w/2;
      o->y=o->ly=h/2;
      o->h=o->lh=map->terrain_height(o->x, o->y);
      o->player_num=g1_player_man.get_local()->get_player_num();

      o->occupy_location();
      o->request_think();
      g1_player_man.get(o->player_num)->add_object(o->global_id);
    }
  }


  return map;
}




void g1_editor_class::new_level_from_dialog()
{

  int w,h;

  i4_str::iterator is=new_dialog.w->get_edit_string()->begin();
  w=is.read_number();
  is=new_dialog.h->get_edit_string()->begin();
  h=is.read_number();


  if (!(w>=G1_MIN_MAP_DIMENSION && h>=G1_MIN_MAP_DIMENSION 
        && w<=G1_MAX_MAP_DIMENSION && h<=G1_MAX_MAP_DIMENSION))
  {
    close_modal();

    create_modal(300, 50, "bad_w_h_title");
    i4_create_dialog(g1_ges("bad_map_w_h_dialog"), 
                     modal_window.get(), style, w, h, 
                     G1_MIN_MAP_DIMENSION,
                     G1_MAX_MAP_DIMENSION,
                     this, G1_EDITOR_MODAL_BOX_CANCEL);
  }
  else  
  {
    add_undo(G1_MAP_ALL);

    close_windows();
      
    changed();

    if (g1_map_is_loaded())
      g1_destroy_map();

    i4_str *res_name=g1_get_res_filnename(*new_dialog.name->get_edit_string());
    i4_file_class *fp=i4_open(*res_name);
    delete res_name;
    
    if (fp)
    {
      g1_load_res_info(fp);
      delete fp;
    }



    g1_initialize_loaded_objects();

    g1_map_class *map=create_default_map(w,h, 
                                         *new_dialog.name->get_edit_string());
  }
  
  close_modal();
}



