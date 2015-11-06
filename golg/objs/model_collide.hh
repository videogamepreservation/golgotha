/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_MODEL_COLLIDE_HH
#define G1_MODEL_COLLIDE_HH

#include "objs/model_draw.hh"
#include "math/point.hh"

i4_bool g1_model_collide_polygonal(g1_object_class *_this,
                                   g1_model_draw_parameters &params,
                                   const i4_3d_vector &start,
                                   i4_3d_vector &ray,
                                   i4_3d_vector &hit_normal,
                                   i4_float *minimum_t=0);

i4_bool g1_model_collide_radial(g1_object_class *_this,
                                g1_model_draw_parameters &params,
                                const i4_3d_vector &start,
                                i4_3d_vector &ray);

#endif
