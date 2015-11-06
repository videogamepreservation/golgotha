/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_PLAYER_PEICE_HH
#define G1_PLAYER_PEICE_HH

#include "objs/map_piece.hh"
#include "path.hh"
#include "sound/sound.hh"


extern g1_object_type g1_supertank_type;

class g1_buster_rocket_class;
class g1_camera_info_struct;       // golg/camera.hh


// defines info about the current state of a weapon ammo type for each supertank
struct g1_stank_ammo_info_struct
{
  g1_stank_ammo_type_struct *ammo_type;
  w16 amount;           // shots remaining
  w16 delay_remaining;  // delay until next shot
  w16 refuel_delay_remaining;  // delay till next 'fuel'
  i4_bool need_refresh; // if border frame should redraw this ammo slot
  
  g1_typed_reference_class<g1_object_class> current_target;
  int ticks_this_has_been_my_current_target;
  
  void setup(li_symbol *sym);
  
  void save(g1_saver_class *fp);
  void load(g1_loader_class *fp);
  
  // so we can update electric beams & machine gun - continous stream type weapons
  g1_typed_reference_class<g1_object_class> last_fired_object;
  g1_stank_ammo_info_struct() { ammo_type=0; amount=0; need_refresh=i4_T; }
};


class g1_player_piece_class : public g1_map_piece_class
{
public:
  void check_for_powerups();
  void check_for_refuel();
  void check_if_turret_in_ground();
  void draw_target_cursors(g1_draw_context_class *context);
  void look_for_targets();
  
  static g1_player_piece_class *cast(g1_object_class *obj)
  {
    if (!obj || !obj->get_type()->get_flag(g1_object_definition_class::TO_PLAYER_PIECE))
      return 0; 

    return (g1_player_piece_class*)obj;
  }

  //vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
  enum { DATA_VERSION=8 };       // main data version
  enum { BASE_DATA_VERSION=1 };
  void save_base_info(g1_saver_class *fp);
  void load_base_info(g1_loader_class *fp);
  i4_float base_angle, lbase_angle;
  i4_float strafe_speed;
  i4_float cur_top_attach;
  w32      move_tick;       // for vibrating top
  w32      upgrade_level_when_built;
  //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

  // path information
  enum {NO_PATH, GAME_PATH, FINAL_POINT};
  g1_path_handle path;

  // user input
  i4_float accel_ratio;
  i4_float strafe_accel_ratio;
  i4_float dtheta;
  i4_bool fire[3];

  //vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
  enum { MOUSE_LOOK_DATA_VERSION=1 };           
  void save_mouse_look_info(g1_saver_class *fp);
  void load_mouse_look_info(g1_loader_class *fp);

  i4_float mouse_look_increment_x,mouse_look_increment_y;
  i4_float mouse_look_x,mouse_look_y;           
  //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

  //vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
  enum { AMMO_DATA_VERSION=3 };
  void save_ammo_info(g1_saver_class *fp);
  void load_ammo_info(g1_loader_class *fp);
  
  enum { MAX_WEAPONS=4 };
  g1_stank_ammo_info_struct ammo[MAX_WEAPONS];       // 0=main, 1=missiles, 2=chain
  void find_weapons();

  //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


  // vvvvvvvvvvvvvvvvvv --- not saved --- vvvvvvvvvvvvvvvvvvvv  
  i4_float turret_kick, lturret_kick; //just an aesthetic thing, the turret kicks back when fired
  
  s1_sound_handle refuel_sound;

  enum { ORIGNAL_RUMBLE_VOLUME=I4_SOUND_VOLUME_LEVELS/8 };

  enum 
  {
    ST_REFUELING = 1<<0, // set when on refueling pad, will cause blue glow to occur
    ST_DAMAGED   = 1<<1, // set when damaged, controller will use damage_count to make red
    ST_GODMODE   = 1<<2, // invulnerable
    ST_INFLIGHT  = 1<<3, // in the air!
    ST_SLIDING   = 1<<4, // sliding!
  };
         
  w32 stank_flags;


  void toggle_stank_flag(w32 flag)
  {
    if (stank_flags & flag)
      stank_flags &= (~flag);
    else
      stank_flags |= flag;
  }

  i4_bool get_stank_flag(w32 flag)
  {
    return stank_flags & flag;
  }

  void set_stank_flag(w32 flag, i4_bool value=i4_T)
  { 
    if (value) 
      stank_flags |= flag; 
    else 
      stank_flags &= (~flag);
  }

  i4_bool strafe_left();
  i4_bool strafe_right();
  void turn_right();
  void turn_left();
  void accelerate_forward();
  void accelerate_backward();
  void track_base(i4_float desired_angle);


  // get the position where the bullet where leave the barel and the direction it travels
  void get_bullet_exit(i4_3d_vector &pos, i4_3d_vector &dir);

  enum find_view_type { USE_SLOP, USE_SCREEN };
  g1_object_class *find_view_target(const i4_3d_vector &view_pos,
                                    const i4_3d_vector &view_dir,
                                    find_view_type type,
                                    float max_dist);

  i4_bool in_range(int slot_number, g1_object_class *o) const;
  i4_bool fire_weapon(int slot_number);
  void save(g1_saver_class *fp);

  virtual void draw(g1_draw_context_class *context);
  virtual void think();
  void set_path(g1_path_handle _path);
  virtual i4_bool deploy_to(float x, float y);
  w32 follow_path();
  i4_bool move(i4_float x_amount, i4_float y_amount);
  i4_bool suggest_move(i4_float &dist,
                       i4_float &dtheta,
                       i4_float &dx, i4_float &dy,
                       i4_float brake_speed=0.1);
  void lead_target(i4_3d_point_class &lead_point, int slot_number);

  //override the damage so that we can turn on god mode
  virtual void damage(g1_object_class *obj, int hp, i4_3d_vector damage_dir);

  //override this so we can keep kill / damage stats
  virtual void notify_damage(g1_object_class *obj, sw32 hp);

  g1_player_piece_class(g1_object_type id, g1_loader_class *fp);
  ~g1_player_piece_class();

  class g1_mini_object *turret;
  
  virtual void calc_action_cam(g1_camera_info_struct &cam, float frame_ratio);
  
  //this is the current "lock" target (the one most near the center of view)
  g1_typed_reference_class<g1_object_class> lock_target;

  g1_typed_reference_class<g1_object_class> laser_target; 
};

#endif
