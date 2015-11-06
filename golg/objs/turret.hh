/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_TURRET_HH
#define G1_TURRET_HH

#include "sound/sound.hh"
#include "objs/map_piece.hh"

class g1_turret_class : public g1_map_piece_class
{
protected:
  g1_mini_object        *muzzle;
  g1_mini_object        *top;
  i4_float guard_angle, guard_pitch;
  int guard_time;
  int order;
public:
  g1_turret_class(g1_object_type id, g1_loader_class *fp);
  virtual void save(g1_saver_class *fp);
      
  virtual void fire();
  virtual void think();
  virtual void setup(i4_float x, i4_float y, g1_object_class *creator);
};

#endif
