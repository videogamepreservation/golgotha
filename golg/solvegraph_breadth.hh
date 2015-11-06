/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_SOLVEGRAPH_BREADTH_HH
#define G1_SOLVEGRAPH_BREADTH_HH

#include "solvegraph.hh"

class g1_breadth_first_graph_solver_class : public g1_graph_solver_class
{
public:
  class solve_node
  {
  public:
    g1_graph_node ref;
    i4_float length;

    solve_node() {}
    solve_node(g1_graph_node node, i4_float length) : ref(node), length(length) {}
    solve_node& operator=(const solve_node &a) { ref=a.ref; length=a.length; return *this; }
  };

protected:
  g1_critical_graph_class *graph;


  solve_node *solve_graph;
  w32 nodes;

  // Breadth first heap
  i4_array<solve_node> heap;

  void clear_heap() { heap.clear(); }

  i4_bool add_node(g1_graph_node node, g1_graph_node from, i4_float len);
  i4_bool get_next_node(g1_graph_node &node, i4_float &len);

  void clear_solve()
  //{{{
  {
    memset(solve_graph, 0, nodes*sizeof(solve_node));
  }
  //}}}
public:

  g1_breadth_first_graph_solver_class() : heap(100,20), solve_graph(0) {}
  g1_breadth_first_graph_solver_class(g1_critical_graph_class *_graph) 
    : heap(100,20), solve_graph(0) { set_graph(_graph); }
  ~g1_breadth_first_graph_solver_class();

  virtual void set_graph(g1_critical_graph_class *_graph);
  virtual i4_bool path_solve(g1_graph_node start_node, g1_graph_node end_node, 
                             w8 group_size, w8 grade,
                             i4_float *point, w16 &points);
};

#endif

//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}
