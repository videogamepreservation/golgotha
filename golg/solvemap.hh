/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_SOLVEMAP_HH
#define G1_SOLVEMAP_HH

#include "arch.hh"
#include "math/num_type.hh"

class g1_block_map_class;
class g1_map_solver_class
{
public:
  g1_map_solver_class() {}

  virtual void set_block_map(g1_block_map_class *_block) = 0;
  virtual i4_bool path_solve(w32 startx, w32 starty, w32 destx, w32 desty, 
                             w8 sizex, w8 sizey, w8 grade,
                             i4_float *point, w16 &points) = 0;
};

#endif

//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}
