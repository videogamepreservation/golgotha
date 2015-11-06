/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#error

#ifndef G1_SOLVEGRAPH_HH
#define G1_SOLVEGRAPH_HH

#include "critical_graph.hh"
#include "math/num_type.hh"

class g1_graph_solver_class
{
public:
  virtual void set_graph(g1_critical_graph_class *_graph) = 0;
  virtual i4_bool path_solve(g1_graph_node start_node, g1_graph_node end_node, 
                             w8 group_size, w8 grade,
                             i4_float *point, w16 &points) = 0;
};

#endif
