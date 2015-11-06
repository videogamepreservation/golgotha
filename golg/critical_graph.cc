/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "critical_graph.hh"
#include "path_api.hh"
#include "map.hh"
#include "map_cell.hh"
#include "map_man.hh"
#include <math.h>


g1_critical_graph_class::~g1_critical_graph_class()
//{{{
{
  if (pool)
    i4_free(pool);
  pool=0;
}
//}}}

void g1_critical_graph_class::clear_critical_graph()
//{{{
{
  if (pool)
    i4_free(pool);
  pool=0;

  pool_connections = 0;

  w32 size = sizeof(connection_class)*MAX_CRITICALS*MAX_CONNECTIONS;
  pool = (connection_class*)i4_malloc(size, "critical_connections");
  memset(pool,0,size);

  connection_class *current = pool;

  for (w32 j=1; j<MAX_CRITICALS; j++)
  {
    critical[j].connections=0;
    critical[j].connection = current;
    current += MAX_CONNECTIONS;
    pool_connections += MAX_CONNECTIONS;
  }
}
//}}}

void g1_critical_graph_class::compact_critical_graph()
//{{{
{
  connection_class *old_pool = pool;

  w32 count=0, i,j;
  for (j=1; j<criticals; j++)
    count += critical[j].connections;

  w32 size = sizeof(connection_class)*count;
  pool = (connection_class*)i4_malloc(size, "critical_connections");

  connection_class *current = pool;
  pool_connections = 0;

  for (j=1; j<criticals; j++)
  {
    for (i=0; i<critical[j].connections; i++)
      current[i] = critical[j].connection[i];
    critical[j].connection = current;
    current += critical[j].connections;
    pool_connections += critical[j].connections;
  }

  if (old_pool)
    i4_free(old_pool);
}
//}}}

void g1_critical_graph_class::expand_critical_graph()
//{{{
{
  connection_class *old_pool = pool;

  w32 size = sizeof(connection_class)*MAX_CRITICALS*MAX_CONNECTIONS;
  pool = (connection_class*)i4_malloc(size, "critical_connections");
  memset(pool,0,size);

  connection_class *current = pool;
  pool_connections = 0;

  for (w32 j=1; j<MAX_CRITICALS; j++)
  {
    if (j<criticals)
      for (w32 i=0; i<critical[j].connections; i++)
        current[i] = critical[j].connection[i];
    critical[j].connection = current;
    current += MAX_CONNECTIONS;
    pool_connections += MAX_CONNECTIONS;
  }

  if (old_pool)
    i4_free(old_pool);
}
//}}}

i4_bool g1_critical_graph_class::add_critical_point(i4_float x, i4_float y)
//{{{
{
  if (criticals<MAX_CRITICALS)
  {
    critical[criticals].x=x;    // oliy change this to float - jc
    critical[criticals].y=y;
    critical[criticals].connections=0;
    critical[criticals].connection=0;
    critical[criticals].selected=0;
    criticals++;

    return i4_T;
  }
  else
    return i4_F;
}
//}}}

void g1_critical_graph_class::save_points(g1_saver_class *f)
//{{{
{
  *f << criticals;
  for (w32 j=1; j<criticals; j++)
  {
    f->write_16(sw16(critical[j].x));
    f->write_16(sw16(critical[j].y));
  }
}
//}}}


void g1_critical_graph_class::save_graph(g1_saver_class *f)
//{{{
{
  w32 count=0, i,j;
  for (j=1; j<criticals; j++)
    count += critical[j].connections;

  *f << (w32)count;
  for (j=1; j<criticals; j++)
  {
    connection_class *conn = &critical[j].connection[0];

    *f << critical[j].connections;
    for (i=0; i<critical[j].connections; i++, conn++)
    {
      *f << conn->ref << conn->dist;
      for (int g=0; g<G1_GRADE_LEVELS; g++)
        *f << conn->size[g];
    }
  }
}
//}}}


void g1_critical_graph_class::load_points(g1_loader_class *f)
//{{{
{
  *f >> criticals;

  for (w32 j=1; j<criticals; j++)
  {
    critical[j].x = i4_float(f->read_16()) + 0.5;
    critical[j].y = i4_float(f->read_16()) + 0.5;
    critical[j].connection=0;
    critical[j].connections=0;
    critical[j].selected=0;
  }

  if (pool)
    i4_free(pool);
  pool = 0;
  pool_connections=0;
}
//}}}


void g1_critical_graph_class::load_graph(g1_loader_class *f)
//{{{
{
  if (pool)
    i4_free(pool);

  pool = 0;

  *f >> pool_connections;

  w32 size = sizeof(connection_class)*pool_connections;

  if (size)
    pool = (connection_class*)i4_malloc(size, "critical_connections");
  else pool=0;

  connection_class *conn = pool;

  for (w32 j=1; j<criticals; j++)
  {
    critical[j].connection = conn;
    *f >> critical[j].connections;
    for (w32 i=0; i<critical[j].connections; i++, conn++)
    {
      *f >> conn->ref >> conn->dist;
      for (int g=0; g<G1_GRADE_LEVELS; g++)
        *f >> conn->size[g];
    }
  }
}
//}}}


//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}
