/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#error

#ifndef CRITICAL_GRAPH_HH
#define CRITICAL_GRAPH_HH

#include "g1_limits.hh"
#include "math/num_type.hh"
#include "saver.hh"

typedef w8 g1_graph_node;

class g1_critical_graph_class
{
  friend class g1_critical_map_maker_class;
public:
  enum { MAX_CRITICALS=256, MAX_CONNECTIONS=100 };

  class connection_class
  {
  public:
    g1_graph_node ref;
    i4_float dist;
    w8 size[G1_GRADE_LEVELS];
  };

  class critical_point_class
  {
  public:
    i4_float x,y; 
    connection_class *connection;
    w8 connections;
    w8 selected;
  };

protected:
  connection_class *pool;
  w32 pool_connections;
public:
  critical_point_class critical[MAX_CRITICALS];
  // note critical point 0 is used to mark the null 

  w16 criticals;

  g1_critical_graph_class() : criticals(1), pool(0) {}
  g1_critical_graph_class(g1_loader_class *f) : pool(0) { load_points(f); }
  ~g1_critical_graph_class();

  void clear_critical_graph();
  void compact_critical_graph();
  void expand_critical_graph();

  i4_bool add_critical_point(float x, float y);

  void save_points(g1_saver_class *f);
  void save_graph(g1_saver_class *f);
  void load_points(g1_loader_class *f);
  void load_graph(g1_loader_class *f);
};

#endif

//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}

