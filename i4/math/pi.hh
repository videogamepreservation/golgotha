/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef I4_PI_HH
#define I4_PI_HH

#include "math/num_type.hh"

inline i4_float i4_pi() { return 3.141592654; }
inline i4_float i4_2pi() { return 2.0*3.141592654; }
inline i4_float i4_deg2rad(i4_float deg) { return deg/180.0*i4_pi(); }

#endif

