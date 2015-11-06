/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_SOLVEMAP_BREADTH_HH
#define G1_SOLVEMAP_BREADTH_HH

#include "arch.hh"
#include "solvemap.hh"
#include <string.h>

class g1_block_map_class;
class g1_breadth_first_map_solver_class : public g1_map_solver_class
{
protected:
  w16 wx, wy;

  w8 *solve_map;
  g1_block_map_class *block;

  // Breadth first Queue
  enum { queue_length=22500 };
  w32 cnx[queue_length], cny[queue_length], cnl[queue_length];
  w32 head,tail;

  void clear_queue() { head = tail = 0; }

  i4_bool add_cell(w32 x,w32 y,w8 d, w32 length);
  i4_bool get_next_cell(w32 &x,w32 &y, w32 &length);

  void clear_solve()
  //{{{
  {
    memset(solve_map, 0, wx*wy);
  }
  //}}}

public:
  enum { VISITED=16, OK=32 };

  w16 width() { return wx; }
  w16 height() { return wy; }

  g1_breadth_first_map_solver_class() : solve_map(0) {}
  g1_breadth_first_map_solver_class(g1_block_map_class *_block) : solve_map(0)
  { set_block_map(_block); }
  ~g1_breadth_first_map_solver_class();

  // Solve Map Accessors
  void visit(int x, int y) { solve_map[y*wx+x] |= VISITED; }
  void unvisit(int x, int y) { solve_map[y*wx+x] &= ~VISITED; }
  i4_bool is_visited(int x, int y) { return (solve_map[y*wx+x] & VISITED)!=0; }

  void ok(int x, int y) { solve_map[y*wx+x] |= OK; }
  void unok(int x, int y) { solve_map[y*wx+x] &= ~OK; }
  i4_bool is_ok(int x, int y) { return (solve_map[y*wx+x] & OK)!=0; }

  w8 solve_dir(int x, int y) { return solve_map[y*wx+x]&0xf; }
  void set_solve_dir(int x, int y, w8 dir) 
  //{{{
  {
    solve_map[y*wx+x] = (solve_map[y*wx+x]&0xf0) | (dir&0xf);
  }
  //}}}

  virtual void set_block_map(g1_block_map_class *_block);
  virtual i4_bool path_solve(w32 startx, w32 starty, w32 destx, w32 desty, w8 size, w8 grade,
                             i4_float *point, w16 &points);
};

#endif

//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}
