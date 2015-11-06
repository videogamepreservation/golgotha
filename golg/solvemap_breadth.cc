/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "solvemap_breadth.hh"
#include "path_api.hh"
#include "memory/malloc.hh"
#include "error/error.hh"

void g1_breadth_first_map_solver_class::set_block_map(g1_block_map_class *_block)
//{{{
{
  block = _block;
  if (!solve_map || block->width()!=wx || block->height()!=wy)
  {
    if (solve_map)
      i4_free(solve_map);
    wx = block->width();
    wy = block->height();
    solve_map = (w8*)i4_malloc(wx*wy,"solve_map");
  }
}
//}}}

g1_breadth_first_map_solver_class::~g1_breadth_first_map_solver_class()
//{{{
{
  if (solve_map)
    i4_free(solve_map);
}
//}}}

i4_bool g1_breadth_first_map_solver_class::add_cell(w32 x,w32 y,w8 d, w32 length)
//{{{
{
  if (d>0 &&
      (x<0 || x>=wx || y<0 || y>=wy || is_visited(x,y) || block->is_blocked(x,y,d)))
    return i4_F;

  set_solve_dir(x,y,d);
  visit(x,y);
    
  cnx[head] = x;
  cny[head] = y;
  cnl[head] = length;
  if (++head>=queue_length) head=0;

  if (head==tail)
    i4_warning("queue overrun.");
    
  return i4_T;
}
//}}}

i4_bool g1_breadth_first_map_solver_class::get_next_cell(w32 &x,w32 &y, w32 &length)
//{{{
{
  if (tail==head)
    return i4_F;

  x = cnx[tail];
  y = cny[tail];
  length = cnl[tail];
  if (++tail>=queue_length) tail=0;

  return i4_T;
}
//}}}

i4_bool g1_breadth_first_map_solver_class::path_solve(w32 startx, w32 starty, 
                                                      w32 destx, w32 desty, 
                                                      w8 sizex, w8 sizey, w8 grade,
                                                      i4_float *point, w16 &points)
//{{{
{
  w32 x,y,l;

  clear_queue();
  clear_solve();

  add_cell(startx,
           starty,
           0,0);

  int found=0;

  while (get_next_cell(x,y,l) && (x!=destx || y!=desty))
  {
    add_cell(x,y-1,G1_NORTH,l+1);
    add_cell(x,y+1,G1_SOUTH,l+1);
    add_cell(x+1,y,G1_WEST,l+1);
    add_cell(x-1,y,G1_EAST,l+1);
  }

  x = destx;
  y = desty;

  w32 last_dir=0;
  points = 0;
  if (solve_dir(x,y)==0)
    return i4_F;
  while (x!=startx || y!=starty)
  {
    ok(x,y);
    if (solve_dir(x,y)!=last_dir)
    {
      last_dir = solve_dir(x,y);
      point[points*2]=   i4_float(x)+0.5;
      point[points*2+1]= i4_float(y)+0.5;
      points++;
    }
    switch (solve_dir(x,y)) 
    {
      case G1_NORTH: y++; break;
      case G1_SOUTH: y--; break;
      case G1_WEST:  x--; break;
      case G1_EAST:  x++; break;
      default:
        I4_ASSERT(0,"busted path solving!\n");
        break;
    }
  }
  ok(x,y);
  point[points*2]=  i4_float(x)+0.5;
  point[points*2+1]=i4_float(y)+0.5;
  points++;

  return i4_T;
}
//}}}

//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}
