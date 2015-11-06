/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_VEHIC_SOUNDS_HH
#define G1_VEHIC_SOUNDS_HH

#include "math/vector.hh"

enum g1_rumble_type
{
  G1_RUMBLE_GROUND,
  G1_RUMBLE_HELI,
  G1_RUMBLE_JET,
  G1_RUMBLE_MISSILE,
  G1_RUMBLE_STANK,
  G1_T_RUMBLES 
};



void g1_add_to_sound_average(g1_rumble_type type, 
                             const i4_3d_vector& pos, const i4_3d_vector& vel);
             
void g1_add_to_sound_average(g1_rumble_type type, const i4_3d_vector& pos);

void g1_reset_sound_averages();
void g1_recalc_sound_averages();
void g1_stop_sound_averages();

#endif
