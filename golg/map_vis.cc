/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "map_cell.hh"
#include <stdlib.h>
#include "poly/polyclip.hh"
#include "poly/polydraw.hh"
#include "map.hh"
#include "math/transform.hh"
#include "time/profile.hh"
#include "r1_api.hh"
#include "g1_render.hh"
#include "tile.hh"
#include "map_vert.hh"
#include "map_view.hh"

static i4_profile_class pf_map_calc_visible("map::calc_visible");

static int cell_compare(const void *a, const void *b)
{
  g1_map_vertex_class *v1=  g1_verts + ((g1_visible_cell *)a)->x + 
    ((g1_visible_cell *)a)->y * g1_map_width_plus_one;

  g1_map_vertex_class *v2=  g1_verts + ((g1_visible_cell *)b)->x + 
    ((g1_visible_cell *)b)->y * g1_map_width_plus_one;

  if (v1->v.z<v2->v.z)
    return 1;
  else if (v1->v.z>v2->v.z)
    return -1;
  else return 0;
}


class g1_map_clip_class
{
public:
  i4_float cx2, cy2;

  g1_map_clip_class(i4_float cx2, i4_float cy2) 
    : cx2(cx2), cy2(cy2) {}
  
  void project(i4_vertex_class *v)
  {
    v->px=v->v.x;
    v->py=v->v.y;
  }
  
  // used by poly_clip template function
  i4_float intersect(i4_vertex_class *v1, i4_vertex_class *v2, int plane)
  {
    switch (plane)
    {
      case I4_CLIP_PLANE_Z1 :
      case I4_CLIP_PLANE_Z2 :
        return 0;

      case I4_CLIP_PLANE_Y1 :
      {
        if (v1->v.y < v2->v.y)
          return (0.0-v1->v.y)/(v2->v.y-v1->v.y);
        else
          return (v1->v.y)/(v1->v.y-v2->v.y);
      } break;

      case I4_CLIP_PLANE_Y2 :
      {
        if (v1->v.y < v2->v.y)
          return (cy2-v1->v.y)/(v2->v.y-v1->v.y);
        else
          return (v1->v.y-cy2)/(v1->v.y-v2->v.y);
      } break;

      case I4_CLIP_PLANE_X1 :
      {
        if (v1->v.x < v2->v.x)
          return (0.0-v1->v.x)/(v2->v.x-v1->v.x);
        else
          return (v1->v.x)/(v1->v.x-v2->v.x);
      } break;

      case I4_CLIP_PLANE_X2 :
      {
        if (v1->v.x < v2->v.x)
          return (cx2-v1->v.x)/(v2->v.x-v1->v.x);
        else
          return (v1->v.x-cx2)/(v1->v.x-v2->v.x);
      } break;

    }
    return 0;
  }

};

class g1_vert_transformer
{
public:
  w32 max_cells;
  w32 t_cells;
  g1_visible_cell *cell_list;
  i4_transform_class t;
  g1_map_vertex_class *verts;
  int map_width_p1;
  i4_float xscale, yscale;

  void add_cell(w8 x, w8 y)
  {
    if (t_cells<max_cells)
    {
      cell_list[t_cells].x=x;
      cell_list[t_cells].y=y;
      t_cells++;
    }
  }

  g1_vert_transformer(i4_transform_class &trans,
                      g1_map_vertex_class *verts,
                      int map_width_p1,
                      g1_visible_cell *cell_list,
                      w32 max_cells,
                      i4_float xscale, i4_float yscale)


    : verts(verts), map_width_p1(map_width_p1), 
      cell_list(cell_list), max_cells(max_cells),
      xscale(xscale), yscale(yscale)
  {
    t=trans;
    t_cells=0;    
  }

  int feature(int x) { return 0; }

  class screen_pointer
  {

  public:
    i4_3d_vector ground;
    g1_map_vertex_class *v;
    w8 x,y;

    screen_pointer& operator=(const screen_pointer& p)
    {
      ground=p.ground;
      v=p.v;
      x=p.x;
      y=p.y;
      return *this;
    }

