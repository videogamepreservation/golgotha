/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "g1_object.hh"

enum
{  
  G1_SUPERTANK            = 0,  
  G1_PEON_TANK            = 1,
  G1_ENGINEER             = 2,
  G1_ROCKET_TANK          = 3,
  G1_EXPLOSION1           = 4,
  G1_BULLET               = 5,
  G1_ROCKET               = 6,
  G1_SUPERTANK_TURRET     = 7,
  G1_JET                  = 8,
  G1_ELECTRIC_CAR         = 9,
  G1_HELICOPTER           = 10,
  G1_ENEMY_ENGINEER       = 11,  
  G1_LIGTBULB             = 12,
  G1_VERY_BIG_GUN         = 13,
  
  G1_CONVOY               = 19,

  G1_GUIDED_MISSILE       = 32,
  G1_SHRAPNEL             = 33,
  G1_SMOKE_TRAIL          = 34,
  G1_JET_BOMB             = 35,
  G1_TOTAL_TYPES
};

g1_object_type *g1_get_old_object_type_remap()
{

  g1_object_type *a=(g1_object_type *)i4_malloc(sizeof(g1_object_type) * G1_TOTAL_TYPES, 
                                                "old obj_remap");

  for (int i=0; i<G1_TOTAL_TYPES; i++)
    a[i]=-1;

  a[G1_SUPERTANK]          = g1_get_object_type("stank");
  a[G1_PEON_TANK]          = g1_get_object_type("peon_tank");
  a[G1_ENGINEER]           = g1_get_object_type("engineer");
  a[G1_ROCKET_TANK]        = g1_get_object_type("rocket_tank");
  a[G1_EXPLOSION1]         = g1_get_object_type("explosion1");
  a[G1_BULLET]             = g1_get_object_type("bullet");
  a[G1_ROCKET]             = g1_get_object_type("rocket");
  a[G1_JET]                = g1_get_object_type("jet");
  a[G1_ELECTRIC_CAR]       = g1_get_object_type("electric_car");
  a[G1_LIGTBULB]           = g1_get_object_type("lightbulb");
  a[G1_VERY_BIG_GUN]       = g1_get_object_type("very_big_gun");
  a[G1_CONVOY]             = g1_get_object_type("convoy");
  a[G1_GUIDED_MISSILE]     = g1_get_object_type("guided_missile");
  a[G1_SHRAPNEL]           = g1_get_object_type("shrapnel");
  a[G1_SMOKE_TRAIL]        = g1_get_object_type("smoke_trail");
  a[G1_JET_BOMB]           = g1_get_object_type("dropped_bomb");  

  return a;
}
