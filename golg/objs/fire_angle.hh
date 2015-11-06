/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_FIRE_ANGLE_HH
#define G1_FIRE_ANGLE_HH

#include "arch.hh"
#include "math/num_type.hh"

i4_bool g1_calc_fire_angle(i4_float x_y_dist, 
                           i4_float z1, 
                           i4_float z2,
                           i4_float g,
                           i4_float vo,
                           i4_float &theta);

#endif
