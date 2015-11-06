/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_FLARE_HH
#define G1_FLARE_HH

#include "math/vector.hh"
class g1_draw_context_class;

void g1_draw_flare(i4_3d_point_class world_pos, g1_draw_context_class *context, 
                   float flare_scale=1.0,
                   int flare_type=0);
                   

void g1_draw_exhaust(i4_3d_point_class world_pos, g1_draw_context_class *context, 
                     float flare_scale=1.0,
                     int flare_type=0,
                     float alpha=1.0);

#endif
