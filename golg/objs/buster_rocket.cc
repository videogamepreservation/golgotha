/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "g1_object.hh"
#include "math/num_type.hh"
#include "math/angle.hh"
#include "math/trig.hh"
#include "math/random.hh"
#include "tile.hh"
#include "saver.hh"
#include "math/pi.hh"
#include "map.hh"
#include "map_man.hh"
#include "object_definer.hh"
#include "resources.hh"

#include "objs/buster_rocket.hh"
#include "objs/map_piece.hh"
#include "objs/smoke_trail.hh"
#include "objs/stank.hh"
#include "objs/particle_emitter.hh"
#include "lisp/lisp.hh"
#include "li_objref.hh"

static li_symbol_ref li_particle_emitter("particle_emitter");
static li_g1_ref_class_member smoke_trail("smoke_trail");

enum { DATA_VERSION=2 };

g1_object_definer<g1_buster_rocket_class>
g1_buster_rocket_def("buster_rocket");

g1_object_definer<g1_buster_rocket_class>
g1_heavy_rocket_def("heavy_rocket");

g1_object_definer<g1_buster_rocket_class>
g1_vortex_def("vortex_missile");

g1_object_definer<g1_buster_rocket_class>
g1_nuke_def("nuke_missile");


g1_buster_rocket_class::g1_buster_rocket_class(g1_object_type id,
                                               g1_loader_class *fp)
  : g1_guided_missile_class(id,fp)
{
}

static r1_texture_ref smoke_ref("smoke_particle");

void g1_buster_rocket_class::add_smoke()
{
  g1_particle_emitter_params p;
  g1_particle_emitter_class *emitter;
  p.defaults();
  p.start_size=0.05;
  p.grow_speed=0.01;
  p.max_speed=0.02;
  p.air_friction=0.80;
  p.num_create_attempts_per_tick=1;
  p.particle_lifetime=20;
  
  p.texture=smoke_ref.get();

  emitter = (g1_particle_emitter_class *)
    g1_create_object(g1_get_object_type(li_particle_emitter.get()));
  emitter->setup(x,y,h, p);
  
  vars->set(smoke_trail, new li_g1_ref(emitter));
}

void g1_buster_rocket_class::update_smoke()
{
  li_class_context c(vars);

  if (!smoke_trail())
    return;

  g1_particle_emitter_class *st=(g1_particle_emitter_class *) smoke_trail()->value();
  if (st)
    st->move(lx,ly,lh);
}

void g1_buster_rocket_class::delete_smoke()
{
  li_class_context c(vars);

  if (!smoke_trail())
    return;

  g1_particle_emitter_class *st=(g1_particle_emitter_class *) smoke_trail()->value();
  if (st)
  {
    st->unoccupy_location();
    st->request_remove();
    vars->set(smoke_trail, li_g1_null_ref());
  }
}


