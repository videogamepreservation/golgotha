/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "objs/sprite_object.hh"
#include "r1_api.hh"

void g1_sprite_object_class::think()
{
  frame++;
  if (frame >= anim_length)
  {
    unoccupy_location();
    request_remove();
  }
  else
  {
    unoccupy_location();
    request_think();  
  }
}


g1_sprite_object_class::g1_sprite_object_class(g1_object_type id,
                                               g1_loader_class *fp)
  : g1_object_class(id,fp)
{
  model_id=0;
}


void g1_sprite_object_class::init()
{
  /// get animation length now rather than in the constructor to make sure model_id is valid
  g1_sprite_class *sprite=g1_sprite_list_man.get_sprite(model_id);
  anim_length = sprite->num_animation_frames;
}


