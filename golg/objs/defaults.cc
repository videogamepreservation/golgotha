/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "string/string.hh"
#include "memory/array.hh"
#include "objs/defaults.hh"
#include "lisp/lisp.hh"
#include "loaders/load.hh"
#include "image/image.hh"
#include "g1_object.hh"

static li_symbol_ref li_sing("sing"), li_mult("mult");

i4_array<g1_object_defaults_struct> g1_obj_defaults(0,32);
i4_array<g1_stank_ammo_type_struct> g1_stank_ammo_types(0,32);
i4_array<g1_damage_map_struct>      g1_damage_maps(0,32);

li_object *g1_defaults(li_object *i, li_environment *env)
{
  while (i)
  {
    g1_object_defaults_struct *d=g1_obj_defaults.add();
    li_object *o=li_car(i,env);

    strcpy(d->object_name, li_get_string(li_symbol::get(li_car(o,env),env)->name(),env));  
    o=li_cdr(o,env);
           
    d->speed=li_get_float(li_car(o,env),env);                 o=li_cdr(o,env);
    d->turn_speed=li_get_float(li_car(o,env),env);            o=li_cdr(o,env);
    d->accel=li_get_float(li_car(o,env),env);                 o=li_cdr(o,env);
    d->cost=li_get_int(li_car(o,env),env);                    o=li_cdr(o,env);

    d->health=li_get_int(li_car(o,env),env);                  o=li_cdr(o,env);
    d->fire_delay=li_get_int(li_car(o,env),env);              o=li_cdr(o,env);
    d->detection_range=li_get_float(li_car(o,env),env);       o=li_cdr(o,env);

    d->fire_type=li_symbol::get(li_car(o,env),env);           o=li_cdr(o,env);
    
    i=li_cdr(i,env);
  }
  return 0;
}

li_object *g1_add_stank_weapons(li_object *i, li_environment *env)
{
  while (i)
  {
    li_object *o=li_car(i,env);
    g1_stank_ammo_type_struct *a=g1_stank_ammo_types.add();
    a->weapon_type=li_symbol::get(li_car(o,env),env);              o=li_cdr(o,env);
    a->max_amount=li_get_int(li_eval(li_car(o,env),env),env);     o=li_cdr(o,env);
    a->refuel_delay=li_get_int(li_eval(li_car(o,env),env),env);   o=li_cdr(o,env);
    a->fire_delay=li_get_int(li_eval(li_car(o,env),env),env);     o=li_cdr(o,env);

    char name[100];
    sprintf(name,"bitmaps/stank/frame_%s.tga", li_get_string(li_car(o,env),env));
    a->icon=i4_load_image(name);
    
    sprintf(name,"bitmaps/stank/dark_%s.tga", li_get_string(li_car(o,env), env));
    a->dark_icon=i4_load_image(name);
    
    
    i=li_cdr(i,env);
  }
  return 0; 
}

li_object *g1_def_weapon_damage(li_object *i, li_environment *env)
{
  while (i)
  {
    li_object *o=li_car(i,env), *c;
    g1_damage_map_struct *a=g1_damage_maps.add();

    a->object_for=li_symbol::get(li_car(o,env),env);      o=li_cdr(o,env);

    li_symbol *htype=li_symbol::get(li_car(o,env),env);
    if (htype==li_sing.get())
      a->hurt_type=g1_damage_map_struct::HURT_SINGLE_GUY;
    else if (htype==li_mult.get())
      a->hurt_type=g1_damage_map_struct::DO_AREA_OF_EFFECT;
    else li_error(env, "unknown hurt type %O", o);
    o=li_cdr(o,env);
    
    a->default_damage=li_get_int(li_car(o,env),env);      o=li_cdr(o,env);
    a->special_damage=li_get_int(li_car(o,env),env);      o=li_cdr(o,env);
    a->speed=li_get_float(li_car(o,env),env);             o=li_cdr(o,env);
    a->range=li_get_float(li_car(o,env),env);             o=li_cdr(o,env);
        
    // count how many exceptions follow
    for (a->t_maps=0, c=o; c; c=li_cdr(c,env), a->t_maps++);

    if (a->t_maps)
    {
      a->maps=(g1_damage_map_struct::map *)i4_malloc(a->t_maps *
                                                     sizeof(g1_damage_map_struct::map),"");
      int i=0;
      for (;o; o=li_cdr(o,env))
      {
        a->maps[i].object_type=li_symbol::get(li_car(li_car(o,env),env),env);
        a->maps[i].damage=li_get_int(li_car(li_cdr(li_car(o,env),env),env),env);
        i++;
      }
    }
    else a->maps=0;
    i=li_cdr(i,env);
  }

  return 0;
}


g1_damage_map_struct *g1_find_damage_map(int object_type)
{
  int t=g1_damage_maps.size();
  for (int i=0; i<t; i++)
    if (g1_get_object_type(g1_damage_maps[i].object_for)==object_type)
      return &g1_damage_maps[i];
  
  return 0;
}

int g1_damage_map_struct::get_damage_for(int object_type)
{
  for (int i=0; i<t_maps; i++)
    if (g1_get_object_type(maps[i].object_type)==object_type)
      return maps[i].damage;

  return default_damage;
}

g1_stank_ammo_type_struct *g1_find_stank_ammo_type(li_symbol *sym)
{
  for (int i=0; i<g1_stank_ammo_types.size(); i++)
    if (g1_stank_ammo_types[i].weapon_type==sym)
      return &g1_stank_ammo_types[i];
  
  return 0;
}

void g1_load_object_defaults()
{
  li_add_function("defaults", g1_defaults, 0);
  li_add_function("def_stank_weapons", g1_add_stank_weapons, 0);
  li_add_function("def_weapon_damage", g1_def_weapon_damage, 0);
  
  li_load("scheme/balance.scm",0);  
}

g1_object_defaults_struct *g1_get_object_defaults(const char *object_name,
                                                  i4_bool barf_on_error)
{
  if (!g1_obj_defaults.size())
    g1_load_object_defaults();

  for (int i=0; i<g1_obj_defaults.size(); i++)
    if (strcmp(g1_obj_defaults[i].object_name, object_name)==0)
      return &g1_obj_defaults[i];

  if (barf_on_error)
    i4_error("unable to find object default for %s",object_name);
  else
    return g1_get_object_defaults("default", i4_T);
                                  
  return 0;
}

void g1_unload_object_defaults()     // call at end of game to clean up memory
{
  g1_obj_defaults.uninit();

  int i;
  
  for (i=0; i<g1_stank_ammo_types.size(); i++)
  {
    if (g1_stank_ammo_types[i].icon) delete g1_stank_ammo_types[i].icon;
    if (g1_stank_ammo_types[i].dark_icon) delete g1_stank_ammo_types[i].dark_icon;
  }
  g1_stank_ammo_types.uninit();
  
  for (i=0; i<g1_damage_maps.size(); i++)
  {
    if (g1_damage_maps[i].maps)
      i4_free(g1_damage_maps[i].maps);
  }
  g1_damage_maps.uninit();

}
