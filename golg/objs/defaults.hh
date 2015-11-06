/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_OBJECT_DEFAULTS_HH
#define G1_OBJECT_DEFAULTS_HH

#include "arch.hh"
#include "math/num_type.hh"
#include "string/string.hh"
class i4_image_class;
class li_symbol;
class g1_damage_map
{
public:
  class entry
  {
  public:
    li_symbol *key;
    sw16 damage;
  };
  i4_array<entry> map;
};

struct g1_object_defaults_struct
{
  char object_name[20];

  i4_float speed, 
    turn_speed, 
    accel;

  w16 cost,
    health,
    fire_delay;

  float detection_range; 

  li_symbol *fire_type;
};


// defines an ammo type the supertank can fire
struct g1_stank_ammo_type_struct
{
  w16 max_amount;         // when refueling, this is the max we can get
  w16 refuel_delay;       // ticks to refuel completely
  w16 fire_delay;         // delay after each shot
  li_symbol *weapon_type; // name of projectile fired (passed to g1_fire)
  
  i4_image_class *icon;       // used to draw the weapon on the status bar
  i4_image_class *dark_icon;
};


// defines how much damage a weapon does to people and for how long
class g1_damage_map_struct
{
public:
  li_symbol *object_for;     // which object does this apply to?
  int special_damage;        // radius of effect or ticks to apply damage
  int default_damage;        // how much damage is done by default (if not in exxception list)
  float speed;               // speed at which weapon travels
  int t_maps;                // how many exceptions are in the array
  float range;            // how far the shot can travel
  
  struct map
  {
    li_symbol *object_type;  // who this exception is for
    int damage;             // how much damage to do to them
  } *maps;

  int get_damage_for(int object_type);
  enum { HURT_SINGLE_GUY,
         DO_AREA_OF_EFFECT } hurt_type;
};


g1_damage_map_struct *g1_find_damage_map(int object_type);
  


g1_stank_ammo_type_struct *g1_find_stank_ammo_type(li_symbol *sym);

g1_object_defaults_struct *g1_get_object_defaults(const char *object_name, 
                                                  i4_bool barf_on_error=i4_T);
void g1_unload_object_defaults();     // call at end of game to clean up memory

#endif


