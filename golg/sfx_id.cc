/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

// Automatically generated file.  DO NOT EDIT.
// To update run the 'enumer' program

#include "sfx_id.hh"
#include "string/string.hh"
#include "file/file.hh"

static char *g1_sfx[g1_sfx_last] = 
{
  "sfx/misc/purchase_button_click.wav",
  "sfx/misc/production_pad_click.wav",
  "sfx/select/m01.wav",
  "sfx/select/m02.wav",
  "sfx/select/m03.wav",
  "sfx/select/m04.wav",
  "sfx/select/f01.wav",
  "sfx/select/f02.wav",
  "sfx/select/f03.wav",
  "sfx/select/f04.wav",
  "sfx/deploy/m01.wav",
  "sfx/deploy/m02.wav",
  "sfx/deploy/m03.wav",
  "sfx/deploy/f01.wav",
  "sfx/deploy/f02.wav",
  "sfx/deploy/f03.wav",
  "sfx/misc/ricochet_1.wav",
  "sfx/explosion/generic.wav",
  "sfx/explosion/ground_vehicle.wav",
  "sfx/explosion/super_mortar.wav",
  "sfx/misc/turbine1_lp.wav",
  "sfx/assembled/jet.wav",
  "sfx/rumble/jet_lp.wav",
  "sfx/assembled/kamikaze_trike.wav",
  "sfx/assembled/engineering_vehicle.wav",
  "sfx/rumble/engineering_vehicle_lp.wav",
  "sfx/assembled/helicopter.wav",
  "sfx/rumble/helicopter_lp.wav",
  "sfx/assembled/peon_tank.wav",
  "sfx/rumble/peon_tank_lp.wav",
  "sfx/fire/peon_tank.wav",
  "sfx/assembled/missile_truck.wav",
  "sfx/rumble/missile_truck_lp.wav",
  "sfx/fire/missile_truck.wav",
  "sfx/misc/missile_truck_lower.wav",
  "sfx/misc/missile_truck_raise.wav",
  "sfx/assembled/electric_car.wav",
  "sfx/rumble/electric_car_lp.wav",
  "sfx/misc/electric_car_charged_lp.wav",
  "sfx/fire/supergun.wav",
  "sfx/rumble/supertank_lp.wav",
  "sfx/misc/supertank_refuel_lp.wav",
  "sfx/misc/supertank_main_gun_refuel.wav",
  "sfx/misc/supertank_chain_gun_refuel.wav",
  "sfx/misc/supertank_missile_refuel.wav",
  "sfx/fire/supertank/main1.wav",
  "sfx/fire/supertank/auto1_lp.wav",
  "sfx/fire/supertank/auto2_lp.wav",
  "sfx/fire/supertank/auto3.lp.wav",
  "sfx/misc/supertank_main_gun_charge_lp.wav"
};

i4_file_class *g1_open_sfx(w32 enum_value)
{
  return i4_open(i4_const_str(g1_sfx[enum_value]));
} 
