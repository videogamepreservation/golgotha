/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef BULLET_HH
#define BULLET_HH

#include "g1_object.hh"
#include "player_type.hh"
#include "path.hh"
#include "objs/model_draw.hh"

class g1_solid_class;
class g1_map_piece_class;
class g1_light_object_class;
class g1_particle_emitter_class;

class g1_bullet_class : public g1_object_class
{
  enum { DATA_VERSION=7 };
protected:  
  void move_forward();
  void done(i4_bool hit=i4_F,g1_object_class *hit_object=0);

  r1_texture_handle tex;
public:
  g1_typed_reference_class<g1_object_class> who_fired_me;
  g1_typed_reference_class<g1_light_object_class> light;
  g1_typed_reference_class<g1_particle_emitter_class> particle_emitter;

  enum { IS_SPRITE=2,
         CREATES_ACID=4,
         CREATES_NAPALM=8,
         MAKE_SMOKE=16,
         SPINS=32
  };
  
  i4_float range,      // how much further we can travel
    range_decrement;   // hom much range to subtract each tick
  
  i4_3d_vector vel;

  virtual i4_float occupancy_radius() const { return 0.1; }

  virtual i4_bool move();

  g1_bullet_class(g1_object_type id, g1_loader_class *fp);

  virtual void save(g1_saver_class *fp);

  
  w32 bullet_flags;

  virtual void setup(const i4_3d_vector &start_pos,
                     const i4_3d_vector &vel,
                     g1_object_class *this_guy_fired_me,
                     i4_float range,
                     w32 flags=0,
                     r1_texture_handle sprite_texture=0,
                     g1_light_object_class *light=0);

  virtual void draw(g1_draw_context_class *context);
  virtual void think();
};

#endif
