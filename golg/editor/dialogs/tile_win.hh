/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef TILE_WINDOW_HH
#define TILE_WINDOW_HH


#include "window/window.hh"
#include "g1_limits.hh"
#include "menu/menuitem.hh"
#include "editor/dialogs/pick_win.hh"

class g1_quad_object_class;

class g1_3d_tile_window : public g1_3d_pick_window
{
  g1_quad_object_class *object;

  public :
  int tile_num;

  void set_tile_num(int num) 
  { 
    tile_num=num;
    request_redraw(i4_F);
  }

  g1_3d_tile_window(w16 w, w16 h,
                    int tile_num,
                    g1_3d_pick_window::camera_struct &camera,
                    i4_image_class *active_back,
                    i4_image_class *passive_back,
                    i4_event_reaction_class *reaction);

  i4_bool selected();
  void do_press();
  void draw_object(g1_draw_context_class *context);
  i4_menu_item_class *copy() { return new g1_3d_tile_window(width(), height(),
                                                            tile_num,
                                                            camera, act, pass,
                                                            reaction->copy()); }
  
  char *name() { return "object window"; }
} ;


#endif



