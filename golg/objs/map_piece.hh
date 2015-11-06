/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

// Golgatha Map Object Class
//

#ifndef MAP_PIECE_HH
#define MAP_PIECE_HH

#include "g1_object.hh"
#include "sound_man.hh"
#include "player_type.hh"
#include "range.hh"
#include "objs/defaults.hh"
#include "objs/model_draw.hh"
#include "objs/model_collide.hh"
#include "obj3d.hh"
#include "global_id.hh"
#include "sound/sfx_id.hh"
#include "objs/vehic_sounds.hh"

class g1_path_object_class;     // objs/path_object.hh

class g1_map_piece_class : public g1_object_class
{
public:
  void fix_forward_link(g1_object_class *next) { next_object = next; }
  void fix_previous_link(g1_object_class *prev) { prev_object = prev; }
protected:
  friend g1_path_object_class;

  g1_typed_reference_class<g1_object_class> next_object;
  g1_typed_reference_class<g1_object_class> prev_object;
  i4_float path_pos, path_len;
  i4_float path_cos, path_sin, path_tan_phi;
  i4_float stagger;

  void unlink();
  void link(g1_object_class *origin);
public:
  g1_object_defaults_struct *defaults;
  
  i4_float           speed;          // current speed of the piece
  i4_float           vspeed;         // vertical speed of the piece
  i4_float           dest_x, dest_y; // if it has a destination, its point & direction is here
  i4_float           dest_z;         // if it has a destination height
  i4_float           dest_theta;
  w16                fire_delay;     // number of game ticks before we can fire again

  g1_id_ref         *path_to_follow;  // null terminated array of global id's
  g1_id_ref          next_path;

  // calculated values
  i4_float           tread_pan;      // accumulated tread texture pan based on speed
  i4_float           terrain_height; // terrain under vehicle
  i4_float           groundpitch, lgroundpitch;
  i4_float           groundroll, lgroundroll;
  i4_float           damping_fraction;
  w8                 ticks_to_blink;   // when taking damage (not saved)

  w8                 death;          // type of death desired

  i4_3d_vector       damage_direction; //inertia vector of whatever attacked last (or killed us)

  g1_typed_reference_class<g1_object_class> attack_target;  // the thing its targeting
  
  g1_map_piece_class(g1_object_type id, g1_loader_class *fp);

  void add_team_flag();
  i4_bool alive() const { return health>0; }

  virtual void hit_ground();      // when an object falls off a cliff and hits the ground
  void find_target(i4_bool unfog=i4_T);
  virtual i4_bool find_target_now() const;

  virtual void damage(g1_object_class *obj, int hp, i4_3d_vector damage_dir);

  void lead_target(i4_3d_point_class &lead_point, i4_float shot_speed=-1);
  
  virtual void save(g1_saver_class *fp);

  virtual float detection_range() const { return defaults->detection_range; }
  
  virtual i4_bool check_move(i4_float dest_x,i4_float dest_y) const;

  virtual i4_bool check_collision(const i4_3d_vector &start, i4_3d_vector &ray)
  { return g1_model_collide_radial(this, draw_params, start, ray); }

  virtual void calc_world_transform(i4_float ratio, i4_transform_class *transform=0);

  //Sound Effects

  virtual void set_path(g1_id_ref *path);  // null terminated array of global id's

  
  g1_rumble_type rumble_type;
  void init_rumble_sound(g1_rumble_type type);

  virtual void init();

  virtual void think();



  virtual void draw(g1_draw_context_class *context);

  i4_bool check_turn_radius();

#if 0 //(OLI) kill this soon! 
  //advances a piece one step to its dest_x,dest_y, following general guidelines
  //for speeding up, slowing down, backing up, etc
  //returns which direction (-1,0,1) it moved and which direction (-1,0,1) it turned
  //in arguments ret_direction and ret_dt
  i4_float advance_to_dest(w32 pathinfo, sw8 &ret_direction, sw8 &ret_dt,
                           i4_float brake_speed=0.9);
#endif

  virtual void request_remove();

  void advance_path();

  i4_bool suggest_move(i4_float &dist,
                       i4_float &dtheta,
                       i4_float &dx, i4_float &dy,
                       i4_float brake_speed=0.1,
                       i4_bool reversible=i4_T);
  
  i4_bool suggest_air_move(i4_float &dist,
                           i4_float &dtheta,
                           i4_3d_vector &d);
  
  //checks the health to make sure the object is still alive
  //if health is <0, the object is removed from the map, and i4_F returned
  i4_bool check_life(i4_bool remove_if_dead=i4_T);

  void get_terrain_info();
  i4_bool occupy_location()
  {
    int ret;
    if (ret = g1_object_class::occupy_location()) 
      get_terrain_info(); 
    return ret;
  }

  virtual void grab_old()
  {
    g1_object_class::grab_old();
    
    lgroundpitch = groundpitch;
    lgroundroll  = groundroll;
  }

  // standard movement
  i4_bool move(i4_float x_amount, i4_float y_amount);

  static g1_map_piece_class *cast(g1_object_class *obj)
  {
    if (!obj || !(obj->get_type()->get_flag(g1_object_definition_class::TO_MAP_PIECE)))
      return 0;
    return (g1_map_piece_class*)obj;
  }

  virtual i4_bool can_attack(g1_object_class *who) const;
  i4_bool in_range(g1_object_class *o) const;
};

#endif
