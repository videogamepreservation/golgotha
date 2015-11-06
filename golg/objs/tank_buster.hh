/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_TANK_BUSTER_HH
#define G1_TANK_BUSTER_HH

#include "objs/map_piece.hh"
#include "objs/stank.hh"
#include "objs/buster_rocket.hh"
#include "resources.hh"

class g1_tank_buster_class : public g1_map_piece_class
{
protected:
  w8 rack_state;
  sw8 explode_delay;

  g1_mini_object *missile;

public:
  
  g1_tank_buster_class(g1_object_type id, g1_loader_class *fp);
  void save(g1_saver_class *fp);

  i4_bool can_attack(g1_object_class *who) const
  {
    g1_player_piece_class *o = g1_player_piece_class::cast(who);
    
    //not a supertank? cant attack it
    if (!o) return i4_F;

    return g1_map_piece_class::can_attack(who);
  }

  virtual void fire();
  virtual void think();

  void raise_missile();
  void lower_missile();
};

#endif