    void move_to(int _x, int _y, g1_vert_transformer &vp) 
    { 
      x=_x;
      y=_y;
      i4_3d_point_class g=i4_3d_point_class(_x,_y,0);
      vp.t.transform(g, ground);
      v=vp.verts + _x + _y * vp.map_width_p1;
    }

    void add(int amount, g1_vert_transformer &vp) 
    { 
      x+=amount;

      ground.x += vp.t.x.x * amount;
      ground.y += vp.t.x.y * amount;
      ground.z += vp.t.x.z * amount;

      v+=amount;
    }

    void next_line(g1_vert_transformer &vp) 
    {
      y++; 
      ground.x+=vp.t.y.x;
      ground.y+=vp.t.y.y;
      ground.z+=vp.t.y.z;

      v+=vp.map_width_p1;
    }
    int get_twidth(g1_vert_transformer &ref)
    {
      return 0;
    }

    w16 * get_ptr(g1_vert_transformer &ref)
    {
      return NULL;
    }

    i4_float * get_lookup(g1_vert_transformer &ref)
    {
      return NULL;
    }

    w16 * get_tptr(g1_vert_transformer &ref)
    {
      return NULL;
    }

    w32 get_screen_width(g1_vert_transformer &ref)
    {
      return 0;
    }

    void write(int s, int t, 
               i4_float w, 
               i4_float r, i4_float g, i4_float b, i4_float a,
               g1_vert_transformer &vp)
    {
      if ((v->flags & g1_map_vertex_class::APPLY_WAVE_FUNCTION)==0)
      {
        i4_float h=v->get_height();
        i4_3d_vector v_air;
        v_air.x = (ground.x + h * vp.t.z.x) * vp.xscale;
        v_air.y = (ground.y + h * vp.t.z.y) * vp.yscale;
        v_air.z = ground.z + h * vp.t.z.z;
      
      //      v->transform(vp.t, x,y);

        v->v=v_air;
      }
      else 
      {
        v->wave_transform(vp.t, x, y);
        v->v.x *= vp.xscale;
        v->v.y *= vp.yscale;
      }


      v->set_is_transformed(1); 

      vp.add_cell(x,y);
    } 
  };  
};



int g1_map_class::calc_visible(i4_transform_class &t,
                               i4_polygon_class *area_poly,
                               g1_visible_cell *buffer, w32 buf_elements,
                               i4_float xscale, i4_float yscale)
{ 
  pf_map_calc_visible.start();

  int ret=0,i;

  i4_polygon_class poly, clipped_poly;

  for (i =0; i<area_poly->t_verts; i++)
  {
    area_poly->vert[i].px=area_poly->vert[i].v.x;
    area_poly->vert[i].py=area_poly->vert[i].v.y;
  }

  poly=*area_poly;
  g1_map_clip_class cell_clip(width(), height());
  i4_poly_clip(cell_clip, poly, clipped_poly,
               (float)0.0,(float)0.0,(float) -1.0, 
               (float)(width()), 
               (float)(height()), (float)1.0);


  if (clipped_poly.t_verts>=3)
  {
    g1_vert_transformer vt(t, 
                           verts, width()+1, 
                           buffer, buf_elements,
                           xscale, yscale);


    i4_poly_draw_class<g1_vert_transformer> draw(vt, clipped_poly);
    ret=vt.t_cells;
  }


  for (i=0; i<ret; i++)
    if (buffer[i].x>=width() || buffer[i].y>=height())
    {
      i4_warning("bad cell");
      return 0;
    }
      

  g1_radar_looking_at(clipped_poly.vert[0].v.x,
                      clipped_poly.vert[0].v.y,
                      clipped_poly.vert[2].v.x,
                      clipped_poly.vert[2].v.y);
                      

  if (ret)
  {
    //if we're using a software rasterizer, must sort the cells (technically not necessary anymore)
    //if (g1_render.r_api->get_render_device_flags() & R1_SOFTWARE)
      //qsort(buffer, ret, sizeof(g1_visible_cell), cell_compare);
    
    //else
      //sort the cells by texture
      //qsort(buffer, ret, sizeof(g1_visible_cell), cell_texture_compare);
  }

  pf_map_calc_visible.stop();
  return ret;
}



