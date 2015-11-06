/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "editor/e_state.hh"
#include "gui/scroll_bar.hh"
#include "editor/dialogs/tile_picker.hh"
#include "window/style.hh"
#include "gui/button.hh"
#include "editor/dialogs/tile_win.hh"
#include "tmanage.hh"
#include "r1_api.hh"
#include "tile.hh"
#include "g1_render.hh"
#include "editor/mode/e_tile.hh"

g1_tile_picker_class::g1_tile_picker_class(i4_graphical_style_class *style, 
                                           g1_scroll_picker_info *info,
                                           i4_image_class *active_back,
                                           i4_image_class *passive_back)
  :
    g1_scroll_picker_class(style,
                           (1<<ROTATE) | (1<<MIRROR) | (1<<GROW) | (1<<SHRINK) | (1<<SCROLL),
                           info,
                           g1_tile_man.remap_size()
                           ),
    active_back(active_back),
    passive_back(passive_back)
    
{

}
 
int g1_tile_picker_class::total_objects()
{
  return g1_tile_man.remap_size();
}

i4_window_class *g1_tile_picker_class::create_window(w16 w, w16 h, int scroll_object_num)
{
  int t=g1_tile_man.remap_size();
  if (scroll_object_num>=t)
    return 0;

  g1_3d_pick_window::camera_struct tile_state;

  return new g1_3d_tile_window(w,h, scroll_object_num, tile_state,
                               active_back, passive_back, 0);
}


void g1_tile_picker_class::change_window_object_num(i4_window_class *win, 
                                                    int new_scroll_object_num)
{
  g1_3d_tile_window *twin=((g1_3d_tile_window *)win);

  g1_3d_pick_window::camera_struct old_camera=twin->camera; 
  int old_num=twin->tile_num;

  twin->set_tile_num(new_scroll_object_num);

}


void g1_tile_picker_class::rotate()
{
  if (g1_e_tile.get_cell_rotation()==G1_ROTATE_270)
    g1_e_tile.set_cell_rotation(G1_ROTATE_0);
  else      
    g1_e_tile.set_cell_rotation((g1_rotation_type)(g1_e_tile.get_cell_rotation()+1));


}

void g1_tile_picker_class::mirror()
{
  g1_e_tile.set_mirrored((i4_bool)(!g1_e_tile.get_mirrored()));
}

