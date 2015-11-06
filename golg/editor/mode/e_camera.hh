/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef CAMERA_HANDLER_HH
#define CAMERA_HANDLER_HH


#include "editor/e_state.hh"
#include "editor/mode/e_mode.hh"

class g1_camera_mode : public g1_mode_handler
{
public:

  virtual state current_state();
  void mouse_down();

  virtual i4_bool select_object(sw32 mx, sw32 my, 
                                i4_float &ox, i4_float &oy, i4_float &oz,
                                select_modifier add_modifier);

  virtual void select_objects_in_area(sw32 x1, sw32 y1, sw32 x2, sw32 y2, 
                                      select_modifier add_modifier);

  virtual void move_selected(i4_float xc, i4_float yc, i4_float zc, 
                             sw32 mouse_x, sw32 mouse_y);

  virtual void post_draw(i4_draw_context_class &context);
  
  virtual void delete_selected();

  virtual void edit_selected();

  g1_camera_mode(g1_controller_edit_class *c) : g1_mode_handler(c) {}
};



class g1_camera_params : public g1_mode_creator
{
public:
  char *name() { return "CAMERA"; }
  g1_camera_params();

  enum
  { 
    ROTATE,
    ZOOM,
    ADD_CAMERA,
    ADD_TARGET,
    ADD_OBJECT,
    MOVE,
    SELECT
  };

  void create_buttons(i4_parent_window_class *containter);
  i4_event_handler_reference_class<i4_parent_window_class> edit_win;


  i4_bool should_show_frames() { return i4_F; }

  void cleanup();
  void set_edit_window(i4_parent_window_class *p, sw32 x, sw32 y);

  g1_mode_handler *create_mode_handler(g1_controller_edit_class *c)
  { return new g1_camera_mode(c); }


} ;

extern g1_camera_params g1_e_camera;



#endif


