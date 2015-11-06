/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "objs/def_object.hh"
#include "lisp/li_init.hh"
#include "lisp/li_class.hh"
#include "li_objref.hh"
#include "map_man.hh"
#include "map.hh"
#include "math/pi.hh"

static li_int_class_member health("health"), explode_health("explode_health"),
  fire_delay("fire_delay"), total_burst("total_burst"),  burst_delay("burst_delay"),
  cost("cost"),  range("range");

static li_symbol_class_member fire_type("bullet");

  

li_object *g1_gunport_damage(li_object *o, li_environment *env)
{
  g1_dynamic_object_class *me=g1_dynamic_object_class::get(li_car(o));
  int hp=li_int::get(li_car(li_cdr(li_cdr(o))))->value();

  health()-=hp;

  me->request_think();
  return 0;
}

li_object *g1_gunport_enter_range(li_object *o, li_environment *env)
{
  return 0;
}



li_object *g1_gunport_think(li_object *o, li_environment *env)
{
  g1_dynamic_object_class *me=g1_dynamic_object_class::get(li_car(o));
  li_class *v=me->vars;

  if (health() < 0)
  {
    if (health() < explode_health())
    {
      me->unoccupy_location();
      me->request_remove();
    }
    else
    {
      me->set_flag(g1_object_class::DANGEROUS,0);
      health()--;
      me->request_think();
    }
  }

  return 0;
}


static li_float_class_member turn_speed("turn_speed"), speed("speed"), 
  max_speed("max_speed"), hill_scale("hill_scale"), damage_speedup("damage_speedup");

li_object *g1_speed_damage(li_object *o, li_environment *env)
{
  g1_dynamic_object_class *me=g1_dynamic_object_class::get(li_car(o));
  int hp=li_int::get(li_car(li_cdr(li_cdr(o))))->value();

  speed()+=hp * damage_speedup();

  me->request_think();
  return 0;
}

li_object *g1_circle_think(li_object *o, li_environment *env)
{
  g1_dynamic_object_class *me=g1_dynamic_object_class::get(li_car(o));


  me->grab_old();
  me->unoccupy_location();
  
  float s=speed();

  me->x += cos(me->theta)*s;
  me->y += sin(me->theta)*s;

  float h=g1_get_map()->terrain_height(me->x, me->y);
  
  if (h<me->h)   // going down hill speed up
  {
    s += (me->h-h) * hill_scale();

    if (s>max_speed())
      s=max_speed();

    speed()=s;
    me->h=h;
  }
  else if (h>me->h)  // going up hill, slow down
  {
    s += (me->h-h) * hill_scale();
    if (s<0)
    {
      me->theta+=i4_pi();     // turn around
      s=0.001;
    }

    speed()=s;
    me->h=h;
  }

  me->theta += turn_speed();

  if (me->occupy_location())
    me->request_think();


  return 0;
}

li_automatic_add_function(g1_speed_damage, "speed_damage");
li_automatic_add_function(g1_circle_think, "circle_think");
li_automatic_add_function(g1_gunport_damage, "gunport_damage");
li_automatic_add_function(g1_gunport_think, "gunport_think");
li_automatic_add_function(g1_gunport_enter_range, "gunport_enter_range");

