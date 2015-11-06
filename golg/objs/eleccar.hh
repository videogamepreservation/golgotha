/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_ELECTRIC_CAR_HH
#define G1_ELECTRIC_CAR_HH

#include "objs/map_piece.hh"

class g1_electric_car_class : public g1_map_piece_class
{
 
public:
  enum {DATA_VERSION=2};

  enum {NUM_ARC_POINTS=5};
  
  typedef struct
  {
    i4_3d_vector lposition,position;
  } arc_point;
  
  void new_arc_points(const i4_3d_vector &laser1, 
                      const i4_3d_vector &laser2, 
                      const i4_3d_vector &target);
  void copy_old_points();

  arc_point arc_points1[NUM_ARC_POINTS];
  arc_point arc_points2[NUM_ARC_POINTS];
  i4_bool   firing;

  ~g1_electric_car_class();

  g1_electric_car_class(g1_object_type id, g1_loader_class *fp);
  virtual void save(g1_saver_class *fp);
    
  enum electric_car_strategy {SIT,RELOAD,REFUEL,ATTACK,KAMIKAZE};
  
  virtual void die();
  virtual void fire();  //FIRE FIRE FIRE!!!!
  virtual void post_think();

  virtual void draw(g1_draw_context_class *context);  

  electric_car_strategy strategy;
  g1_mini_object        *wheel;
  g1_mini_object        *gun_left;
  g1_mini_object        *gun_right;

  
  i4_bool can_attack(g1_object_class *who) const;
};

#endif
