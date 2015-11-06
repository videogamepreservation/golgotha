/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_VERY_BIG_GUN_HH
#define G1_VERY_BIG_GUN_HH

#include "sound/sound.hh"
#include "objs/map_piece.hh"

class g1_very_big_gun_class : public g1_map_piece_class
{
 
public:
  enum {DATA_VERSION=1};
  
  g1_very_big_gun_class(g1_object_type id, g1_loader_class *fp);
  virtual void save(g1_saver_class *fp);
      
  enum { ORIGNAL_RUMBLE_VOLUME=I4_SOUND_VOLUME_LEVELS/8 };

  virtual g1_fire_range_type range();

  virtual void fire();
  virtual void think();
};

#endif
