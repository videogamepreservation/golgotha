/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "lisp/lisp.hh"
#include "map.hh"
#include "lisp/li_init.hh"
#include "map_man.hh"
#include "math/pi.hh"
#include "math/angle.hh"
#include "map_cell.hh"
#include "map_vert.hh"

int g1_rotate_remap[4]={0, 1, 2, 3};

li_object *g1_rotate_map_90(li_object *o, li_environment *env)
{
  if (!g1_map_is_loaded()) return 0;  
  g1_map_class *map=g1_get_map();

  
  int ow=map->width(), oh=map->height(), nw=map->height(), nh=map->width(), size;

  size=sizeof(g1_map_cell_class)*nw*nh;
  if (size>i4_largest_free_block())
  {
    i4_warning("not enough memory");
    return 0;
  }
  
  g1_map_cell_class *ncells=(g1_map_cell_class *)i4_malloc(size,"");


  size=sizeof(g1_map_vertex_class)*(nw+1)*(nh+1);
  if (size>i4_largest_free_block())
  {
    i4_free(ncells);
    i4_warning("not enough memory");
    return 0;
  }     
  g1_map_vertex_class *nverts=(g1_map_vertex_class *)i4_malloc(size,"");

  li_call("add_undo", li_make_list(new li_int(G1_MAP_ALL)));
 
  g1_object_class *olist[G1_MAX_OBJECTS];
  int t=map->make_object_list(olist, G1_MAX_OBJECTS), i,x,y;

  for (i=0; i<t; i++)
    olist[i]->unoccupy_location();

  

  // move the cells
  g1_map_cell_class *ocells=map->cell(0,0);
  for (y=0; y<oh; y++)
    for (x=0; x<ow; x++, ocells++)
    {
      int ns=y+x*nw;
      ncells[ns]=*ocells;

      ncells[ns].set_rotation((g1_rotation_type)((g1_rotate_remap[ncells[ns].get_rotation()])));
    }

  // move the vertexes
  g1_map_vertex_class *overts=map->vertex(0,0);
  for (y=0; y<oh+1; y++)
    for (x=0; x<ow+1; x++, overts++)
    {
      int ns=y+x*(nw+1);
      nverts[ns]=*overts;      
    }


  map->change_map(nw, nh, ncells, nverts);

  // move the objects and rotate them
  for (i=0; i<t; i++)
  {
    float x=olist[i]->x;
    olist[i]->x=olist[i]->y;
    olist[i]->y=x;

    olist[i]->theta += i4_pi()/3.0/2.0;
    i4_normalize_angle(olist[i]->theta);
    
    olist[i]->grab_old();
  }
  

  for (i=0; i<t; i++)
    olist[i]->occupy_location();

  
  li_call("editor_refresh");
  li_call("editor_changed");
  return 0;
}


li_automatic_add_function(g1_rotate_map_90, "Map/Rotate 90");
