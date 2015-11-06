/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "objs/bullet.hh"
#include "input.hh"
#include "map.hh"
#include "math/pi.hh"
#include "math/trig.hh"
#include "math/random.hh"
#include "resources.hh"

#include "objs/model_draw.hh"

#include "objs/explosion1.hh"
#include "saver.hh"
#include "tile.hh"

#include "sfx_id.hh"
#include "sound_man.hh"
#include "objs/map_piece.hh"
#include "objs/explosion1.hh"
#include "objs/flak.hh"
#include "objs/shrapnel.hh"
#include "object_definer.hh"
#include "math/pi.hh"
#include "g1_texture_id.hh"
#include "r1_api.hh"
#include "g1_tint.hh"
#include "map_man.hh"
#include "flare.hh"
#include "objs/light_o.hh"
#include "objs/particle_emitter.hh"
#include "lisp/li_class.hh"
#include "li_objref.hh"
#include "camera.hh"
#include "time/profile.hh"

static i4_profile_class pf_bullet("bullet_think");

static li_symbol_ref explosion_type("explosion1"),
  flak_type("flak"),
  shrapnel_type("shrapnel"),
  light_type("lightbulb"),
  particle_emitter_type("particle_emitter"),
  damager_type("damager");

g1_object_definer<g1_bullet_class> g1_b120mm_def("b120mm");
g1_object_definer<g1_bullet_class> g1_b90mm_def("b90mm");
g1_object_definer<g1_bullet_class> g1_acid_def("acid", g1_object_definition_class::HAS_ALPHA);
g1_object_definer<g1_bullet_class> g1_napalm_def("napalm", g1_object_definition_class::HAS_ALPHA);
g1_object_definer<g1_bullet_class> g1_chain_gun_def("chain_gun", g1_object_definition_class::HAS_ALPHA);

static li_float_class_member start_size("start_size"), 
  grow_speed("grow_speed"), max_speed("max_speed");

static li_object_class_member texture_name("texture_name");
static r1_texture_ref bullet_particle("bullet_particle");


void g1_bullet_class::think()
{
  pf_bullet.start();
  move();
  pf_bullet.stop();
}


void g1_bright_ambient(i4_transform_class *object_to_world, 
                      i4_float &ar, i4_float &ag, i4_float &ab)
{
  ar=ag=ab=1;
}


g1_bullet_class::g1_bullet_class(g1_object_type id,
                                 g1_loader_class *fp)
  : g1_object_class(id,fp)
{
  range=10;

  if (fp && fp->check_version(DATA_VERSION))
  {
    x=-1;
//     fp->read_reference(who_fired_me);    
//     fp->read_format("fffff4",
//                     &vel.x, &vel.y, &vel.z,
//                     &range, &range_decrement,
//                     &bullet_flags);

//     fp->end_version(I4_LF);
  }
  else
  {
    bullet_flags=0;
    vel=i4_3d_vector(0,0,0);
    range=0;
    range_decrement=1;
  } 

  if (fp)
    flags|=THINKING;
}


void g1_bullet_class::move_forward()
{
  grab_old();
  x += vel.x; y += vel.y;  h += vel.z;

  if (bullet_flags & SPINS)
    roll+=0.9;

  range -= range_decrement;

  if (light.get())
    light->move(x,y,h);

  if (particle_emitter.get())
    particle_emitter->move(x,y,h);
}


void g1_bullet_class::setup(const i4_3d_vector &start_pos,
                            const i4_3d_vector &_vel,
                            g1_object_class *this_guy_fired_me,
                            i4_float _range,
                            w32 flags,
                            r1_texture_handle sprite_texture,
                            g1_light_object_class *_light)
                            
