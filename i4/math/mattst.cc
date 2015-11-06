/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifdef _MANGLE_INC
#include "TRANS~W4.HH"
#else
#include "transform.hh"
#endif
#include "math/pi.hh"

main()
{
  i4_transform_class ma,mb,mc;
  i4_3d_vector a,b,c;

  a.set(2,1,1);
  b.set(1,2,1);

  ma.identity();
  ma.transform(a,c);
  ma.transform(b,c);

  mb.translate(1.0,1.0,1.0);
  mb.transform(a,c);
  mb.transform(b,c);

  mc.multiply(ma,mb);
  mc.transform(a,c);
  mc.transform(b,c);

  mb.rotate_x(i4_pi());
  mb.transform(a,c);
  mb.transform(b,c);

  ma.multiply(mc,mb);
  ma.transform(a,c);
  ma.transform(b,c);
}
