/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "g1_object.hh"
#include "map.hh"
#include "map_man.hh"
#include "lisp/li_class.hh"
#include "objs/structure_death.hh"
#include "objs/particle_emitter.hh"
#include "objs/explode_model.hh"
#include "objs/carcass.hh"
#include "math/random.hh"
#include "math/pi.hh"

// circular distribution precomputation
#include "objs/circle_distrib.cc"

static li_symbol_ref particles("particle_emitter");
static r1_texture_ref explosion2("smoke_particle");
static li_symbol_ref explode_model("explode_model");
static g1_model_ref carcass("garage_charred");

g1_particle_emitter_params t_params;
i4_float t_rad = 0.6;
i4_float t_fac_grav = 0.002;
i4_float t_roll = 0.01;
i4_float t_rspeed = 0.05;

int death_time = 50;

class global_setter_class
{
public:
  global_setter_class()
  {
    t_params.defaults();
    t_params.start_size=0.4;
    t_params.particle_lifetime=20;
    t_params.grow_speed=-t_params.start_size/t_params.particle_lifetime;
    t_params.max_speed=0.05;
    t_params.air_friction=0.90;
    t_params.num_create_attempts_per_tick=1;
    t_params.creation_probability=0.50;
    t_params.speed.set(0, 0, 0.05);
    t_params.emitter_lifetime=death_time;
  };
} global_setter;

i4_bool g1_structure_death_class::think()
{
  if (me->health<=0)
  {
    me->request_think();
    me->health--;
    me->h += i4_float(me->health)*t_fac_grav;
    me->roll += t_roll;
      
    if (me->health<-death_time)
    {
      me->unoccupy_location();
      me->request_remove();
    }
    return i4_F;
  }
  return i4_T;
}

void g1_structure_death_class::damage(g1_object_class *obj, int hp, i4_3d_vector damage_dir)
{
  if (me->health<=0)
    return;

  me->health-=hp;

  if (me->health<=0)
  {
    me->health=0;

    g1_explode_model_class *e;
    e=(g1_explode_model_class *)g1_create_object(g1_get_object_type(explode_model.get()));
    g1_explode_params params;
    params.stages[0].setup(0,  0.0008, G1_APPLY_SING);
    params.stages[1].setup(10, 0,      G1_APPLY_SING);
    params.t_stages=2;
    params.gravity=0.05;
    e->setup(me, i4_3d_vector(0,0,0), params);

    i4_float limit = me->occupancy_radius();
    limit *= limit;

    for (int n=0; n<sizeof(distrib)/sizeof(distrib[0]) && distrib[n][2]<limit; n++)
    {
      g1_particle_emitter_params params;
      params.defaults();
      params.start_size=0.4;
      params.grow_speed=-params.start_size/20;
      params.max_speed=0.1;
      params.air_friction=0.90;
      params.num_create_attempts_per_tick=1;
      params.creation_probability=0.50;
      params.particle_lifetime=20;
      params.speed.set(0, 0, 0.05);

      params = t_params;

      params.emitter_lifetime += i4_rand()&0x15;

      params.texture=explosion2.get();
      g1_particle_emitter_class *emitter = 
        (g1_particle_emitter_class *)g1_create_object(g1_get_object_type(particles.get()));

      i4_float 
        th = i4_float_rand()*i4_2pi(),
        px = distrib[n][0] + me->x,
        py = distrib[n][1] + me->y;

      params.speed.x = t_rspeed*distrib[n][0];
      params.speed.y = t_rspeed*distrib[n][1];

      if (emitter)
        emitter->setup(px,py, g1_get_map()->terrain_height(px,py), params);
    }

    g1_create_carcass(me, carcass.get(), -1, -1, 0);

//     g1_object_class *o=g1_create_object(g1_get_object_type("garage_charred"));
//     o->x = me->x;
//     o->y = me->y;
//     o->h = me->h;
//     o->theta = me->theta;
//     o->pitch = me->pitch;
//     o->roll = me->roll;
//     o->grab_old();
//     o->occupy_location();

    me->request_think();
  }
}
