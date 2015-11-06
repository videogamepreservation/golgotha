/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef ROCKET_HH
#define ROCKET_HH

#include "g1_object.hh"
#include "player_type.hh"
#include "path.hh"

class g1_solid_class;
class g1_map_piece_class;
class g1_smoke_trail_class;

#define G1_ROCKET_DATA_VERSION 1

class g1_rocket_class : public g1_object_class
{
protected:  
  g1_typed_reference_class<g1_map_piece_class> who_fired_me, track_object;
  g1_typed_reference_class<g1_smoke_trail_class> smoke_trail;

  i4_float zv,    // z velocity
    speed;        // velocity in the x-y plane (uses theta for direction)


  i4_float g() { return 0.05; }
  i4_float vo() { return 0.4;   }

public:

  w32 damage;

  // fire_at setups all the rocket variables so it will hit a specific object fired
  // from sx,sy,sz  if the object is too far away the rocket will land in front of the object
  // if owner is not 0, then owner will be notified when the rocket hits or misses
  // this function will also add the rocket to the map and do a request_think()
  virtual void fire_at(i4_float sx, i4_float sy, i4_float sz, 
                       i4_float angle,
                       g1_map_piece_class *kill_target,
                       g1_map_piece_class *owner,
                       w32 damage);
  
  // fire is called to setup the rocket if the rocket is not being fired at anything in
  // particular.  In this case it will travel it's maximum range at the specified angle
  // if owner is not 0, then owner will be notified when the rocket hits or misses
  // this function will also add the rocket to the map and do a request_think()
  virtual void fire(i4_float sx, i4_float sy, i4_float sz, i4_angle angle, 
                    g1_map_piece_class *owner,
                    w32 damage);
  
  g1_rocket_class(g1_object_type id, g1_loader_class *fp);
  virtual void save(g1_saver_class *fp);
  virtual void think();
  virtual void draw(g1_draw_context_class *context);

  virtual i4_bool move(i4_float x_amount,
                       i4_float y_amount,
                       i4_float z_amount);

};

#endif
