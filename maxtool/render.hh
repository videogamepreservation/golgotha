/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef __M1_RENDER_HH_
#define __M1_RENDER_HH_

#include "time/time.hh"
#include "window/window.hh"
#include "window/wmanager.hh"
#include "math/transform.hh"
#include "math/point.hh"
#include "obj3d.hh"
#include "r1_api.hh"
#include "light.hh"
#include "max_object.hh"
#include "player_type.hh"
#include "m1_info.hh"

class m1_utility_state_class;

class m1_utility_window_class : public i4_parent_window_class
{                                            
private:
  int grab;
  int last_x, last_y, last_but;

  i4_float center_x, center_y, scale_x, scale_y;
  void calc_params()
  //{{{
  {
    int win_w=width(), win_h=height();
    w32 max_dim=win_w > win_h ? win_w : win_h;

    center_x=(i4_float)win_w/2.0;
    center_y=(i4_float)win_h/2.0;

    scale_x = (float)max_dim/win_w;
    scale_y = (float)max_dim/win_h;
  }
  //}}}

  friend m1_utility_state_class;
  m1_utility_state_class *state;

  i4_bool animating;

  i4_time_class last_time;

  m1_poly_object_class *get_obj() { return m1_info.obj; }

  w32 draws_needed;

  i4_float focal_length() { return 300; }

  i4_window_manager_class *wm;
  r1_render_api_class *api;

  void drop_files(int t_files, i4_str **filenames);
public:

  enum 
  // Grab Masks for input
  {
    LEFT_BUTTON   = 1,
    RIGHT_BUTTON  = 2,
    MIDDLE_BUTTON = 4,
    LEFT_KEY      = 8,
    RIGHT_KEY     = 16,
    MIDDLE_KEY    = 32,
    LEFT   = LEFT_BUTTON | LEFT_KEY,
    RIGHT  = RIGHT_BUTTON | RIGHT_KEY,
    MIDDLE = MIDDLE_BUTTON | MIDDLE_KEY,
  };

  w32 background_color;
  i4_angle theta,phi;
  i4_float dist, pan_x, pan_y, pan_z;
  g1_player_type current_player;
  i4_transform_class transform;


  char *name() { return "m1_utility_window_class"; }

  m1_utility_window_class(w16 window_width, 
                          w16 window_height,
                          r1_render_api_class *api,
                          i4_window_manager_class *wm,
                          i4_float theta,
                          i4_float phi,
                          i4_float dist);

  void init();
  void recenter();
  void recalc_view();

  i4_bool is_animating() const { return animating; }
  void set_animation(i4_bool on) { last_time.get(); animating = on; request_redraw(); }

  void get_configuration(i4_float &_theta,
                         i4_float &_phi,
                         i4_float &_dist)
  //{{{
  {
    _theta=theta;
    _phi=phi;
    _dist=dist;
  }
  //}}}

  virtual void receive_event(i4_event *ev);

  i4_bool project_point(const i4_3d_point_class &p, r1_vert &v);
  void draw_3d_text(i4_3d_point_class p1, const i4_const_str &text, 
                    w32 color,
                    i4_draw_context_class &context);
  void draw_3d_line(i4_3d_point_class p1,i4_3d_point_class p2,i4_color color, i4_bool on_top=i4_F);
  void draw_3d_point(i4_3d_point_class p, i4_color color, i4_bool on_top=i4_F);
  void draw_plane(const i4_3d_vector &u, const i4_3d_vector &v, w8 color);
  void update_object(i4_float time);
  void render_object(i4_draw_context_class &context);

  virtual void parent_draw(i4_draw_context_class &context);
  
  void set_current_player(g1_player_type player) { current_player=player; }

  void pan(i4_float x, i4_float y);

  void set_object(const i4_const_str &filename);
  void select_poly(int poly_num);

  i4_bool find_hit(int mouse_x, int mouse_y);
  void xlate_selected(int fromx, int fromy, int tox, int toy);

  void set_state(m1_utility_state_class *_state)
  //{{{
  {
    state = _state;
  }
  //}}}

  void restore_state();
};

class m1_utility_state_class
{
public:
  i4_float center_x() const;
  i4_float center_y() const;
  i4_float scale_x() const;
  i4_float scale_y() const;

  int width() const;
  int height() const;

  int mouse_x() const;
  int mouse_y() const;
  int buttons() const;

  int last_x() const;
  int last_y() const;

  virtual i4_bool mouse_down() { return i4_F; }
  virtual i4_bool mouse_drag() { return i4_F; }
  virtual i4_bool mouse_up()   { return i4_F; }
};

extern i4_event_handler_reference_class<m1_utility_window_class> m1_render_window;

inline w32 m1_texture_memory_size() { return 2048*1024; }

#endif

//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}
