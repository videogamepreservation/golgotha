/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef GUIDED_MISSILE_HH
#define GUIDED_MISSILE_HH

#include "g1_object.hh"
#include "player_type.hh"
#include "path.hh"
#include "sound/sfx_id.hh"
#include "objs/light_o.hh"

class g1_solid_class;
class g1_map_piece_class;
class g1_particle_emitter_class;
class g1_voice_class;

#define G1_GUIDED_MISSILE_DATA_VERSION 6

class g1_guided_missile_class : public g1_object_class
{
protected:  
  s1_sound_handle rumble_sound;

  i4_float damping_fraction;
  g1_typed_reference_class<g1_light_object_class> light;
public:
  g1_guided_missile_class(g1_object_type id, g1_loader_class *fp);
    
  virtual void setup(const i4_3d_vector &pos,
                     const i4_3d_vector &dir,
                     g1_object_class *this_guy_fired_me,
                     g1_object_class *track_me);

  virtual void start_sounds();

  virtual void add_explode();

  virtual void add_smoke();
  virtual void update_smoke();
  virtual void delete_smoke();
  virtual void request_remove();

  virtual void think();

  virtual i4_bool move(i4_float x_amount,
                       i4_float y_amount,
                       i4_float z_amount);
};

#endif
