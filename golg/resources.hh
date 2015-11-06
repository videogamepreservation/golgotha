/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_RESOURCES_HH
#define G1_RESOURCES_HH

#include "math/num_type.hh"
#include "math/point.hh"


class i4_image_class;
class i4_cursor_class;
class i4_string_manager_class;
class i4_str;
class i4_const_str;

enum { 
  G1_DEFAULT_CURSOR,
  G1_SELECT_CURSOR,
  G1_MOVE_CURSOR,
  G1_TARGET_CURSOR,
  G1_FOLLOW_MODE_CURSOR,
  G1_X_CURSOR,
  G1_TOTAL_CURSORS  
};


class g1_resource_class    // these are all defined in constants.res
{
public:
  struct rect
  {
    sw16 x1,y1,x2,y2;
  };
private:
  i4_string_manager_class *use;

  void g_float(i4_float &f, char *res_name);
  void g_cursor(i4_cursor_class *&c, char *res_name, int big, int color);
  void g_point(i4_3d_point_class &p, char *res_name);
  void g_w32(w32 &x, char *res_name);
  void g_sw32(sw32 &x, char *res_name);
  void g_w16(w16 &x, char *res_name);
  void g_sw16(sw16 &x, char *res_name);
  void g_rect(rect &r, char *res_name);
public:


  enum { IMAGE_MONEY_0, 
         IMAGE_MONEY_9=9,
         IMAGE_MONEY_DOLLAR,
         IMAGE_MONEY_COMMA,
         
         OPTIONS_OPEN,
         OPTIONS_VIS_LOW,
         OPTIONS_VIS_MEDIUM,
         OPTIONS_VIS_HIGH,
         OPTIONS_SOUND,
         OPTIONS_SOUND_3D,
         OPTIONS_SHADOWS_OFF,
         OPTIONS_SHADOWS_ON,
         OPTIONS_INTERLACE,
         OPTIONS_DOUBLEPIXEL,
         OPTIONS_PIXEL_NORMAL,
         OPTIONS_GAME_SPEED
       };

  int t_build_context_helps;
  i4_const_str *build_context_help;     // fetched from res/g1.res

  int t_images;
  i4_image_class **images;


  enum dflags {
    DRAW_BUILDINGS=1,
    DRAW_GROUND=2,
    DRAW_OBJECTS=4,
    DRAW_SPRITES=8,
    DRAW_CLEAR=16
  } ;
  enum { DRAW_ALL = 
         DRAW_BUILDINGS |
         DRAW_GROUND  | 
         DRAW_OBJECTS |
         DRAW_SPRITES };

  w8 draw_flags;
  w8 render_window_expand_mode;


  i4_str *username;  // for net games


  i4_cursor_class *big_cursors[G1_TOTAL_CURSORS];
  i4_cursor_class *small_cursors[G1_TOTAL_CURSORS];

  i4_float recoil_factor, 
    recoil_gravity;

  i4_float sink_rate;
  sw16 water_damage;

  i4_float player_turn_speed,
    strafe_turn_speed,
    player_accel,
    player_strafe_accel,
    player_max_speed,
    player_max_strafe_speed,
    player_stop_friction,
    player_roll_speed,
    player_roll_max,
    player_turret_radius,
    player_fire_slop_angle,
    bullet_speed;


  rect health_window, chain_gun_window, 
    main_gun_window, missile_window, options_window, lives,
    mouse_scroll, balance,
    build_buttons;
  

  i4_float camera_dist,
    camera_angle,
    startegy_camera_angle,
    startegy_camera_dist,
    strategy_camera_turn_angle,
    follow_camera_dist,
    follow_camera_height,
    follow_camera_rotation;

  float lod_switch_dist, 
        lod_disappear_dist,
         skimpy_details_dist;
    

  i4_3d_point_class player_top_attach;
  i4_3d_point_class rocket_tank_top_attach;
  i4_3d_point_class turret_muzzle_attach, turret_top_attach;
  i4_3d_point_class supergun_muzzle_attach;
  i4_3d_point_class repairer_tip_attach;

  i4_float repairer_boom_offset;

  i4_float gravity, 
    damping_friction, 
    damping_fraction, 
    ln_damping_friction;

  enum radius_mode_type { VIEW_LOW, VIEW_MEDIUM, VIEW_FAR } radius_mode;
  i4_float view_radii[3];  // contains the three fields, near, medium, and far 
  i4_float visual_radius() { return view_radii[radius_mode]; }


  w16 net_hostname_x, net_hostname_y, 
    net_username_x, net_username_y, 
    net_found_x1, net_found_x2, net_found_y,
    net_start_x, net_start_y;
  
  w16 net_find_port, net_udp_port;

  w16 small_health_added, large_health_added;
  w16 small_bullets_added, large_bullets_added;
  w16 small_missiles_added, large_missiles_added;
  w16 small_money_added, large_money_added;
  w16 small_chain_added, large_chain_added;

    
  sw16 compass_x, compass_y;

  w16 stat_pad_size_x, stat_pad_size_y;
  sw16 stat_pad_start_x, stat_pad_start_y,
    stat_pad_step_x, stat_pad_step_y;
    

  // temporary vehicle values
  w32 supertank_machine_gun_damage,
      supertank_turret_damage,
      supertank_guided_missile_damage;
    


  w32 disable_a3d;
  w32 disable_sound;
  w32 action_strategy_ticks_to_switch;

  w32 lock_cheat;
  i4_bool paused;      // if the game is paused

  void load();
  void cleanup();
};

extern g1_resource_class g1_resources;


inline i4_float g1_near_z_range() { return 0.01f; }
inline i4_float g1_far_z_range() { return 100.0; }

#endif
