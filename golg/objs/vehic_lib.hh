/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_VEHIC_LIB_HH
#define G1_VEHIC_LIB_HH

#include "arch.hh"
#include "math/num_type.hh"
#include "math/pi.hh"
#include "math/angle.hh"

//helper functions for vehicle AI

//rotates and snaps angle FROM to angle TO at turnspeed SPEED
//returns the direction it rotated (-1,0,or 1)
i4_float i4_rotate_to(i4_float &from, i4_float to, i4_float speed);

//snaps angle FROM so that its between 0 and 2pi

#endif
