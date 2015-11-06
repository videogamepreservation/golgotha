/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef OBJECT_WINDOW_HH
#define OBJECT_WINDOW_HH

#include "window/window.hh"
#include "g1_limits.hh"
#include "menu/menuitem.hh"
#include "editor/dialogs/pick_win.hh"
#include "g1_object.hh"
#include "reference.hh"

class g1_object_class;
class r1_render_api_class;

class g1_3d_object_window : public g1_3d_pick_window
{
public:
  g1_typed_reference_class<g1_object_class> object;
  g1_object_type      object_type;
  w16                 array_index;

  i4_menu_item_class *copy() { return new g1_3d_object_window(width(), height(), 
                                                              object.valid() ? object->id : 0,
                                                              array_index,
                                                              camera,
                                                              act, pass, reaction->copy()); }

  void set_object_type(g1_object_type type, w16 array_index);

  g1_3d_object_window(w16 w, w16 h,
                      g1_object_type obj_type,
                      w16 array_index,
                      g1_3d_pick_window::camera_struct &camera,
                      i4_image_class *active_back,
                      i4_image_class *passive_back,
                      i4_event_reaction_class *reaction);


  void draw_object(g1_draw_context_class *context);

  i4_bool selected();
  void do_press();

  void do_idle();

  char *name() { return "object window"; }
} ;


#endif
