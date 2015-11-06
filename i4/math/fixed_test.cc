/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifdef _MANGLE_INC
#include "FIXED~HA.HH"
#else
#include "fixed_point.hh"
#endif
#include <stdio.h>

i4_fixed_point<16> a,b;
i4_fixed_point<7> c,d,e,f,g,h;

main()
{
  a = 1.05;
  b = 5;
  i4_fixed_point_convert(c,a+b);
  d = 4;
  e = 0.3;
  f = e+0.6;
  f -= 0.3;
  g = d*e/f;
  printf("%f + %f = %f\n",double(a),double(b),double(c));
  printf("%f * %f / %f = %f\n",double(d),double(e),double(f),double(g));

  printf("%f / %f = %f\n",double(a), double(b), double(a.divide(b,3)));
}
