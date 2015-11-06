/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef I4_POLY_CLIP_HH
#define I4_POLY_CLIP_HH

#include "poly/poly.hh"

enum { I4_CLIP_PLANE_Z1=32,
       I4_CLIP_PLANE_Z2=16,
       I4_CLIP_PLANE_Y1=8,
       I4_CLIP_PLANE_Y2=4,
       I4_CLIP_PLANE_X1=2,
       I4_CLIP_PLANE_X2=1 };


static inline int i4_clip_get_side_on(i4_float x1, i4_float y1, i4_float z1,
                                      i4_float x2, i4_float y2, i4_float z2,
                                      i4_vertex_class *v,
                                      int plane)
{
  switch (plane)
  {
    case I4_CLIP_PLANE_Z1 : return v->v.z>z1;
    case I4_CLIP_PLANE_Z2 : return v->v.z<z2;
    case I4_CLIP_PLANE_Y1 : return v->py>y1;
    case I4_CLIP_PLANE_Y2 : return v->py<y2;
    case I4_CLIP_PLANE_X1 : return v->px>x1;
    case I4_CLIP_PLANE_X2 : return v->px<x2;
  }
  return 0;
}



template <class class_with_intersect_and_project_functions>
void i4_poly_clip(class_with_intersect_and_project_functions &cwipf,
                  i4_polygon_class &poly,
                  i4_polygon_class &output_poly,
                  i4_float x1, i4_float y1, i4_float z1,
                  i4_float x2, i4_float y2, i4_float z2)
{
  i4_polygon_class other, *p1, *p2, *swap;
  p1=&poly; p2=&other;

  int prev, cur, prev_side_on, cur_side_on;

  w32 plane=I4_CLIP_PLANE_Z1;
  while (plane)
  {
    p2->t_verts=0;
    prev=p1->t_verts-1;
    prev_side_on=i4_clip_get_side_on(x1, y1, z1, x2, y2, z2, p1->vert+prev, plane);

    for (cur=0; cur<p1->t_verts; cur++)
    {
      i4_vertex_class *v2=p1->vert+cur, *o;
      i4_vertex_class *v1=p1->vert+prev;

      cur_side_on=i4_clip_get_side_on(x1, y1, z1, x2, y2, z2, p1->vert+cur, plane);

      if (prev_side_on ^ cur_side_on)  // are they on different sides?
      {
        i4_float t=cwipf.intersect(v1, v2, plane);
        if (t<0) t=0; 
        else if (t>1) t=1;
        o=p2->add_vert(i4_3d_point_class(v1->v.x + (v2->v.x-v1->v.x)*t, 
                                         v1->v.y + (v2->v.y-v1->v.y)*t, 
                                         v1->v.z + (v2->v.z-v1->v.z)*t),
                       v1->s + (v2->s-v1->s)*t,  
                       v1->t + (v2->t-v1->t)*t,
                       v1->r + (v2->r-v1->r)*t,
                       v1->g + (v2->g-v1->g)*t,
                       v1->b + (v2->b-v1->b)*t);

        cwipf.project(o);   // reproject the new vertex, it's v.x & v.y are different
      }

      if (cur_side_on)
      {
        o=p2->add_vert(v2->v, v2->s, v2->t, v2->r, v2->g, v2->b);
        o->px=v2->px;
        o->py=v2->py;   // copy the projected x & y
      }

      prev=cur;
      prev_side_on=cur_side_on;
    }

    plane=plane>>1;

    if (p2==&other)
    {
      p2=&output_poly;
      p1=&other;
    }
    else
    {
      p1=&output_poly;
      p2=&other;
    }
  }
}


#endif
