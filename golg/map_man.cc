/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "map_man.hh"
#include "map.hh"
#include "g1_object.hh"

g1_map_cell_class *g1_cells=0;
g1_map_vertex_class *g1_verts=0;
int g1_map_width, g1_map_height, g1_map_width_plus_one;


g1_map_class *g1_current_map_PRIVATE=0;

void g1_destroy_map()
{
  if (g1_current_map_PRIVATE)
  {
    delete g1_current_map_PRIVATE;
    g1_current_map_PRIVATE=0;

    for  (int i=0; i<=g1_last_object_type; i++)
      if (g1_object_type_array[i] && 
          (g1_object_type_array[i]->flags & g1_object_definition_class::DELETE_WITH_LEVEL))
      {
        g1_object_definition_class *od = g1_object_type_array[i];
        g1_remove_object_type(i);
        delete od;
      }
  
  }
}


void g1_set_map(g1_map_class *map)
{
  if (map)
  {
    g1_cells=map->cell(0,0);
    g1_verts=map->vertex(0,0);    
    g1_map_width=map->width();
    g1_map_width_plus_one=g1_map_width+1;
    g1_map_height=map->height();
    g1_current_map_PRIVATE=map;
  }
  else
  {
    g1_cells=0;
    g1_verts=0;
    g1_map_width=0;
    g1_map_height=0;
    g1_current_map_PRIVATE=0;
  }
}