{    
  bullet_flags = flags;
  range  = _range;

  x=start_pos.x;
  y=start_pos.y;
  h=start_pos.z;

  lx=start_pos.x;
  ly=start_pos.y;
  lh=start_pos.z;

  vel=_vel;
  range_decrement=vel.length()/range;
  
  ltheta=theta = i4_atan2(vel.y, vel.x);   
  pitch=lpitch = i4_atan2(-vel.z, sqrt(vel.x * vel.x +
                                       vel.y * vel.y));
  lroll=roll   = 0;
  player_num   = this_guy_fired_me->player_num;
  who_fired_me = this_guy_fired_me;

  if (bullet_flags & IS_SPRITE)
    tex=sprite_texture;
  else
    draw_params.setup(name());

  if (!get_flag(MAP_OCCUPIED))
  {
    if (!occupy_location())
      return;
    request_think();
  }

  i4_bool possible_visible;

  float r=g1_resources.visual_radius();
  if (g1_current_view_state()->dist_sqrd(i4_3d_vector(x,y,h))<r*r)
    possible_visible=i4_T;
  else
    possible_visible=i4_F;
  
  if ((bullet_flags & MAKE_SMOKE) && possible_visible)
  {
    int type=g1_get_object_type(particle_emitter_type.get());
    particle_emitter = (g1_particle_emitter_class *)g1_create_object(type);

    if (particle_emitter.get())
    {
      g1_particle_emitter_params p;
      p.defaults();
      p.start_size=0.0001;
      p.grow_speed=0.001;
      p.max_speed=0.01;

      p.texture=bullet_particle.get();
      particle_emitter->setup(x,y,h, p);
    }
  }

  light=_light;

  move();

}


void g1_bullet_class::save(g1_saver_class *fp)
{
  // tell direived classes to save themselves
  g1_object_class::save(fp);

  fp->start_version(DATA_VERSION);

  fp->write_reference(who_fired_me);    
  
  fp->write_format("fffff44",
                   &vel.x, &vel.y, &vel.z,
                   &range, &range_decrement,
                   &bullet_flags);

  fp->end_version();
}


void g1_bullet_class::draw(g1_draw_context_class *context)
{
  if ((bullet_flags & IS_SPRITE)==0)
  {
    g1_get_ambient_function_type old_ambient=g1_render.get_ambient;
    g1_render.get_ambient=g1_bright_ambient;

    g1_render.r_api->set_filter_mode(R1_BILINEAR_FILTERING);
    g1_model_draw(this, draw_params, context);
    g1_render.r_api->set_filter_mode(R1_NO_FILTERING);

    g1_render.get_ambient=old_ambient;
  }
  else
  {
    i4_3d_vector pos(i4_interpolate(lx,x, g1_render.frame_ratio),
                     i4_interpolate(ly,y, g1_render.frame_ratio),
                     i4_interpolate(lh,h, g1_render.frame_ratio)), t_pos;
    context->transform->transform(pos, t_pos);  
    g1_render.render_sprite(t_pos, tex, 0.05, 0.05);
  }
}


void g1_bullet_class::done(i4_bool hit, g1_object_class *hit_object)
{
  unoccupy_location();
  request_remove();

  if (hit_object)
  { 
    g1_shrapnel_class *shrap = 
      (g1_shrapnel_class *)g1_create_object(g1_get_object_type(shrapnel_type.get()));

    if (shrap)
      shrap->setup(x,y,h,5,1);

    g1_apply_damage(this, who_fired_me.get(), hit_object, vel);     
  }
  else if (hit)
  {
    g1_flak_class *flak = (g1_flak_class *)g1_create_object(g1_get_object_type(flak_type.get()));

    if (flak)
      flak->setup(i4_3d_vector(x,y,h), 0.1);
  }

  if (light.get())
  {
    light->unoccupy_location();
    light->request_remove();
  }

  if (particle_emitter.get())
    particle_emitter->stop();
}


i4_bool g1_bullet_class::move()

{
  // we traveled to far already, let's go away
  if (range<0)
  {
    done();
    return i4_F;
  }

  g1_object_class *hit = 0;

  i4_3d_vector pos(x,y,h);
  if (!g1_get_map()->check_non_player_collision(player_num,pos,vel,hit))
  {  
    unoccupy_location();

    move_forward();
    
    occupy_location();
    request_think();
    
    return i4_T;
  }
  else
  {
#ifdef TEST_COLLIDE
    x += vel.x;
    y += vel.y;
    h += vel.z;
    vel.set(0,0,0);
    grab_old();
#else
    if (hit)
    {
      float v_len=vel.length();
      
      if (v_len>0.05)
      {
        i4_3d_vector dir=vel;      // scoot the bullet back so smoke & stuff show up in front
        dir/=v_len;                // of the vehicle
        dir*=0.05;
        vel-=dir;
      }
      else
        vel.set(0,0,0);
      x+=vel.x;
      y+=vel.y;
      h+=vel.z;
    }

    done(i4_T, hit); //we've hit something or the ground
#endif
  }
  
  return i4_F;
}



