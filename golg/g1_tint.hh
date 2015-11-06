/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_TINT_HH
#define G1_TINT_HH

// this is in a seperate file so maxtool and golgotha can share this code

#include "g1_limits.hh"
#include "r1_api.hh"

enum { G1_NUM_HURT_TINTS = 5 };
enum g1_tint_type { G1_TINT_OFF, G1_TINT_POLYS, G1_TINT_ALL };

extern g1_tint_type g1_tint;
extern r1_color_tint_handle g1_player_tint_handles[G1_MAX_PLAYERS];
extern r1_color_tint_handle g1_hurt_tint_handles[G1_NUM_HURT_TINTS];

extern int g1_hurt_tint;


struct g1_tint_struct { i4_float r,g,b; };
extern g1_tint_struct g1_player_tint_data[G1_MAX_PLAYERS];
extern g1_tint_struct g1_hurt_tint_data[G1_NUM_HURT_TINTS];

// this will register the color tints with the rendering api, afterwards you can call
// api->set_color_tint(player_tint_handles[player_num]);
void g1_init_color_tints(r1_render_api_class *api);



#endif
