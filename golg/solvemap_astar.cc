/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include <math.h>
#include "solvemap_astar.hh"
#include "memory/malloc.hh"
#include "error/error.hh"
#include "map.hh"
#include "map_man.hh"
#include "time/profile.hh"

i4_profile_class pf_solve_astar("solve_astar");

const i4_float sqrt2 = sqrt(2.0);

enum { VISITED = g1_map_cell_class::SCRATCH1, OK = g1_map_cell_class::SCRATCH2 };

inline void visit(w16 x, w16 y) 
{
  g1_get_map()->cell(x,y)->flags |= VISITED;
}

inline void ok(w16 x, w16 y) 
{
  g1_get_map()->cell(x,y)->flags |= OK;
}

inline i4_bool is_visited(w16 x, w16 y) 
{
  return (g1_get_map()->cell(x,y)->flags & VISITED)!=0; 
}

inline void set_solve_link(w16 x, w16 y, w16 from_x, w16 from_y) 
{
  g1_map_cell_class *c = g1_get_map()->cell(x,y);

  c->scratch_x = from_x;
  c->scratch_y = from_y;
}

inline void solve_link(w16 x, w16 y, w16 &from_x, w16 &from_y) 
{
  g1_map_cell_class *c = g1_get_map()->cell(x,y);

  from_x = c->scratch_x;
  from_y = c->scratch_y;
}

typedef g1_astar_map_solver_class::cell cell_type;
int cell_compare(const i4_float *a, const cell_type *b)
{
  i4_float ca = *a, cb = b->hint + b->length;
  if (cb<ca) 
    return -1;
  else if (cb>ca)
    return 1;
  else
    return 0;
}

i4_bool g1_astar_map_solver_class::add_link(w16 from_x,w16 from_y, w16 x,w16 y,
                                            i4_float from_length)
//{{{
{
  set_solve_link(x,y, from_x,from_y);
  visit(x,y);
  
  i4_float 
    dx = dest_x - x,
    dy = dest_y - y,
    hint = dx*dx+dy*dy,
    cost = hint + from_length;
  w32 loc;
  cell *p;

  if (heap.size()>0)
  {
    w32 hs = heap.size();
    i4_bsearch(&cost, loc, &heap[0], hs, cell_compare);

    p = heap.add_at(loc);
  }
  else
    p = heap.add();

  p->x = x;
  p->y = y;
  p->length = from_length;
  p->hint = hint;
    
  return i4_T;
}
//}}}

i4_bool g1_astar_map_solver_class::add_step_link(w16 from_x,w16 from_y, w16 x,w16 y,
                                                 i4_float from_length)
//{{{
{
  g1_map_class *map = g1_get_map();

  if (x<=1 || x>=map->width()-1 || y<=1 || y>=map->height())
    return i4_F;

  if (is_visited(x,y))
    return i4_F;

  if (g1_get_map()->cell(x,y)->get_solid_list())
    return i4_F;

  return add_link(from_x,from_y, x,y, from_length);
}
//}}}

i4_bool g1_astar_map_solver_class::add_path_link(w16 from_x,w16 from_y, w16 x,w16 y,
                                                 i4_float from_length)
//{{{
{
  g1_map_class *map = g1_get_map();

  if (x<=1 || x>=map->width()-1 || y<=1 || y>=map->height())
    return i4_F;

  if (is_visited(x,y))
    return i4_F;

  return add_link(from_x,from_y, x,y, from_length);
}
//}}}

i4_bool g1_astar_map_solver_class::get_next_cell(w16 &x,w16 &y, i4_float &length)
//{{{
{
  if (heap.size()==0)
    return i4_F;

  cell *p = &heap[heap.size()-1];
  x = p->x;
  y = p->y;
  length = p->length;

  heap.remove(heap.size()-1);

  return i4_T;
}
//}}}

void g1_astar_map_solver_class::clear_solve()
//{{{
{
  int x=0,y=0;
  g1_map_class *map=g1_get_map();
  g1_map_cell_class *c;
  
  for (y=0; y<map->height(); y++)
  {
    c = map->cell(0,y);
    for (x=0; x<map->width(); x++)
    {
      c->flags &= ~VISITED;
      c++;
    }
  }
}
//}}}

i4_bool g1_astar_map_solver_class::path_solve(i4_float startx, i4_float starty, 
                                              i4_float destx, i4_float desty, 
                                              i4_float *point, w16 &points)
//{{{
{
  pf_solve_astar.start();

  w16 x = (w16)startx,y = (w16)starty;
  i4_float l;

  dest_x = (w16)destx;
  dest_y = (w16)desty;

  clear_heap();
  clear_solve();

  add_link(x, y, x, y, 0);

  i4_bool found=i4_F;

  while (!found && get_next_cell(x,y,l))
  {
    if (x==dest_x && y==dest_y)
    {
      found = i4_T;
      break;
    }

    add_step_link(x,y, x,y+1, l+1);
    add_step_link(x,y, x,y-1, l+1);
    add_step_link(x,y, x-1,y, l+1);
    add_step_link(x,y, x+1,y, l+1);

    add_step_link(x,y, x+1,y+1, l+sqrt2);
    add_step_link(x,y, x-1,y+1, l+sqrt2);
    add_step_link(x,y, x+1,y-1, l+sqrt2);
    add_step_link(x,y, x-1,y-1, l+sqrt2);

    g1_object_chain_class *c = g1_get_map()->cell(x,y)->get_obj_list();
  }

  if (!found)
  {
    pf_solve_astar.stop();
    return i4_F;
  }

  w16 nx=dest_x,ny=dest_y;
  sw16 dx=0,dy=0;
  x = nx+1;

  points = 0;
  while (x!=nx || y!=ny)
  {
    ok(nx,ny);
    if (nx-x==dx && ny-y==dy)
      points--;
    dx = nx-x;
    dy = ny-y;
    x = nx; y = ny;
    point[points*2]=   i4_float(x*2+1)/2.0;
    point[points*2+1]= i4_float(y*2+1)/2.0;
    points++;
    solve_link(x,y, nx,ny);
  }

  pf_solve_astar.stop();
  return i4_T;
}
//}}}

//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}
