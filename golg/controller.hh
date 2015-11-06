/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef CONTROLLER_HH
#define CONTROLLER_HH

// This class is used to display a perspective view from an object.  This
// object is tracked and feed input that goes to the controller window.
#include "window/window.hh"
#include "math/num_type.hh"
#include "error/error.hh"
#include "math/transform.hh"
#include "g1_object.hh"
#include "time/time.hh"
#include "g1_render.hh"
#include "map_man.hh"
#include "memory/que.hh"
#include "camera.hh"
#include "draw_context.hh"
#include "time/timedev.hh"

class g1_quad_object_class;
class i4_graphical_style_class;
class g1_player_piece_class;
class g1_map_class;
class r1_render_api_class;
class i4_menu_class;
class g1_map_view_class;
class i4_event_handler_class;
class g1_draw_context_class;
class g1_map_piece_class;
class i4_window_message_class;
class i4_do_command_event_class;
class i4_end_command_event_class;
class g1_loader_class;
class g1_saver_class;
class g1_object_controller_class;


class g1_controller_global_id_reset_notifier : public g1_global_id_reset_notifier
{
public:
  g1_object_controller_class *for_who;

  g1_controller_global_id_reset_notifier(g1_object_controller_class *for_who)
    : for_who(for_who) {}


  void reset();
};



class g1_object_controller_class : public i4_parent_window_class
{
protected:
  g1_controller_global_id_reset_notifier notifier;
  i4_bool first_frame;

  g1_draw_context_class g1_context;
  void setup_context(i4_draw_context_class &i4_context);


  // this is the object we are currently tracking with the camera
  g1_player_piece_class *get_track();

  g1_map_view_class *radar_view; // maintains the view of the map as seen on radar

  enum flag_type {
    KEY_FOCUS          =1,     // if we have keyboard focus
    GOT_KEYBOARD       =4,     // if a keyboard exists on sytem
    NEED_CLEAR         =8      // if we need to clear the screen the next frame
  } ;
  w32 flags;

  void set_flag(flag_type flag, i4_bool on=i4_T) { if (on) flags|=flag; else flags&=(~flag); }
  i4_bool get_flag(flag_type flag) { if (flags & flag) return i4_T; else return i4_F; }


  i4_float last_theta, last_h;
  i4_float last_x, last_y;


  i4_menu_class *context_menu;
  i4_graphical_style_class *style;
  w32 last_mouse_x, last_mouse_y;


  w32 frames_to_count;
  i4_float frame_rate;

  enum { FRAME_MEASURE_INTERVAL=8 };

  void create_context_menu();
  void delete_context_menu();


  void set_track(g1_player_piece_class *new_track);

  void draw_overhead(g1_draw_context_class *context);

  // Window Input
  void get_keys();
  void lose_keys();

  void do_command_event(i4_do_command_event_class *ev);
  void end_command_event(i4_end_command_event_class *ev);

  void window_event(i4_window_message_class *wev);
  void key_press(i4_key_press_event_class *ev);
  void calc_camera_transform();

  void change_mouse_mask(int new_mask);
  void draw_counts();

  g1_selectable_list selectable_list;

  // Clipping

  // Worldspace coordinates of the view frustrum
  i4_3d_vector camera_point[8];

  // View Frustrum bounding box
  class bbox_class
  {
  public:
    i4_3d_vector min, max;
  } bbox;
  
  class plane_class
  // View Frustrum Cutting Planes
  {
  public:
    // Uses the plane equation: normal*x + D = 0
    i4_3d_vector normal;
    i4_float D;
    w8 n,p;

    void calc_np()
    {
      // calculate the index of the most positive(p) & negative(n) corners
      //   in calculating the plane equation

      p = 0;

      // p points toward the normal
      p |= (normal.x>0) ? 1 : 0;
      p |= (normal.y>0) ? 2 : 0;
      p |= (normal.z>0) ? 4 : 0;
      
      // n points away from the normal
      n = p^7;
    }

    i4_float evaluate(const i4_3d_vector& point)
    {
      return normal.dot(point) + D;
    }
  } plane[4];

  // Text Messages
  void start_scroll();
  void stop_scroll();
  i4_time_device_class::id message_scroll_id;
  i4_bool scroll_active;

  enum { MAX_MESSAGES=3 };
  struct message
  {
    w32 color;
    char text[40];
  };
  message messages[MAX_MESSAGES];

  // Spinning status indicators
  i4_time_class last_stat_time;                   // time spin started

  i4_float stat_x, stat_y;
  int stat_type;
  int stat_inc;
  sw32 stat_time;                            // time left  in milli for spin

  struct spin_event
  {
    g1_model_id_type model;
    enum spin_type { GAINED, LOST} type;
    spin_event() { model=0; }
    spin_event(g1_model_id_type model, w8 type) : model(model), type((spin_type)type) {}
  };

  i4_fixed_que<spin_event,8> spin_events;     // list of models to spin up
  spin_event current_spin;
  virtual void reparent(i4_image_class *draw_area, i4_parent_window_class *parent);
public:
  void scroll_message(const i4_const_str &message, i4_color color=0xffffff);

  void reset_global_ids();
  virtual clear_if_cannot_draw() { return i4_T; }
  g1_view_state_class view;

  void pan(i4_float x, i4_float y, i4_float z);
  void rotate(i4_float about_game_z, i4_float up_down);
  void zoom(i4_float dist);
  
  i4_bool add_spin_event(char *model_name, w8 lost=0);


  i4_transform_class transform;  // current transform to go world to screen space

  void get_pos(i4_3d_vector& pos);

  void setup_clip();            
  // after transform is valid, setup clipping planes
  int test_clip(const i4_3d_vector& min, const i4_3d_vector& max);
  // test bounding box for visibility in the view frustrum
  //   -1 = completely outside
  //    0 = on boundary
  //    1 = completely within view

  w8 cursor_state;

  virtual void update_cursor();

  g1_typed_reference_class<g1_object_class> follow;

  g1_player_type team_on() const;

  void update_camera();
  void set_exclusive_mode(i4_bool yes_no);

  g1_map_class *get_map() { return g1_get_map(); }

  i4_time_class last_frame_draw_time;      // time last frame draw took place

  i4_bool view_to_game(sw32 mouse_x, sw32 mouse_y,
                       i4_float &game_x, i4_float &game_y,
                       i4_float &dir_x, i4_float &dir_y);
  // converts 2D view coordinates & projects the point onto the game map
  //   if doesn't intersect map, returns i4_F

  // resize will change the width and height of our window and notify our parent of the change
  virtual void resize(w16 new_width, w16 new_height);

  i4_graphical_style_class *get_style() { return style; }
  g1_object_controller_class(w16 w, w16 h, 
                             i4_graphical_style_class *style);

  virtual void editor_pre_draw(i4_draw_context_class &context) { ; }
  virtual void editor_post_draw(i4_draw_context_class &context) { ; }

  virtual void parent_draw(i4_draw_context_class &context);

  virtual void receive_event(i4_event *ev);

  virtual void show_self(w32 indent) 
  {    
    char fmt[50];
    sprintf(fmt,"%%%ds object_controller_class",indent);
    i4_warning(fmt," ");
  }

  g1_object_class *find_object_under_mouse(sw32 mx, sw32 my);

  enum { MOUSE_LEFT=1, 
         MOUSE_RIGHT=2 };
  w8 mouse_mask;

  char *name() { return "g1_object_controller"; }
  ~g1_object_controller_class();

};
   
extern i4_event_handler_reference_class<g1_object_controller_class> g1_current_controller;


#endif





