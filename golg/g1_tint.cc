/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "g1_tint.hh"
#include "lisp/lisp.hh"
#include "lisp/li_init.hh"

r1_color_tint_handle g1_player_tint_handles[G1_MAX_PLAYERS];

g1_tint_struct g1_player_tint_data[G1_MAX_PLAYERS] = 
{  
  { 1.0, 1.0, 1.0 },   // no tint (this entry is not actually used)
  { 0.2, 1.0, 0.2 },   // green
  { 1.0, 0.2, 0.2 },   // red
  { 0.2, 0.2, 1.0 },   // blue
  { 1.0, 1.0, 0.2 }    // yellow  
};

g1_tint_struct g1_hurt_tint_data[G1_NUM_HURT_TINTS] = 
{  
  { 1.0, 1.0, 1.0 },
  { 1.0, 0.7, 0.7 },
  { 1.0, 0.5, 0.5 },
  { 1.0, 0.2, 0.2 },
  { 1.0, 0.0, 0.0 } 
};
r1_color_tint_handle g1_hurt_tint_handles[G1_NUM_HURT_TINTS];

void g1_init_color_tints(r1_render_api_class *api)
{
  int i;

  g1_player_tint_handles[0] = 0;
  for (i=1; i<G1_MAX_PLAYERS; i++)
    g1_player_tint_handles[i] = api->register_color_tint(g1_player_tint_data[i].r,
                                                         g1_player_tint_data[i].g,
                                                         g1_player_tint_data[i].b);

  g1_hurt_tint_handles[0] = 0;
  for (i=1; i<G1_NUM_HURT_TINTS; i++)
    g1_hurt_tint_handles[i] = api->register_color_tint(g1_hurt_tint_data[i].r,
                                                       g1_hurt_tint_data[i].g,
                                                       g1_hurt_tint_data[i].b);
}                                 

int g1_hurt_tint=0;
g1_tint_type g1_tint=G1_TINT_POLYS;
