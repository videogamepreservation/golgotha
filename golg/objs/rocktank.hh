/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_ROCKET_TANK_HH
#define G1_ROCKET_TANK_HH

#include "objs/map_piece.hh"
#include "resources.hh"

class g1_rocket_tank_class : public g1_map_piece_class
{
protected:
  g1_mini_object *missile_rack;
  i4_float guard_angle, guard_pitch;
  int guard_time;
public:
  g1_rocket_tank_class(g1_object_type id, g1_loader_class *fp);
  void save(g1_saver_class *fp);

  virtual void fire();
  virtual void think();

  void raise_rack();
  void lower_rack();
  int get_chunk_names(char **&list);
};

#endif
