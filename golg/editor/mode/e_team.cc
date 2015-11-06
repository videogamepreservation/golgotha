/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "editor/e_state.hh"
#include "lisp/li_init.hh"
#include "lisp/lisp.hh"
#include "map.hh"
#include "map_man.hh"
#include "g1_tint.hh"
#include "player.hh"
#include "lisp/li_dialog.hh"

static li_object *set_team(int num)
{
  li_call("add_undo", li_make_list(new li_int(G1_MAP_OBJECTS)));

  g1_object_class *olist[G1_MAX_OBJECTS];
  sw32 t=g1_get_map()->make_object_list(olist, G1_MAX_OBJECTS);

  for (int i=0; i<t; i++)
    if (olist[i]->selected())
      olist[i]->change_player_num(num);


  g1_edit_state.current_team=num;

  li_call("redraw");
    
  return 0;
}


static li_object *team_0(li_object *o, li_environment *env) { return set_team(0); }
static li_object *team_1(li_object *o, li_environment *env) { return set_team(1); }
static li_object *team_2(li_object *o, li_environment *env) { return set_team(2); }
static li_object *team_3(li_object *o, li_environment *env) { return set_team(3); }
static li_object *team_4(li_object *o, li_environment *env) { return set_team(4); }


static li_object *tint_none(li_object *o, li_environment *env)
{
  g1_tint=G1_TINT_OFF;
  li_call("redraw");
  return 0;
}

static li_object *tint_polys(li_object *o, li_environment *env)
{
  g1_tint=G1_TINT_POLYS;
  li_call("redraw");
  return 0;
}

static li_object *tint_all(li_object *o, li_environment *env)
{
  g1_tint=G1_TINT_ALL;
  li_call("redraw");
  return 0;
}


static int team_editing;
li_object *g1_set_player(li_object *o, li_environment *env)
{
  g1_player_man.get(team_editing)->vars=li_car(o, env);
  return 0;
}


static li_object *edit_team(int num)
{
  team_editing=num;
  li_create_dialog("Player Vars", g1_player_man.get(num)->vars.get(),
                   0, g1_set_player);
  return 0;
}

li_object *edit_team_0(li_object *o, li_environment *env) { return edit_team(0); }
li_object *edit_team_1(li_object *o, li_environment *env) { return edit_team(1); }
li_object *edit_team_2(li_object *o, li_environment *env) { return edit_team(2); }
li_object *edit_team_3(li_object *o, li_environment *env) { return edit_team(3); }
li_object *edit_team_4(li_object *o, li_environment *env) { return edit_team(4); }


li_automatic_add_function(team_0, "team_0");
li_automatic_add_function(team_1, "team_1");
li_automatic_add_function(team_2, "team_2");
li_automatic_add_function(team_3, "team_3");
li_automatic_add_function(team_4, "team_4");

li_automatic_add_function(edit_team_0, "edit_team_0");
li_automatic_add_function(edit_team_1, "edit_team_1");
li_automatic_add_function(edit_team_2, "edit_team_2");
li_automatic_add_function(edit_team_3, "edit_team_3");
li_automatic_add_function(edit_team_4, "edit_team_4");

li_automatic_add_function(tint_none, "tint_none");
li_automatic_add_function(tint_polys, "tint_polys");
li_automatic_add_function(tint_all, "tint_all");
