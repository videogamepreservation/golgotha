/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_PEON_TANK_HH
#define G1_PEON_TANK_HH

#include "objs/map_piece.hh"

class g1_peon_tank_class : public g1_map_piece_class
{ 
protected:
  g1_mini_object *turret;
  i4_float guard_angle;
  int guard_time;
public:
  g1_peon_tank_class(g1_object_type id, g1_loader_class *fp);
  virtual void save(g1_saver_class *fp);

  virtual void draw(g1_draw_context_class *context);

  virtual void fire();
  virtual void think();

  i4_float turret_kick,lturret_kick; //just an aesthetic thing, the turret kicks back when fired
  int get_chunk_names(char **&list);
};

#endif
