/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "player.hh"
#include "objs/def_object.hh"
#include "lisp/li_class.hh"
#include "lisp/li_init.hh"
#include "controller.hh"


static li_float_class_member damage_multiplier_fraction("damage_multiplier_fraction"),
  spin_speed("spin_speed");



li_object *g1_armor_occupy(li_object *o, li_environment *env)
{
  g1_dynamic_object_class *me=g1_dynamic_object_class::get(li_first(o));

  if (me->g1_object_class::occupy_location())
  {
    g1_player_man.get(me->player_num)->damage_multiplier() *= damage_multiplier_fraction();
    return li_true_sym;
  }
  return 0;
}


li_object *g1_armor_unoccupy(li_object *o, li_environment *env)
{
  g1_dynamic_object_class *me=g1_dynamic_object_class::get(li_first(o));
  me->g1_object_class::unoccupy_location();
  g1_player_man.get(me->player_num)->damage_multiplier() /= damage_multiplier_fraction();
  return 0;
}

li_object *g1_armor_think(li_object *o, li_environment *env)
{
  g1_dynamic_object_class *me=g1_dynamic_object_class::get(li_first(o));
  me->mini_objects[0].rotation.z+=spin_speed();
  return 0;
}



li_object *g1_armor_change_player_num(li_object *o, li_environment *env)
{ 
  g1_dynamic_object_class *me=g1_dynamic_object_class::get(li_first(o));
  int new_team=li_int::get(li_second(o))->value();

  if (new_team!=me->player_num)
  {
    g1_player_man.get(new_team)->damage_multiplier()*=damage_multiplier_fraction();
    g1_player_man.get(me->player_num)->damage_multiplier()/=damage_multiplier_fraction();


    if (new_team==g1_player_man.local_player && g1_current_controller.get())
      g1_current_controller->add_spin_event("powerup_shields", 0);

    if (me->player_num==g1_player_man.local_player && g1_current_controller.get())
      g1_current_controller->add_spin_event("powerup_shields", 1);

    me->g1_object_class::change_player_num(new_team);
  }

  return 0;
}

li_automatic_add_function(g1_armor_occupy, "armor_building_occupy_location");
li_automatic_add_function(g1_armor_unoccupy, "armor_building_unoccupy_location");
li_automatic_add_function(g1_armor_think, "armor_building_think");
li_automatic_add_function(g1_armor_change_player_num, "armor_building_change_player_num");
