/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_SPRITE_OBJECT_HH
#define G1_SPRITE_OBJECT_HH

#include "g1_object.hh"
#include "objs/sprite.hh"

class g1_sprite_object_class : public g1_object_class
{  
public:
  w16 model_id;
  w16 frame;
  w32 anim_length;

  g1_sprite_object_class(g1_object_type id, g1_loader_class *fp);

  virtual void init();
  virtual void think();

  virtual i4_float occupancy_radius() const
  {
    return g1_sprite_list_man.get_sprite(model_id)->extent;
  }
};


#endif
