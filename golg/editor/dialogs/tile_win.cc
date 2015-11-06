/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "editor/dialogs/tile_win.hh"
#include "obj3d.hh"
#include "math/pi.hh"
#include "image/image.hh"
#include "draw_context.hh"
#include "window/win_evt.hh"
#include "device/kernel.hh"
#include "image/color.hh"
#include "r1_api.hh"
#include "editor/e_state.hh"
#include "tile.hh"
#include "map_vert.hh"
#include "map_cell.hh"
#include "g1_render.hh"
#include "editor/mode/e_tile.hh"
#include "resources.hh"



g1_3d_tile_window::g1_3d_tile_window(w16 w, w16 h,
                                     int tile_num,
                                     g1_3d_pick_window::camera_struct &camera,
                                     i4_image_class *active_back,
                                     i4_image_class *passive_back,
                                     i4_event_reaction_class *reaction)
  :  g1_3d_pick_window(w,h,
                       active_back, passive_back,
                       camera,
                       reaction),
     object(object),
     tile_num(tile_num)

{
  w32 min_screen = w<h ? w : h;
}


inline void g1_setup_vert(r1_vert *v, float px, float py, float z)
{
  v->px=px;
  v->py=py;
  v->v.z=z;
  v->w=1.0/z;
}

void g1_3d_tile_window::do_press()
{
  if (tile_num<g1_tile_man.remap_size())
  {
    g1_e_tile.set_cell_type(g1_tile_man.get_remap(tile_num));

    if (strcmp(g1_edit_state.major_mode,"TILE"))
      g1_edit_state.set_major_mode("TILE");

    if (g1_e_tile.get_minor_mode() != g1_tile_params::PLACE)
      g1_edit_state.set_minor_mode("TILE",g1_tile_params::PLACE);
  }
}

i4_bool g1_3d_tile_window::selected()
{
  if (tile_num==g1_e_tile.get_cell_type())
    return i4_T;
  else
    return i4_F;
}

void g1_3d_tile_window::draw_object(g1_draw_context_class *context)
{
  if (tile_num<g1_tile_man.remap_size())
  {
    r1_texture_handle han=g1_tile_man.get_texture(g1_tile_man.get_remap(tile_num)); 
    r1_render_api_class *render_api=g1_render.r_api;

    if (active)
      render_api->set_constant_color(0xffffff);
    else               
      render_api->set_constant_color(0x9f9f9f);

    if (tile_num<g1_tile_man.remap_size())
    {
      render_api->use_texture(han, width(), 0);

      r1_vert v1[3],v2[3];
      g1_setup_tri_texture_coords(v1, v2, g1_e_tile.get_cell_rotation(), g1_e_tile.get_mirrored());

      g1_setup_vert(v1,   0, height()-1, 50);
      g1_setup_vert(v1+1, width()-1,height()-1, 50);
      g1_setup_vert(v1+2, width()-1,0, 50);

      render_api->render_poly(3, v1);

      g1_setup_vert(v2,   0, height()-1, 50);
      g1_setup_vert(v2+1, width()-1, 0, 50);
      g1_setup_vert(v2+2, 0, 0, 50);

      render_api->render_poly(3, v2);
    }
  }
}



