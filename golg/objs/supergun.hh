/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_SUPERGUN_HH
#define G1_SUPERGUN_HH

#include "sound/sound.hh"
#include "objs/map_piece.hh"

class g1_supergun_class : public g1_map_piece_class
{
protected:
  g1_mini_object        *muzzle;
  i4_float target_angle, target_pitch;
  i4_float target_groundspeed, target_zspeed;
  int time;
  int mode;
public:
  g1_supergun_class(g1_object_type id, g1_loader_class *fp);
  virtual void save(g1_saver_class *fp);
      
  enum { ORIGNAL_RUMBLE_VOLUME=I4_SOUND_VOLUME_LEVELS/8 };

  virtual i4_bool suggest_new_attack_target(g1_object_class *new_target, g1_fire_range_type range);
  virtual g1_fire_range_type range() { return G1_FIRE_RANGE_1; }

  virtual void fire();
  virtual void think();
  virtual void setup(i4_float x, i4_float y, g1_object_class *creator);

  virtual i4_bool deploy_to(float x, float y);

  virtual i4_bool occupy_location()
  {
    int ret;
    if (ret = g1_object_class::occupy_location_model(draw_params)) 
      get_terrain_info(); 
    return ret;
  }
};

#endif
