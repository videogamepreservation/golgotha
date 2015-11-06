/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_TIME_MODE_HH
#define G1_TIME_MODE_HH

#include "editor/mode/e_mode.hh"
#include "editor/dialogs/tile_picker.hh"
#include "rotation.hh"

class g1_map_vertex_class;

class g1_tile_mode : public g1_mode_handler
{
  float vert_exact_z;
  g1_map_vertex_class *move_pivot;
  i4_bool focus_visible;

public:
  virtual void mouse_down();
  virtual void mouse_move(sw32 mx, sw32 my);

  virtual i4_bool select_object(sw32 mx, sw32 my, 
                                i4_float &ox, i4_float &oy, i4_float &oz,
                                select_modifier mod);

  virtual void select_objects_in_area(sw32 x1, sw32 y1, sw32 x2, sw32 y2, 
                                      select_modifier mod);

  virtual void move_selected(i4_float xc, i4_float yc, i4_float zc,
                             sw32 mouse_x, sw32 mouse_y);

  virtual void hide_focus();
  virtual void show_focus();

  virtual state current_state();
  virtual void key_press(i4_key_press_event_class *kev);

  g1_tile_mode(g1_controller_edit_class *c);

};


class g1_tile_params : public g1_mode_creator
{
  void refresh_picker();
  i4_parent_window_class *picker_mp_window;
  i4_event_handler_reference_class<g1_tile_picker_class> picker;
  i4_event_handler_reference_class<i4_parent_window_class> mp_window;
  g1_rotation_type current_cell_rotation;
  i4_bool mirrored;
  int vert_move_snap;                      // number of units to move a vert when dragging
  w16               current_cell_type;

  g1_scroll_picker_info picker_info;

public:
  g1_tile_params();
  char *name() { return "TILE"; }

  enum
  { ROTATE,
    ZOOM,
    PLACE,
    FILL,
    HEIGHT
  };
  

  g1_rotation_type get_cell_rotation() { return current_cell_rotation; }
  void set_cell_rotation(g1_rotation_type rot) { current_cell_rotation=rot; refresh_picker(); }
  
  i4_bool get_mirrored() { return mirrored; }
  void set_mirrored(i4_bool value) { mirrored=value; refresh_picker(); }

  w16 get_cell_type() { return current_cell_type; }
  void set_cell_type(w16 type) { current_cell_type=type; refresh_picker(); }


  enum { HEIGHT_ONLY=1 } modifer_flags;           

  void create_buttons(i4_parent_window_class *container);   

  g1_mode_handler *create_mode_handler(g1_controller_edit_class *c)
  { return new g1_tile_mode(c); }

  i4_bool allow_picker_creation() { return picker.get()==0; }

  void open_picker();
  void cleanup();


} ;

extern g1_tile_params g1_e_tile;



#endif
