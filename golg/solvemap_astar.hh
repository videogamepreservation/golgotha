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
#include "memory/array.hh"
#include "math/num_type.hh"
#include "map_cell.hh"
#include <string.h>

class g1_block_map_class;
class g1_astar_map_solver_class
{
public:
  class cell
  {
  public:
    w16 x,y;
    i4_float length;
    i4_float hint;

    cell() {}
    cell(w16 x, w16 y, i4_float length, i4_float hint) : x(x), y(y), length(length), hint(hint) {}
    cell(const cell &a) : x(a.x), y(a.y), length(a.length), hint(a.hint) {}
  };
protected:
  w16 wx, wy;
  w16 dest_x, dest_y;

  // Breadth first Queue
  i4_array<cell> heap;

  void clear_heap() { heap.clear(); }

  i4_bool add_link(w16 from_x,w16 from_y, w16 x,w16 y, i4_float length);
  i4_bool add_step_link(w16 from_x,w16 from_y, w16 x,w16 y, i4_float length);
  i4_bool add_path_link(w16 from_x,w16 from_y, w16 x,w16 y, i4_float length);
  i4_bool get_next_cell(w16 &x,w16 &y,i4_float &length);

  void clear_solve();
public:
  enum { VISITED=g1_map_cell_class::SCRATCH1, OK=g1_map_cell_class::SCRATCH2 };

  g1_astar_map_solver_class() : heap(2048,1024) {}

  virtual i4_bool path_solve(i4_float startx, i4_float starty, i4_float destx, i4_float desty, 
                             i4_float *point, w16 &points);
};

#endif

//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}
