/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_SKY_HH
#define G1_SKY_HH

#include "memory/array.hh"
#include "string/string.hh"

extern i4_array<i4_str *> g1_sky_list;

class i4_window_class;
struct g1_camera_info_struct;
class g1_draw_context_class;
class i4_transform_class;

void g1_draw_sky(i4_window_class *window,
                 g1_camera_info_struct &current_camera,
                 i4_transform_class &transform,
                 g1_draw_context_class *context);


#endif
