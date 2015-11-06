/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_CONTROLLER_EDIT_CLASS_HH
#define G1_CONTROLLER_EDIT_CLASS_HH

#include "map.hh"
#include "device/event.hh"
#include "device/device.hh"
#include "math/point.hh"
#include "controller.hh"
#include "math/spline.hh"
#include "r1_vert.hh"
#include "editor/mode/e_mode.hh"
#include "map_cell.hh"

class g1_object_controller_class;
class g1_object_class;
class i4_spline_class;

class g1_controller_edit_class : public g1_object_controller_class
{
  friend class g1_mode_handler;
  friend class g1_tile_mode;
  friend class g1_camera_mode;
  friend class g1_light_mode;
  friend class g1_object_mode;
  friend class g1_ai_mode;

  g1_mode_handler *mode;

  i4_bool graph_changed;
  i4_bool drag_select, move_points, focus_visible;

  i4_spline_class::point *move_point;
  g1_object_class *place_object;
  i4_bool place_object_on_map;
  
  g1_object_class *selected_object;
  g1_player_type selected_object_player_num;

  // this this cell the mouse is currently on for this controller
  w32 cell_x,  cell_y; 

  //  i4_float last_game_x, last_game_y;   // projection on to ground

  // sw32 sel_x1, sel_y1, sel_x2, sel_y2;

  sw32 last_mouse_x, last_mouse_y;

  //  i4_bool mouse_down, mouse_grabbed;

  // if we need to restore the map back to normal (because we changed the
  // map for visual cues)
  i4_bool need_restore_cell;
  g1_map_cell_class saved_cell;
  void save_and_put_cell();

  // this will replace the 'original' with new cell and move all the objects
  // on original to new_cell
  void replace_cell(g1_map_class *map,
                    g1_map_cell_class &original, 
                    g1_map_cell_class &new_cell);

  void send_to_parent(i4_event *ev);  // defined in editor.cc

  void changed();          // so user can save, defined in editor.cc
  void change_map_cell();
  void fill_map();         // does a flood fill at the current cell x&y



  //  void tile_mouse_moved(sw32 x, sw32 y, i4_float gx, i4_float gy);
  //  void object_mouse_moved(sw32 x, sw32 y);

  void add_movie_control_point(int list_number);

  enum { SELECTED_PLAYER_NUM=2 };

  void refresh();

  i4_window_class *edit_win;

  void display_edit_win();  // defined in editor.cc

  //  void camera_mouse_move(sw32 x, sw32 y);
  //  void zoom_mouse_move(sw32 x, sw32 y);
  //  void rotate_mouse_move(sw32 x, sw32 y);
  //  void grab_mouse();
  //  void ungrab_mouse();

  void select_points_in_area(sw32 x1, sw32 y1, sw32 x2, sw32 y2,
                             g1_mode_handler::select_modifier mod);

  //  void draw_drag_rectangle(i4_draw_context_class &context);
  //  void drag_area(sw32 &dx1, sw32 &dy1, sw32 &dx2, sw32 &dy2);

  void clear_selected_points();
  void move_selected_points(i4_float xa, i4_float ya, i4_float za);
  void delete_selected_points();
  void insert_control_points();

  //  void height_modify_button_down();
  //  void height_modify_mouse_move(sw32 mx, sw32 my);
  g1_map_vertex_class *find_map_vertex(sw32 x, sw32 y,
                                       sw32 &vx, sw32 &vy);
  void clear_selected_verts();
  void select_verts_in_area(sw32 x1, sw32 y1, sw32 x2, sw32 y2,
                            g1_mode_handler::select_modifier mod);

  i4_bool move_verts;

  void change_light_direction();

  sw32 move_vert_x, move_vert_y;
  i4_float move_vert_z;

  i4_spline_class::point *find_spline_point(sw32 mx, sw32 my, w32 instance);  

  int get_current_splines(i4_spline_class **buffer, int buf_size);
  void setup_context();


public:
  virtual clear_if_cannot_draw() { return i4_F; }
  enum { MAX_SPLINE_EDIT=10 };

  void tile_cell_draw(sw32 x, sw32 y);
  void object_cell_draw(sw32 x, sw32 y);


  i4_bool active() { return g1_current_controller.get()==this; }
  ~g1_controller_edit_class();

  void focused();
  void unfocused();

  
  //  void set_current_height(w8 height);

  g1_controller_edit_class(w16 w, w16 h,
                           i4_graphical_style_class *style);

  void mouse_moved(sw32 x, sw32 y);

  void button_down(i4_mouse_button_down_event_class *bev);
  void button_up(i4_mouse_button_up_event_class *bev);


  void restore_cell();
    
  void update_cursor();

  void show_focus();
  void hide_focus();

  virtual void receive_event(i4_event *ev);

  void draw_3d_line(const i4_3d_point_class &p1,
                    const i4_3d_point_class &p2,
                    i4_color color1,     // for first point
                    i4_color color2,     // for second point
                    i4_image_class *local_image,
                    g1_draw_context_class *context);

  void draw_3d_point(sw32 w,
                     i4_color color,
                     const i4_3d_point_class &p,
                     i4_image_class *local_image,
                     g1_draw_context_class *context);

  void draw_spline(i4_image_class *local_image,
                   g1_draw_context_class *context, 
                   i4_color cpoint_color,
                   i4_color line_color,
                   i4_color spline_color,
                   i4_spline_class *s,
                   w32 cur_frame);

  virtual void process_input(i4_time_class tick_time);

  virtual void editor_pre_draw(i4_draw_context_class &context);
  virtual void editor_post_draw(i4_draw_context_class &context);

  void move_selected_heights(sw32 z_change);

  void setup_mode();

  void major_mode_change();
  g1_mode_handler *get_mode() { return mode; }

  char *name() { return "contedit"; }
};


#endif



