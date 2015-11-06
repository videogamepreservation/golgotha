/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef MODE_HANDLER_HH
#define MODE_HANDLER_HH


#include "math/num_type.hh"
#include "arch.hh"
#include "time/time.hh"
#include "device/device.hh"
#include "math/point.hh"
#include "device/event.hh"

class g1_controller_edit_class;
class i4_draw_context_class;
class i4_key_press_event_class;
class i4_graphical_style_class;
class i4_window_class;
class i4_parent_window_class;
class i4_transform_class;

class g1_mode_handler : public i4_event_handler_class
{
private:
  i4_bool drag, drag_select;
  sw32 sx1,sy1,sx2,sy2;

  i4_3d_point_class pos,        // current un-snapped position of drag object
    snap_pos;                  // current real (snapped) position of drag object

  i4_time_class last_click_time;
  void grab_mouse();
  void ungrab_mouse();

protected:
  sw32 lx(); 
  sw32 ly();
  sw32 x(); 
  sw32 y();

  i4_bool mouse_down_flag;
  g1_controller_edit_class *c;
  i4_bool active();
  
public:
  i4_transform_class *get_transform();

  w8 cursor_type;

  enum state { DRAG_SELECT, ZOOM, ROTATE, OTHER };
  enum select_modifier { CLEAR_OLD_IF_NO_SELECTION, 
                         ADD_TO_OLD, 
                         SUB_FROM_OLD,
                         FOR_CURSOR_HINT  // don't actually select anything
  };

  virtual state current_state() = 0;

  virtual void mouse_move(sw32 mx, sw32 my);
  virtual void mouse_down();
  virtual void mouse_up();
  virtual void do_command(i4_do_command_event_class *cmd);
  virtual void end_command(i4_end_command_event_class *cmd);

  virtual i4_bool select_object(sw32 mx, sw32 my, 
                                i4_float &ox, i4_float &oy, i4_float &oz,
                                select_modifier mod) 
  { return i4_F; }

  virtual void select_objects_in_area(sw32 x1, sw32 y1, sw32 x2, sw32 y2, 
                                      select_modifier add_modifier)
  { ; }

  virtual void delete_selected() { ;}

  virtual void move_selected(i4_float xc, i4_float yc, i4_float zc, 
                             sw32 mouse_x, sw32 mouse_y) { ; }

  virtual void edit_selected() { ; }

  virtual void post_draw(i4_draw_context_class &context);
  
  virtual void key_press(i4_key_press_event_class *kev) { ; }

  virtual void hide_focus() { ; }
  virtual void show_focus() { ; }
  
  virtual void idle() { ; }

  virtual i4_bool pass_through_focus_click();

  virtual void update_cursor();
  virtual void uninit();

  g1_mode_handler(g1_controller_edit_class *c);

  char *name() { return "mode handler"; }
};


class g1_mode_creator : public i4_event_handler_class
{
protected:
public:
  w8 minor_mode;

  static g1_mode_creator *first;
  g1_mode_creator *next;

  i4_graphical_style_class *get_style();

  g1_mode_creator();
  w8 get_minor_mode() { return minor_mode; }

  virtual i4_bool set_minor_mode(w8 mode) { minor_mode=mode; return i4_T; }

  // these buttons will be placed on the right of the major mode buttons
  virtual void create_buttons(i4_parent_window_class *container) = 0;

  // the name is used to find the major button, & major mode button help
  virtual char *name() = 0;

  // creates on of the above which handles mouse & keys for this mode
  virtual g1_mode_handler *create_mode_handler(g1_controller_edit_class *c) = 0;

  // will be called when the editor cleans up, should free all memory associated with mode
  virtual void cleanup() { ; }
  virtual ~g1_mode_creator();
};


#endif
