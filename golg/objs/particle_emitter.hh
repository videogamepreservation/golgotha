/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef PARTICLE_EMITTER_HH
#define PARTICLE_EMITTER_HH

#include "g1_object.hh"
#include "tex_id.hh"
#include "file/file.hh"

struct g1_particle_emitter_params
{
  r1_texture_handle texture;
  
  i4_float start_size;
  i4_float start_size_random_range;  // added to start size

  i4_float grow_speed;
  i4_float grow_accel;
  i4_3d_vector speed;      // travel speed
  
  sw32     num_create_attempts_per_tick;
  i4_float creation_probability;     // 0..1 likelyhood each tick of creating a new particle
  i4_float max_speed;
  i4_float air_friction;
  i4_float gravity;

  sw32  particle_lifetime; // ticks particles will be around (they also die if size<0)
  sw32  emitter_lifetime;  // ticks the emitter will be around

  void defaults();
};

struct g1_particle_class
{  
  float x,y,z, lx,ly,lz;
  float xv, yv, zv;
  float grow_speed;
  float size;
  int   ticks_left;
  i4_bool in_use;

  void load(i4_file_class *fp);
  void save(i4_file_class *fp);
};

class g1_particle_emitter_class : public g1_object_class
{
  g1_object_chain_class cell_on;
  enum { DATA_VERSION=0xabce };
public:
  float x1,y1, x2,y2;             // ground plane bounds for occupy location
  float radius;
  i4_bool stopping;

  enum { MAX_PARTICLES=40  };
  g1_particle_class particles[MAX_PARTICLES];
  g1_particle_emitter_params params;
  
  int t_in_use;

  g1_particle_emitter_class(g1_object_type id, g1_loader_class *fp);
  virtual void save(g1_saver_class *fp);


  virtual i4_float occupancy_radius() const {  return radius; }

  void occupy_bounds();

  void setup(float x, float y, float h, g1_particle_emitter_params &params);
  virtual void draw(g1_draw_context_class *context);
  virtual void think();

  void stop() { stopping=i4_T; } 
  void move(float new_x, float new_y, float new_h);
  i4_bool occupy_location();
  void unoccupy_location();  
};




#endif
