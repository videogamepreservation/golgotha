/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_CLIP_HH
#define G1_CLIP_HH

#error "don't use this file anymore"

#include "draw_context.hh"
#include "math/vector.hh"
#include "poly/poly.hh"
#include "poly/polyclip.hh"


inline i4_float g1_clip_delta() { return 0.001; }

inline w8 g1_calc_clip_code(i4_3d_vector &v, i4_float near_z, i4_float far_z, i4_bool clip_far)
{
  float zp = v.z;// + g1_clip_delta();

  w8 clip_code=0;

  if (v.x>zp)  
    clip_code |= 1;
  else if (v.x<-(zp)) 
    clip_code |= 2;
  
  if (v.y>zp )  
    clip_code |= 4;
  else  if (v.y<-(zp)) 
    clip_code |= 8;
 
  if (v.z<near_z)// + g1_clip_delta())   
    clip_code |=16;
  else if (clip_far && v.z>far_z)// - g1_clip_delta()) 
    clip_code |=32; 

  return clip_code;
}


class g1_3d_clip_helper
{
public:

  g1_draw_context_class *context;
  i4_float cz1,cz2;


  g1_3d_clip_helper(g1_draw_context_class *context, i4_float cz1, i4_float cz2) 
    : context(context),
      cz1(cz1),
      cz2(cz2)
  {
  }

  i4_float intersect(i4_vertex_class *i, i4_vertex_class *j, w32 plane)
  {
    switch (plane)
    {
      case I4_CLIP_PLANE_Y2 : // bottom
        return (i->v.y - i->v.z*half_clipy()) / 
          ((j->v.z - i->v.z)*half_clipy() - j->v.y + i->v.y);
        
      case I4_CLIP_PLANE_Y1: // top
        return (- i->v.z*half_clipy() - i->v.y) / 
          ((j->v.z - i->v.z)*half_clipy() + j->v.y - i->v.y);

      case I4_CLIP_PLANE_X2: // right
        return (i->v.x - i->v.z*half_clipx()) / 
          ((j->v.z - i->v.z)*half_clipx() - j->v.x + i->v.x);

      case I4_CLIP_PLANE_X1: // left
        return (- i->v.z*half_clipx() - i->v.x) / 
          ((j->v.z - i->v.z)*half_clipx() + j->v.x - i->v.x);

      case I4_CLIP_PLANE_Z2: // far
        return (cz2 - i->v.z) / (j->v.z - i->v.z);

      case I4_CLIP_PLANE_Z1: // near
        return (cz1 - i->v.z) / (j->v.z - i->v.z);
    }
    return 0;
  }

  void project(i4_vertex_class *i)
  {
    i4_float w = 1.0 / i->v.z;
    i->px = i->v.x*w + context->center_x;
    i->py = i->v.y*w + context->center_y;
  }

};


#endif
