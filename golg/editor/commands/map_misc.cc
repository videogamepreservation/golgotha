/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "map_cell.hh"
#include "map_vert.hh"
#include "map_man.hh"
#include "lisp/lisp.hh"
#include "lisp/li_init.hh"
#include "map.hh"
#include "objs/path_object.hh"


li_object *g1_fog_map(li_object *, li_environment *env)
{  
  li_call("add_undo", li_make_list(new li_int(G1_MAP_CELLS | G1_MAP_VERTS)));
  g1_map_cell_class *c=g1_cells;
  int t=g1_map_width * g1_map_height, i;

  for (i=0; i<t; i++, c++)
    c->flags |= g1_map_cell_class::FOGGED;
  

  g1_map_vertex_class *v=g1_verts;
  t=(g1_map_width+1) * (g1_map_height+1);

  for (i=0; i<t; i++, v++)
    v->flags |= g1_map_vertex_class::FOGGED;


  li_call("redraw");
  return 0;
}


li_object *g1_unfog_map(li_object *, li_environment *env)
{  
  li_call("add_undo", li_make_list(new li_int(G1_MAP_CELLS | G1_MAP_VERTS)));
  g1_map_cell_class *c=g1_cells;
  int t=g1_map_width * g1_map_height, i;

  for (i=0; i<t; i++, c++)
    c->flags &= ~g1_map_cell_class::FOGGED;
  

  g1_map_vertex_class *v=g1_verts;
  t=(g1_map_width+1) * (g1_map_height+1);

  for (i=0; i<t; i++, v++)
    v->flags &= ~g1_map_vertex_class::FOGGED;


  li_call("redraw");
  return 0;
}

static void select_rest(g1_path_object_class *p, g1_team_type team)
{
  li_call("add_undo", li_make_list(new li_int(G1_MAP_OBJECTS)));

  if (p && !p->get_flag(g1_object_class::SCRATCH_BIT))
  {
    p->set_flag(g1_object_class::SCRATCH_BIT | g1_object_class::SELECTED, 1);
    
    int t=p->total_links(team);
    for (int j=0; j<t; j++)
      select_rest(p->get_link(team, j), team);

    p->set_flag(g1_object_class::SCRATCH_BIT, 0);
  }
}

li_object *g1_select_restof_path(li_object *o, li_environment *env)
{
  li_call("add_undo", li_make_list(new li_int(G1_MAP_OBJECTS)));

  for (g1_path_object_class *p=g1_path_object_list.first(); p; p=p->next)
  {
    if (p->get_flag(g1_object_class::SELECTED))
    {
      p->set_flag(g1_object_class::SCRATCH_BIT, 1);

      for (g1_team_type team=G1_ALLY; team<G1_MAX_TEAMS; team=(g1_team_type)(team+1))
      {
        int t=p->total_links(team);
        for (int j=0; j<t; j++)
          select_rest(p->get_link(team, j), team);
      }

      p->set_flag(g1_object_class::SCRATCH_BIT, 0);
    }
  }

  return 0;
}

li_object *g1_add_cloud_shadow(li_object *o, li_environment *env)
{
  li_call("add_undo", li_make_list(new li_int(G1_MAP_OBJECTS)));

  g1_object_class *c=g1_create_object(g1_get_object_type("cloud_shadow"));
  c->x=g1_map_width/2;
  c->y=g1_map_height/2;
  c->h=g1_get_map()->terrain_height(c->x, c->y)+0.1;
  c->occupy_location();

  li_call("redraw");
  return 0;
}

li_object *g1_remove_cloud_shadow(li_object *o, li_environment *env)
{
  li_call("add_undo", li_make_list(new li_int(G1_MAP_OBJECTS)));
  
  int type=g1_get_object_type("cloud_shadow");
  g1_object_class *olist[G1_MAX_OBJECTS];
  sw32 t=g1_get_map()->make_object_list(olist, G1_MAX_OBJECTS);
  for (int i=0; i<t; i++)
  {
    if (olist[i]->id==type)
    {
      olist[i]->unoccupy_location();
      olist[i]->request_remove();     
    }
  }

  li_call("redraw");
  return 0;
}



li_automatic_add_function(g1_fog_map, "fog_map");
li_automatic_add_function(g1_unfog_map, "unfog_map");
li_automatic_add_function(g1_select_restof_path, "select_restof_path");

li_automatic_add_function(g1_add_cloud_shadow, "add_cloud_shadow");
li_automatic_add_function(g1_remove_cloud_shadow, "remove_cloud_shadow");
