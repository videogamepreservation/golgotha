/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "objs/chunk_explosion.hh"
#include "object_definer.hh"
#include "objs/explode_model.hh"
#include "map.hh"
#include "map_man.hh"
#include "lisp/lisp.hh"
#include "g1_rand.hh"
#include "objs/explosion1.hh"
#include "time/profile.hh"

static i4_profile_class pf_chunk_explosion("chunk_explosion");

g1_chunk_explosion_class::g1_chunk_explosion_class(g1_object_type id, g1_loader_class *fp)
  : g1_object_class(id, fp) {}

void g1_chunk_explosion_class::setup(const i4_3d_vector &pos,
                                     const i4_3d_vector &rotations,
                                     g1_quad_object_class *model,
                                     const i4_3d_vector &_dir,
                                     int ticks_till_explode)

{
  dir=_dir;
  dir.x=dir.x*0.1 + g1_float_rand(5)*0.04-0.08;
  dir.y=dir.y*0.1 + g1_float_rand(7)*0.04-0.08;
  dir.z=g1_float_rand(6)*0.1+0.1;

  avel.x=g1_float_rand(8)*0.2;
  avel.y=g1_float_rand(1)*0.2;
  avel.z=g1_float_rand(0)*0.4-0.2;
  
  ticks_left=ticks_till_explode;
      
  draw_params.setup(model);
  x=lx=pos.x;
  y=ly=pos.y;
  h=lh=pos.z;
  theta = rotations.z;
  pitch = rotations.y;
  roll = rotations.x;
  occupy_location();
  request_think();
}             
             
void g1_chunk_explosion_class::draw(g1_draw_context_class *context)
{
  g1_model_draw(this, draw_params, context);
}

static li_symbol_ref explode_model("explode_model");

float jc_f=0.0008;
float jc_l=0.2;

static li_symbol_ref explosion1("explosion1");

void g1_chunk_explosion_class::think()
{
  pf_chunk_explosion.start();

  if (!ticks_left)
  {
    g1_explode_model_class *e;
    e=(g1_explode_model_class *)g1_create_object(g1_get_object_type(explode_model.get()));
    g1_explode_params params;
    params.stages[0].setup(1, jc_f, G1_APPLY_SING);
    params.stages[1].setup(20, 0, G1_APPLY_SING);
    params.t_stages=2;
    e->setup(this, i4_3d_vector(0,0,0), params);

    g1_explosion1_class *explosion;
    explosion=(g1_explosion1_class *)g1_create_object(g1_get_object_type(explosion1.get()));
    explosion->setup(x,y,h);
         
    unoccupy_location();
    request_remove();
  }
  else
  {
    i4_3d_vector ray=dir;

    g1_object_class *hit;
    if (g1_get_map()->check_non_player_collision(0xff, i4_3d_vector(x,y,h), ray, hit))
      dir.z=-dir.z*0.75;

    unoccupy_location();
    grab_old();
    x+=ray.x;
    y+=ray.y;
    h+=ray.z;

    pitch+=avel.x;
    roll+=avel.y;
    theta+=avel.z;

    if (occupy_location())
      request_think();
    ticks_left--;
  }

  pf_chunk_explosion.stop();
}


g1_object_definer<g1_chunk_explosion_class> g1_chunk_explosion_def("chunk_explosion");



