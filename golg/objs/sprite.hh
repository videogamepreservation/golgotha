/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_SPRITE_HH
#define G1_SPRITE_HH

#include "g1_limits.hh"
#include "tex_id.hh"

class r1_render_api_class;

class g1_sprite_class
{
public:
  i4_float texture_scale;
  i4_float add_x, add_y;     // added to sprites position before transform
                             // these numbers are calculated from vertex[0] of the quadgon 
                             // loaded from 3d studio model
  r1_texture_handle texture;
  sw32 num_animation_frames;
  i4_float extent;
} ;

class g1_sprite_list_class
{
  g1_sprite_class array[G1_MAX_SPRITES];

public:
  void load(r1_render_api_class *tmap);
  g1_sprite_class *get_sprite(w16 handle) { return &array[handle]; }
} ;

extern g1_sprite_list_class g1_sprite_list_man;

#endif
