/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "solvegraph_breadth.hh"
#include "search.hh"

void g1_breadth_first_graph_solver_class::set_graph(g1_critical_graph_class *_graph)
{
  graph=_graph;
  if (!graph || !solve_graph || graph->criticals!=nodes)
  {
    if (solve_graph)
      i4_free(solve_graph);

    nodes = graph->criticals;
    solve_graph = (solve_node*)i4_malloc(nodes*sizeof(solve_node), "solve_graph");
  }
}

g1_breadth_first_graph_solver_class::~g1_breadth_first_graph_solver_class()
{
  if (solve_graph)
    i4_free(solve_graph);
}


int compare_nodes(const g1_breadth_first_graph_solver_class::solve_node *a, 
                  const g1_breadth_first_graph_solver_class::solve_node *b)
{
  // smallest length last
  if (b->length > a->length)
    return 1;
  else if (b->length < a->length)
    return -1;
  else
    return 0;
}

i4_bool g1_breadth_first_graph_solver_class::add_node(g1_graph_node node, g1_graph_node from, 
                                                      i4_float len)
{
  if (solve_graph[node].ref && solve_graph[node].length<len)
    // found better one already
    return i4_F;

  // store current path graph
  solve_graph[node].ref = from;
  solve_graph[node].length = len;

  // add into heap
  w32 loc;
  solve_node test(node,len);

  if (heap.size())
  {
    i4_bsearch(&test, loc, &heap[0], (w32)heap.size(), compare_nodes);
    heap.add_at(test, loc);
  }
  else
    heap.add(test);

  return i4_T;
}

i4_bool g1_breadth_first_graph_solver_class::get_next_node(g1_graph_node &node, i4_float &len)
{
  w32 loc;

  if ((loc=heap.size())==0)
    // nothing left in heap
    return i4_F;

  // get last node (shortest) & remove
  loc--;
  node = heap[loc].ref;
  len = heap[loc].length;
  heap.remove(loc);

  return i4_T;
}

i4_bool g1_breadth_first_graph_solver_class::path_solve(g1_graph_node start_node, 
                                                        g1_graph_node end_node, 
                                                        w8 group_size, w8 grade,
                                                        i4_float *point, w16 &points)
{
  g1_graph_node node;
  i4_float len;

  clear_heap();
  clear_solve();

  if (!start_node || !end_node)
    return i4_F;

  add_node(start_node, 0, 0);

  while (get_next_node(node, len))
  {
    g1_critical_graph_class::connection_class *c = graph->critical[node].connection;
    for (int i=0; i<graph->critical[node].connections; i++, c++)
      if (group_size<=c->size[grade])
        add_node(c->ref, node, len + c->dist);
  }

  points=0;
  
  if (solve_graph[end_node].ref==0)
    return i4_F;

  // count nodes
  node = end_node;
  points = 0;
  while (node!=start_node)
  {
    point[points*2+0] = i4_float(graph->critical[node].x)+0.5;
    point[points*2+1] = i4_float(graph->critical[node].y)+0.5;
    points++;
    node = solve_graph[node].ref;
  }
  point[points*2+0] = i4_float(graph->critical[start_node].x)+0.5;
  point[points*2+1] = i4_float(graph->critical[start_node].y)+0.5;
  points++;

  return i4_T;
}

