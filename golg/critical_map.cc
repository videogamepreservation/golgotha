/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "critical_map.hh"
#include "path_api.hh"
#include "map.hh"
#include "map_cell.hh"
#include "status/status.hh"
#include <math.h>


void g1_critical_map_maker_class::clear_critical_map()
//{{{
{
  int i,j,k;
  g1_map_cell_class *c=map->cell(0,0);

  for (j=0; j<map->height(); j++)
    for (i=0; i<map->width(); i++, c++)
      memset(c->nearest_critical, 0, sizeof(c->nearest_critical));
}
//}}}


g1_graph_node g1_critical_map_maker_class::get_critical(w16 x, w16 y, int n)
//{{{
{
  return map->cell(x,y)->nearest_critical[grade][tofrom][n];
}
//}}}


i4_bool g1_critical_map_maker_class::critical_full(w16 x, w16 y, g1_graph_node crit)
//{{{
{
  w8 *p = map->cell(x,y)->nearest_critical[grade][tofrom];
  if (p[G1_CRITICALS_PER_CELL-1]!=g1_map_cell_class::NO_CRITICAL)
    return i4_T;
  for (int i=0; i<G1_CRITICALS_PER_CELL; i++)
    if (p[i]==crit)
      return i4_T;
  return i4_F;
}
//}}}


void g1_critical_map_maker_class::set_critical(w16 x, w16 y, g1_graph_node critical)
//{{{
{
  I4_ASSERT(critical!=g1_map_cell_class::NO_CRITICAL, "can't set critical point to blank");
  I4_ASSERT(!critical_full(x,y,critical), "Adding too many critical points to one spot");

  w8 *p = map->cell(x,y)->nearest_critical[grade][tofrom];
  while (*p!=g1_map_cell_class::NO_CRITICAL)
    p++;
  *p = critical;
}
//}}}


i4_bool g1_critical_map_maker_class::add_critical(w32 x,w32 y,w8 d, g1_graph_node critical)
//{{{
{
  if (d>0 &&
      (x<0 || x>=map->width() || y<0 || y>=map->height() || critical_full(x,y,critical)))
    return i4_F;
    
  set_critical(x,y,critical);
    
  cnx[head] = x;
  cny[head] = y;
  crit[head] = critical;
  if (++head>=queue_length) head=0;

  if (head==tail)
    i4_warning("queue overrun.");
    
  return i4_T;
}
//}}}


i4_bool g1_critical_map_maker_class::get_next_critical(w32 &x,w32 &y, g1_graph_node &critical)
//{{{
{
  if (tail==head)
    return i4_F;

  x = cnx[tail];
  y = cny[tail];
  critical = crit[tail];
  if (++tail>=queue_length) tail=0;

  return i4_T;
}
//}}}


i4_bool g1_critical_map_maker_class::make_critical_graph()
//{{{
{
  w32 i,j,g;
  g1_critical_graph_class::connection_class *c;
  int add;

  i4_status_class *status=i4_create_status(i4gets("make_crit_graph"),1);
    
  critical->clear_critical_graph();

  int canceled=0;
  float update_time=0;

  for (j=1; j<critical->criticals && !canceled; j++)
  {
    if (status)
    {
      update_time=(j+1)/(float)critical->criticals;
      if (!status->update(update_time))
        canceled=1;
    }
          

    g1_critical_graph_class::critical_point_class *crit = &critical->critical[j];
    for (i=1; i<critical->criticals && !canceled; i++) 
    {
      if (i==j)
        continue;

      w8 size[G1_GRADE_LEVELS];

      if (!status->update(update_time))
        canceled=1;

      g1_critical_graph_class::critical_point_class *crit2 = &critical->critical[i];
      for (g=0; g<G1_GRADE_LEVELS; g++) 
        size[g] = (w8)(64.0*map->block[g].line_of_sight(crit->x, crit->y, crit2->x, crit2->y));

      add = 0;
      for (g=0; g<G1_GRADE_LEVELS-1; g++) 
        if (size[g])
          add=1;

      if (add)
      {
        I4_ASSERT(crit->connections<g1_critical_graph_class::MAX_CONNECTIONS,
                  "Adding too many connections");
        
        // goto next available connection entry
        c = &crit->connection[crit->connections];
        crit->connections++;
        
        // calculate info
        i4_float 
          dx = crit2->x - crit->x,
          dy = crit2->y - crit->y;
        
        c->ref  = i;
        c->dist = sqrt(dx*dx+dy*dy);

        for (g=0; g<G1_GRADE_LEVELS; g++) 
          c->size[g] = size[g];
      }
    }
  }

  if (status)
    delete status;

  return !canceled;
}
//}}}
  

i4_bool g1_critical_map_maker_class::make_critical_map()
//{{{
{
  w32 x,y;
  g1_graph_node c;
 
  i4_status_class *stat = i4_create_status(i4gets("make_critical_map"), 1);
  int canceled=0;

  clear_critical_map();
  tofrom=0;
  float stime=0;

  for (grade=0; grade<G1_GRADE_LEVELS && !canceled; grade++)
  {
    stime=(float)grade/(G1_GRADE_LEVELS*2);

    clear_queue();
    
    for (c=1; c<critical->criticals; c++)
    {
      if (stat) 
        if (!stat->update(stime))
          canceled=1;

      add_critical((w32)critical->critical[c].x,
                   (w32)critical->critical[c].y,
                   0,c);
    }
    
    while (get_next_critical(x,y,c))
    {
      if (!map->block[grade].is_blocked(x,y,G1_SOUTH))
        add_critical(x,y-1,G1_NORTH,c);
      if (!map->block[grade].is_blocked(x,y,G1_NORTH))
        add_critical(x,y+1,G1_SOUTH,c);
      if (!map->block[grade].is_blocked(x,y,G1_EAST))
        add_critical(x+1,y,G1_WEST,c);
      if (!map->block[grade].is_blocked(x,y,G1_WEST))
        add_critical(x-1,y,G1_EAST,c);
    }
  }

  w32 wx = map->width(), wy=map->height();
  tofrom=1;
  for (grade=0; grade<G1_GRADE_LEVELS && !canceled; grade++)
  {
    stime=(float)(grade+G1_GRADE_LEVELS)/(G1_GRADE_LEVELS*2);

    clear_queue();
    
    for (c=1; c<critical->criticals; c++)
    {
      if (stat && !stat->update(stime))
        canceled=1;

      add_critical((w32)critical->critical[c].x,
                   (w32)critical->critical[c].y,
                   0,c);
    }
    
    while (get_next_critical(x,y,c))
    {
      if (stat && !stat->update(stime))
        canceled=1;

      if (y<wy-1 && !map->block[grade].is_blocked(x,y-1,G1_NORTH))
        add_critical(x,y-1,G1_NORTH,c);
      if (y>0 && !map->block[grade].is_blocked(x,y+1,G1_SOUTH))
        add_critical(x,y+1,G1_SOUTH,c);
      if (x<wx-1 && !map->block[grade].is_blocked(x+1,y,G1_WEST))
        add_critical(x+1,y,G1_WEST,c);
      if (x>0 && !map->block[grade].is_blocked(x-1,y,G1_EAST))
        add_critical(x-1,y,G1_EAST,c);
    }
  }

  if (stat) 
    delete stat;
  
  return !canceled;
}
//}}}


i4_bool g1_critical_map_maker_class::make_criticals(g1_map_class *_map, 
                                                 g1_critical_graph_class *graph)
//{{{
{
  map = _map;
  critical = graph;

  if (make_critical_map() && make_critical_graph())
    return i4_T;
  else return i4_F;
    
}
//}}}


//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}
