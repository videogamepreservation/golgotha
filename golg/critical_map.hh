/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef CRITICAL_MAP_HH
#define CRITICAL_MAP_HH

#include "arch.hh"
#include "critical_graph.hh"

class g1_map_class;

class g1_critical_map_maker_class
{
protected:
  g1_map_class *map;
  g1_critical_graph_class *critical;

  enum { queue_length=22500 };
  w32 cnx[queue_length], cny[queue_length];
  g1_graph_node crit[queue_length];
  w32 head,tail;

  void clear_queue() { head = tail = 0; }

  // critical map accessors
  i4_bool critical_full(w16 x, w16 y, g1_graph_node crit);
  void set_critical(w16 x, w16 y, g1_graph_node critical);

  i4_bool add_critical(w32 x,w32 y,w8 d, g1_graph_node critical);
  i4_bool get_next_critical(w32 &x,w32 &y, g1_graph_node &critical);

  i4_bool make_critical_map();
  i4_bool make_critical_graph();

  void clear_critical_map();
public:
  w8 grade, tofrom;
  g1_graph_node get_critical(w16 x, w16 y, int n=0);

  // returns false if canceled by user
  i4_bool make_criticals(g1_map_class *_map, g1_critical_graph_class *_graph);
  g1_critical_map_maker_class() 
    : map(0), critical(0) {}
};

#endif

//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}

