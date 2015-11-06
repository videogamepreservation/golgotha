/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef _G1_FRUSTRUM_CLIP_HH
#define _G1_FRUSTRUM_CLIP_HH

#error "don't use this file anymore"

class i4_polygon_class;

#include "arch.hh"

  //clips x,y,z,s,t,r,g,and b
i4_polygon_class *g1_full_clip(i4_polygon_class *src, i4_polygon_class *dest,
                               w32 center_x, w32 center_y);

  //only clips x,y,z
i4_polygon_class *g1_geometric_clip(i4_polygon_class *src, i4_polygon_class *dest,
                                    w32 center_x, w32 center_y);

i4_polygon_class *g1_fast_full_clip(i4_polygon_class *src, 
                                    i4_polygon_class *dest,
                                    w32 center_x, w32 center_y);

#endif

