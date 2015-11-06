/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_DROPPED_BOMB_HH
#define G1_DROPPED_BOMB_HH


#include "g1_object.hh"
#include "player_type.hh"
#include "path.hh"

class g1_solid_class;
class g1_map_piece_class;
class g1_smoke_trail_class;
class g1_map_piece_class;
class g1_dropped_bomb_class : public g1_object_class
{
  enum { DATA_VERSION=3 };

protected:  

  g1_typed_reference_class<g1_object_class> who_fired_me;
  g1_typed_reference_class<g1_smoke_trail_class> smoke_trail;
  
  void delete_smoke();

public:
  g1_model_draw_parameters draw_params;

  i4_float z_velocity;
  i4_float speed;

  g1_dropped_bomb_class(g1_object_type id, g1_loader_class *fp);
  virtual void save(g1_saver_class *fp);
  virtual void setup(const i4_3d_vector &pos, g1_object_class *this_guy_fired_me);
  
  virtual void think();
  
  virtual i4_bool move(const i4_3d_vector &vel);

  void draw(g1_draw_context_class *context);
};

#endif
