/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef __POINT_HH_
#define __POINT_HH_

#include "arch.hh"
#include "math/vector.hh"

class i4_3d_point_class : public i4_3d_vector
{
public:
  i4_3d_point_class() {}
  i4_3d_point_class(const i4_3d_vector &p) : i4_3d_vector(p) {}
  i4_3d_point_class(i4_float _x, i4_float _y, i4_float _z) : i4_3d_vector(_x, _y, _z) {}

  i4_3d_point_class& operator=(const i4_3d_vector &p) { set(p.x,p.y,p.z); return *this; }

  i4_bool almost_equal(i4_3d_point_class &a, i4_float error=0.001)
  {
    return (i4_bool)
      ((i4_fabs(x - a.x)<error) &&
          (i4_fabs(y - a.y)<error) &&
          (i4_fabs(z - a.z)<error));
  }

  i4_float distance(const i4_3d_point_class &other)
  { return sqrt( (x-other.x)*(x-other.x) + (y-other.y)*(y-other.y) + (y-other.y)*(y-other.y)); }
};

#endif

