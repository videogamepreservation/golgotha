/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_VERT_HH
#define G1_VERT_HH

#include "math/point.hh"
#include "math/vector.hh"

class g1_vert_class
{
public:
  i4_3d_point_class v;         // x,y,z position of point
  i4_3d_vector      normal;    // normal of vert, used for lighting
};


#endif
