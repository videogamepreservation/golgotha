/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef OBJECT_MODE_HH
#define OBJECT_MODE_HH


#include "editor/e_state.hh"
#include "editor/mode/e_mode.hh"
#include "editor/dialogs/object_picker.hh"

class g1_object_class;

class g1_object_mode : public g1_mode_handler
{
  g1_typed_reference_class<g1_object_class> add;
  sw32 sel_color, norm_color;

  i4_bool no_more_move_undos;
  w32 current_edit_type;

public:
  float real_object_theta;        // unsnapped_value
  float get_snap_theta();

  enum { SET_TEAM=1, CLOSE_WINDOW };


  virtual state current_state();
  void mouse_down();
  void mouse_move(sw32 mx, sw32 my);
  void key_press(i4_key_press_event_class *kev);

  virtual i4_bool select_object(sw32 mx, sw32 my, 
                                i4_float &ox, i4_float &oy, i4_float &oz,
                                select_modifier mod);

  virtual void select_objects_in_area(sw32 x1, sw32 y1, sw32 x2, sw32 y2,
                                      select_modifier mod);

  virtual void move_selected(i4_float xc, i4_float yc, i4_float zc,
                             sw32 mouse_x, sw32 mouse_y);

  virtual void delete_selected();

  virtual void post_draw(i4_draw_context_class &context);

  virtual void hide_focus();
  virtual void show_focus();

  virtual void idle();
  virtual void edit_selected();

  g1_object_mode(g1_controller_edit_class *c);
};


class g1_object_params : public g1_mode_creator
{
  w16 current_object_type;
    


  i4_event_handler_reference_class<i4_parent_window_class> edit_win;
  float object_move_snap;


public:

  g1_object_params();
  char *name() { return "OBJECT"; }
  enum  { ROTATE,    ZOOM,    ADD,    SELECT,    MOVE,    OBJECT_ROTATE  };


  void refresh_picker()
  {
    g1_refresh_object_picker();
  }

  w16 get_object_type() 
  { 
    return current_object_type;
  }

  void set_object_type(w16 type) 
  {     
    current_object_type=type; 
    refresh_picker(); 
  }

  void create_buttons(i4_parent_window_class *containter);
  void cleanup();

  g1_mode_handler *create_mode_handler(g1_controller_edit_class *c);
  void set_edit_window(i4_window_class *p);
  void close_edit_window();
};

extern g1_object_params g1_e_object;



#endif
