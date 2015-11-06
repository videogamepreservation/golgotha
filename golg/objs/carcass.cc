/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "g1_object.hh"
#include "objs/model_draw.hh"
#include "lisp/lisp.hh"
#include "tex_id.hh"
#include "objs/particle_emitter.hh"
#include "object_definer.hh"


static li_symbol_ref part_emit("particle_emitter");
static r1_texture_ref smoke_texture("smoke_particle");

class g1_carcass_class : public g1_object_class
{
public:
  int time_left;

  g1_carcass_class(g1_object_type id, g1_loader_class *fp)
    : g1_object_class(id, fp)
  {
    if (fp)    // remove from game on load
      x=-1;
  }
  
  void draw(g1_draw_context_class *context)
  {
    g1_model_draw(this, draw_params, context);
    flags|=TARGETABLE | GROUND | BLOCKING | CAN_DRIVE_ON;
  }
  
  void setup(g1_object_class *from,
             g1_quad_object_class *model,
             int ticks,
             int ticks_to_smoke,
             g1_quad_object_class *lod_model)
  {
    if (ticks_to_smoke>0)
    {
      g1_particle_emitter_class *smoke=
        (g1_particle_emitter_class *)g1_create_object(g1_get_object_type(part_emit.get())); 
      
      g1_particle_emitter_params params;
      params.defaults();
      params.texture=smoke_texture.get();
      params.start_size=0.05;
      params.grow_speed=0.005;
      params.particle_lifetime=50;
      params.num_create_attempts_per_tick=1;
      params.creation_probability=0.5;
      params.speed=i4_3d_vector(0.001, 0.001, 0.05);
      params.emitter_lifetime = ticks_to_smoke;

      if (smoke) smoke->setup(from->x, from->y, from->h, params);
    }

    time_left = ticks;

    theta=from->theta;
    x=from->x;
    y=from->y;
    h=from->h;
    grab_old();

    draw_params.setup(model, 0, lod_model);
  }


  void think()
  {
    if (time_left>=0)
    {
      time_left--;
      if (time_left<=0)
      {
        unoccupy_location();
        request_remove();
      }
    }
  }
};

g1_object_definer<g1_carcass_class> 
g1_carcass_def("carcass", g1_object_definition_class::EDITOR_SELECTABLE);

g1_object_class *g1_create_carcass(g1_object_class *from,
                                   g1_quad_object_class *model,
                                   int ticks,
                                   int ticks_to_smoke,
                                   g1_quad_object_class *lod_model)
{
  g1_carcass_class *c=(g1_carcass_class *)g1_create_object(g1_carcass_def.type);
  c->setup(from, model, ticks, ticks_to_smoke, lod_model);
  c->occupy_location();
  return c;
}


