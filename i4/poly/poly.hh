/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef I4_POLY_HH
#define I4_POLY_HH


#include "error/error.hh"
#include "math/point.hh"

struct i4_vertex_class
{
  w32 outcode;
  i4_3d_point_class v;                         // unprojected x,y,z
  i4_float s, t;                               // texture coordinates 0..1
  i4_float px, py;                             // projected x,y
  i4_float w;                                  // 1/v.z
  i4_float r,g,b;                              // lighting intensity 0..1
  i4_float a;                                  // alpha 0..1

  // w is 1/z, cx and cy are center of window
  void project(i4_float w, i4_float center_x, i4_float center_y)
  {
    px=v.x*w + center_x;
    py=v.y*w + center_y;
  }
};


class i4_polygon_class
{
public:
  enum { V_BUF_SIZE=8 };

  int t_verts;					 // number of poly vertices
  i4_vertex_class vert[V_BUF_SIZE];              // vertex buffer

  i4_polygon_class() { t_verts=0; }

  i4_vertex_class *add_vert(i4_3d_point_class v,
                            i4_float s, i4_float t,
                            i4_float r, i4_float g, i4_float b)
  {
    I4_ASSERT(t_verts<V_BUF_SIZE, "vert buffer full");

    vert[t_verts].v=v;
    vert[t_verts].s=s;
    vert[t_verts].t=t;
    vert[t_verts].r=r;
    vert[t_verts].g=g;
    vert[t_verts].b=b;
    t_verts++;
    return vert+t_verts-1;
  }
  
  i4_vertex_class *add_vert(i4_vertex_class *src_vert)
  {
    I4_ASSERT(t_verts<V_BUF_SIZE, "vert buffer full");

    vert[t_verts] = *src_vert;
    t_verts++;
    return vert+t_verts-1;    
  }
};


#endif
