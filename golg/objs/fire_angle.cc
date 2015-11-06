/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "objs/fire_angle.hh"
#include "math/pi.hh"
#include <math.h>

i4_bool g1_calc_fire_angle(i4_float x_y_dist, 
                           i4_float z1, 
                           i4_float z2,
                           i4_float g,
                           i4_float vo,
                           i4_float &theta)
{  
  if (z1!=z2)
  {
    i4_float z_dist=z2-z1;
    i4_float b=x_y_dist * sqrt(1 - (4 * z_dist * g)/(2 * vo*vo));
    i4_float c=(2 * z_dist);

    // ignore the +b solution because it occurs in negative time

    i4_float cos_theta2=(x_y_dist - b)/c;
    if (cos_theta2>=-1 && cos_theta2<=1)      
    {
      theta=acos(cos_theta2);
      return i4_T;
    }
    return i4_F;
  }
  else
  {
    i4_float cos_theta=g * x_y_dist / (2 * vo*vo);

    if (cos_theta>=-1 && cos_theta<=1)
    {
      theta=acos(cos_theta);
      if (theta>=i4_pi()/4)
        theta=i4_pi()/2-theta;


      return i4_T;
    }
    return i4_F;
  }
}
