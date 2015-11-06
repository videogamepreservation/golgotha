/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "map_man.hh"
#include "map.hh"
#include "lisp/li_init.hh"
#include "lisp/lisp.hh"
#include "objs/map_piece.hh"


li_object *g1_drop_selected(li_object *o, li_environment *env)
{
  if (!g1_map_is_loaded())
    return 0;

  li_call("add_undo", li_make_list(new li_int(G1_MAP_OBJECTS)));
    
  g1_object_class *olist[G1_MAX_OBJECTS];
  sw32 t=g1_get_map()->make_object_list(olist, G1_MAX_OBJECTS);

  for (int i=0; i<t; i++)
  {
    if (olist[i]->selected())
    {
      olist[i]->unoccupy_location();
      olist[i]->h=olist[i]->lh=g1_get_map()->terrain_height(olist[i]->x, olist[i]->y);
      olist[i]->occupy_location();
    }
  }
  li_call("redraw");
  
  return 0;
}



li_object *g1_floor_selected(li_object *o, li_environment *env)
{
  li_call("add_undo", li_make_list(new li_int(G1_MAP_OBJECTS)));
    
  g1_object_class *olist[G1_MAX_OBJECTS];
  int t=g1_get_map()->make_object_list(olist, G1_MAX_OBJECTS),i;

  float lowest=1000;
  for (i=0; i<t; i++)
    if (olist[i]->selected())
      if (olist[i]->h<lowest)
        lowest=olist[i]->h;

  for (i=0; i<t; i++)
    if (olist[i]->selected())
    {
      olist[i]->h=lowest;
      olist[i]->grab_old();
    }

  li_call("redraw");
  
  return 0;
}



li_object *g1_ceil_selected(li_object *o, li_environment *env)
{
  li_call("add_undo", li_make_list(new li_int(G1_MAP_OBJECTS)));
    
  g1_object_class *olist[G1_MAX_OBJECTS];
  int t=g1_get_map()->make_object_list(olist, G1_MAX_OBJECTS),i;

  float heighest=0;
  for (i=0; i<t; i++)
    if (olist[i]->selected())
      if (olist[i]->h>heighest)
        heighest=olist[i]->h;

  for (i=0; i<t; i++)
    if (olist[i]->selected())
    {
      olist[i]->h=heighest;
      olist[i]->grab_old();
    }

  li_call("redraw");
  
  return 0;
}

li_object *g1_select_game_pieces(li_object *o, li_environment *env)
{
  li_call("add_undo", li_make_list(new li_int(G1_MAP_OBJECTS)));
    
  g1_object_class *olist[G1_MAX_OBJECTS];
  int t=g1_get_map()->make_object_list(olist, G1_MAX_OBJECTS),i;
  for (i=0; i<t; i++)    
    if (g1_map_piece_class::cast(olist[i]))
      olist[i]->set_flag(g1_object_class::SELECTED,1);
    else
      olist[i]->set_flag(g1_object_class::SELECTED,0);

  li_call("redraw");
  
  return 0;
}


li_object *g1_select_similar(li_object *o, li_environment *env)
{
  li_call("add_undo", li_make_list(new li_int(G1_MAP_OBJECTS)));
    
  g1_object_class *olist[G1_MAX_OBJECTS];
  int t=g1_get_map()->make_object_list(olist, G1_MAX_OBJECTS),i;

  w8 *sel_types=(w8 *)i4_malloc(g1_last_object_type,"");
  memset(sel_types, 0, g1_last_object_type);
  for (i=0; i<t; i++)
    if (olist[i]->get_flag(g1_object_class::SELECTED))
      sel_types[olist[i]->id]=1;

  for (i=0; i<t; i++)
    if (sel_types[olist[i]->id])
      olist[i]->set_flag(g1_object_class::SELECTED,1);

  i4_free(sel_types);
  
  li_call("redraw");
  
  return 0;
}


li_automatic_add_function(g1_drop_selected, "Objects/Drop Selected");
li_automatic_add_function(g1_select_similar, "Objects/Select Similar");
li_automatic_add_function(g1_select_game_pieces, "Objects/Select Game Pieces");
li_automatic_add_function(g1_floor_selected, "Map/Floor Selected");
li_automatic_add_function(g1_ceil_selected, "Map/Ceil Selected");
