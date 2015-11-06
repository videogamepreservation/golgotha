/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_CAMERA_HH
#define G1_CAMERA_HH

#include "math/num_type.hh"
#include "math/vector.hh"
#include "reference.hh"
#include "g1_object.hh"

class g1_loader_class;     // golg/saver.hh
class g1_saver_class;      // golg/saver.hh
class i4_transform_class;  // i4/math/transform.hh
class g1_object_class;     // golg/g1_object.hh

enum g1_view_mode_type
{
  G1_EDIT_MODE,          // camera is set exactly by someone without outside influence
  G1_STRATEGY_MODE,      // camera looks down from fixed hieght above ground
  G1_ACTION_MODE,        // camera follow objects from 1st person
  G1_FOLLOW_MODE,        // camera follows objects from 3rd person
  G1_CAMERA_MODE,        // camera follows spline set down by the editor
  G1_WATCH_MODE,         // camera wanders around looking at interesting things
  G1_CIRCLE_WAIT         // camera is circling an object/location until the user presses a key
};

enum g1_watch_type       // priorities for the camera
{
  G1_WATCH_INVALID,      // indicates camera event is not in use
  G1_WATCH_IDLE,         // lowest priority, vehicles moving
  G1_WATCH_EVENT,        // heli take off, bridge assembly, etc.
  G1_WATCH_FIRE,         // shots fired
  G1_WATCH_HIT,          // someone got hit
  G1_WATCH_EXPLOSION,    // someone blew up
  G1_WATCH_USER,         // user wants top look here
  G1_WATCH_FORCE         // force the camera here (base blown up)
};

  

struct g1_camera_info_struct
{
  i4_float gx, gy, gz;
  i4_float ground_x_rotate, ground_y_rotate;  
  i4_float ground_rotate,      // rotation about game z
    horizon_rotate,     // rotation up/down
    roll;

  void defaults(); 
  g1_camera_info_struct() { defaults(); }
  void load(g1_loader_class *fp);
  void save(g1_saver_class *fp);

};

struct g1_camera_event
{
  g1_typed_reference_class<g1_object_class> camera_at;
  g1_watch_type type;       // type of camera shot this is
  // object we are sort of tracking (0 if not tracking)
  g1_typed_reference_class<g1_object_class> follow_object;        
  int min_time, max_time;   // min/max times to sustain camera shot (in ticks)
  int time_elapsed;         // time that we've spent on this shot already (in ticks)

  g1_camera_event();
  void object_ids_changed();   // call after level is reloaded
};


struct g1_view_state_class
{
  enum { DATA_VERSION=1 };
  
  i4_3d_vector move_offset;    // next time the camera updates it will move this distance (not z)

  struct circle_info
  {
    float zvel,
          theta,  theta_vel,
          dist, dist_vel;
          
    i4_3d_vector looking_at;
  } circle;


  g1_typed_reference_class<g1_object_class> camera1, camera2;

  g1_camera_event next_cam, current_cam;

 

  
  w32 follow_object_id;
  w32 ticks_to_watch_spot;
  
  g1_view_mode_type view_mode;
  g1_watch_type watch_type;
  
  g1_camera_info_struct start,        // when the camera is zooming from one location to another
                       end;

  // where is the camera between start and end? 1.0 = end, 0.0 = start
  float start_end_interpolate_fraction;

  // how much is added to the above fraction in each tick, calculated by how many ticks
  // you want the camera to take to get to it's destination
  float start_end_interpolate_fraction_step;  

  // do not set this directly, it is calculated from start and end
  g1_camera_info_struct current;
    

  i4_bool start_invalid;      // when the game first starts, the start camera is invalid

  
  void defaults();
  
  void update_circle_mode();
  void update_follow_mode();
  void update_action_mode();
  void update_watch_mode();
  void update_camera_mode();
  void update_strategy_mode();
  void update_edit_mode();
  void use_next_cam();
public:
  i4_transform_class *get_transform();
  void get_camera_pos(i4_3d_vector &v);
  // returns the sqaured distance to the camera
  float dist_sqrd(const i4_3d_vector &v);  

  void load(g1_loader_class *fp);
  void save(g1_saver_class *fp);
 
  g1_view_state_class();  
  g1_camera_info_struct *get_camera();    // calculates current and returns a pointer to it
  void update();                   // should be called after every tick, updates end camera pos
  
  g1_view_mode_type get_view_mode() { return view_mode; }
  void set_view_mode(g1_view_mode_type mode) { view_mode=mode; }

  void calc_transform(i4_transform_class &t);

  void suggest_camera_mode(g1_view_mode_type mode,
                           w32 follow_object_global_id=0);

  // called by radar map
  void set_camera_position(float game_x, float game_y);
  
  void suggest_camera_event(g1_camera_event &event);

  void rotate(i4_float about_game_z, i4_float up_down);
  void pan(i4_float x, i4_float y, i4_float z);
  void zoom(float dist);
};

g1_view_state_class *g1_current_view_state();

#endif







