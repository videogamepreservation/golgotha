/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef GT1_VISIBILE_HH
#define GT1_VISIBILE_HH

#include "arch.hh"
class g1_map_class;

i4_bool g1_visibility_check(sw32 x1, sw32 y1, 
                            sw32 &x2, sw32 &y2, 
                            sw32 &hit_x, sw32 &hit_y,
                            g1_map_class *map);

#endif
