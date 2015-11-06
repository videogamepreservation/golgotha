/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_MAP_VIS_HH
#define G1_MAP_VIS_HH

#include "arch.hh"
#include "math/num_type.hh"
#include "math/transform.hh"
#include "poly/poly.hh"

class i4_image16;

class g1_visible_projection
{
  i4_float cx1,cy1,cx2,cy2;
  i4_transform_class *trans;

public:
  // used by poly_clip template function
  i4_float intersect(i4_vertex_class *v1, i4_vertex_class *v2, int plane);
  void project(i4_vertex_class *v)
  {
    v->px=v->v.x;
    v->py=v->v.y;
  }
 
  enum { MAX_CELLS=3000 };

  struct point { i4_float x,y; };

  struct cell
  {
    w16 x,y;
    i4_float z;
  };

  cell list[MAX_CELLS];

  w32 t_cells;
  
  void add(w16 x, w16 y, i4_float z)
  {
    if (t_cells<MAX_CELLS)
    {
      list[t_cells].x=x;
      list[t_cells].y=y;
      list[t_cells].z=z;
      t_cells++;
    }    
  }

  // used by poly_draw template function
  i4_bool do_z() { return i4_T; }       // do you want w value for each pixel write? (1/z)
  i4_bool do_st() { return i4_F; }      // do you want texture coordinates?
  int do_light() { return 0; }          // return 0 for no light
  i4_bool do_p() { return i4_F; }       // no perspective
  i4_bool do_spans() { return i4_F; }   // no unrolling


  class screen_pointer
  {
    int x,y;
    g1_visible_projection *v;

  public:
    void move_to(int _x, int _y, g1_visible_projection &vp) { x=_x; y=_y; }
    screen_pointer &operator+=(int v) { x+=v; return *this; }
    void next_line(g1_visible_projection &vp) { y++; }
    int get_twidth(g1_visible_projection &ref)
    {
      return 0;
    }

    w16 * get_ptr(g1_visible_projection &ref)
    {
      return NULL;
    }

    i4_float * get_lookup(g1_visible_projection &ref)
    {
      return NULL;
    }

    w16 * get_tptr(g1_visible_projection &ref)
    {
      return NULL;
    }

    w32 get_screen_width(g1_visible_projection &ref)
    {
      return 0;
    }

    void write(int s, int t, 
               i4_float w, 
               i4_float r, i4_float g, i4_float b,
               g1_visible_projection &vp)
    {
      vp.add(x,y,w);
    } 
  };


  g1_visible_projection(i4_transform_class &t,
                        w32 n_points,
                        point *p,
                        w32 map_w, w32 map_h);
};


#endif





