/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "g1_object.hh"
#include "tile.hh"
#include "math/num_type.hh"
#include "math/pi.hh"
#include "math/trig.hh"
#include "math/angle.hh"
#include "g1_rand.hh"
#include "objs/model_draw.hh"
#include "objs/explosion1.hh"
#include "objs/map_piece.hh"
#include "saver.hh"
#include "map.hh"
#include "map_man.hh"
#include "object_definer.hh"
#include "resources.hh"
#include "player.hh"
#include "objs/particle_emitter.hh"
#include "g1_render.hh"
#include "flare.hh"
#include "objs/guided_missile.hh"
#include "sound/sfx_id.hh"
#include "lisp/lisp.hh"
#include "li_objref.hh"
#include "lisp/li_vect.hh"
#include "objs/smoke_trail.hh"
#include "objs/vehic_sounds.hh"
#include "camera.hh"
#include "time/profile.hh"

static i4_profile_class pf_missile("missile_think");

g1_object_definer<g1_guided_missile_class>
g1_guided_missile_def("guided_missile");

static li_symbol_ref li_smoke_trail("smoke_trail");
static li_symbol_ref li_explosion1("explosion1");
static li_g1_ref_class_member track_object("track_object");
static li_g1_ref_class_member smoke_trail("smoke_trail");
static li_g1_ref_class_member who_fired_me("who_fired_me");
static li_vect_class_member velocity("velocity");
static li_float_class_member fuel("fuel");

static li_symbol_ref light_type("lightbulb");

g1_guided_missile_class::g1_guided_missile_class(g1_object_type id,
                                                 g1_loader_class *fp)
  : g1_object_class(id,fp)
{     
  radar_type=G1_RADAR_WEAPON;

  char lod_name[256];
  draw_params.setup(name());
  set_flag(AERIAL        | 
           HIT_AERIAL    |
           HIT_GROUND    |
           SHADOWED, 
           1);

  damping_fraction = 1.0 - get_type()->defaults->accel/get_type()->defaults->speed;
}

void g1_guided_missile_class::request_remove()
{
  li_class_context context(vars);

  delete_smoke();

  if (light.get())
  {
    light->unoccupy_location();
    light->request_remove();
  }

  
  g1_object_class::request_remove();
}

void g1_guided_missile_class::think()
{    
  pf_missile.start();
  i4_3d_vector accel;
  i4_3d_vector vel=velocity();

  if (fuel() > 0)
  {
    g1_object_class *track = track_object()->value();
    
    if (track)
    {
      i4_3d_vector pos(x,y,h);
      i4_3d_vector lead(track->x, track->y, track->h + 0.05);
      i4_3d_vector mp_diff(track->x - track->lx, track->y - track->ly, track->h - track->lh);
      i4_3d_vector tvel(vel);

      // leading code
      i4_3d_vector diff(lead);
      diff -= pos;

      i4_float t = diff.length()/get_type()->defaults->speed;

      mp_diff*=t;
      lead += mp_diff;

      // base acceleration vector
      accel = lead;
      accel -= pos;
//       accel.z *= 1.5;             // correct height faster

      // determine look ahead for ground avoidance
      int look_ahead = i4_f_to_i(accel.length());
      if (look_ahead>3) look_ahead=3;
      accel.normalize();

#if 0
      // attempt to cancel old velocity
      tvel.normalize();
      tvel *= 0.4;
      accel -= tvel;
      accel.normalize();
#endif

#if 1
      // attempt to cancel perp component of old velocity
      if (vel.dot(accel)>0)
      {
        i4_3d_vector perp(-accel.y, accel.x, 0);

        i4_float perp_vel = vel.dot(perp);

        perp_vel /= get_type()->defaults->accel;
        if (perp_vel<-1.0)
          accel = perp;
        else if (perp_vel>1.0)
        {
          accel = perp;
          accel.reverse();
        }
        else
        {
          accel *= sqrt(1.0 - perp_vel*perp_vel);
          perp *= -perp_vel;
          accel += perp;
        }
      }
#endif

      if (accel.z>-0.5)
      {
        // ground tracking
        i4_3d_vector p(pos);
        i4_float height;
        for (int i=0; i<look_ahead; i++)
        {
          p += accel;
          height = g1_get_map()->terrain_height(p.x,p.y) + 0.02;
          if (height>p.z)
          {
            accel.set(p.x, p.y, height + (height - p.z));
            accel -= pos;
            accel.normalize();
            p.z = height;
          }
        }
      }

      theta = i4_atan2(vel.y,vel.x);
      pitch = i4_atan2(-vel.z, sqrt(vel.y*vel.y+vel.x*vel.x));
    }
    else
    {
      accel.set(cos(theta),
                sin(theta),
                0);
      pitch *= 0.8;
    }
    accel *= get_type()->defaults->accel;
    vel += accel;
    vel *= damping_fraction;
  }
  else
    vel += i4_3d_vector(0,0,-g1_resources.gravity);

  velocity() = vel;
  
  if (move(vel.x,vel.y,vel.z))  
  {
    if (light.get())
      light->move(x,y,h);

    request_think(); 
  }

  pf_missile.stop();
}

void g1_guided_missile_class::setup(const i4_3d_vector &pos,
                                    const i4_3d_vector &dir,
                                    g1_object_class *this_guy_fired_me,
                                    g1_object_class *track_me)
{
  li_class_context context(vars);
  
  w32 i;

  x=lx=pos.x;  y=ly=pos.y;  h=lh=pos.z;
 
  i4_float start_speed=get_type()->defaults->speed*0.3;

  i4_3d_vector vel(dir);
  
  vel.normalize();
  vel *= start_speed;

  vars->set(velocity,new li_vect(vel));

  vars->set(track_object, new li_g1_ref(track_me));
  vars->set(who_fired_me, new li_g1_ref(this_guy_fired_me));
  player_num = this_guy_fired_me->player_num;

  g1_damage_map_struct *dmap=get_type()->get_damage_map();  
  fuel() = dmap->range;

  ltheta=theta = i4_atan2(dir.y, dir.x);   
  pitch=lpitch = i4_atan2(-dir.z, sqrt(dir.x * dir.x + dir.y * dir.y));
  lroll=roll   = 0;
  
  request_think();

  g1_player_info_class *player=g1_player_man.get(player_num);

  if (occupy_location())
  {
    start_sounds();

    float r=g1_resources.visual_radius();
    if (g1_current_view_state()->dist_sqrd(i4_3d_vector(x,y,h))<r*r)
    {
      add_smoke();
      light = (g1_light_object_class *)g1_create_object(g1_get_object_type(light_type.get()));
      light->setup(x,y,h, 1, 0, 0, 1);
      light->occupy_location();
    }
  }
}

void g1_guided_missile_class::start_sounds()
{
}

void g1_guided_missile_class::add_smoke()
{
  int smoke_type=g1_get_object_type(li_smoke_trail.get());
  g1_smoke_trail_class *st=(g1_smoke_trail_class *)g1_create_object(smoke_type);
  
  vars->set(smoke_trail, new li_g1_ref(st));              
  if (st)
  {
    st->setup(x, y, h, 0.02, 0.02, 0xff0000, 0xffffff);   // red to white
    st->occupy_location();
  }
}

void g1_guided_missile_class::update_smoke()
{
  li_class_context c(vars);

  if (!smoke_trail())
    return;

  g1_smoke_trail_class *st=(g1_smoke_trail_class *) smoke_trail()->value();
  if (st)
    st->update_head(x,y,h);
}

void g1_guided_missile_class::delete_smoke()
{
  li_class_context c(vars);

  if (!smoke_trail())
    return;

  g1_smoke_trail_class *st=(g1_smoke_trail_class *) smoke_trail()->value();
  if (st)
  {
    st->unoccupy_location();
    st->request_remove();
    vars->set(smoke_trail, li_g1_null_ref());
  }
}

void g1_guided_missile_class::add_explode()
{
  g1_explosion1_class *explosion;

  int explosion_type=g1_get_object_type(li_explosion1.get()); 
  explosion = (g1_explosion1_class *)g1_create_object(explosion_type);
  if (explosion)
  {
    i4_float rx,ry,rh;
    rx = g1_float_rand(3) * 0.2;
    ry = g1_float_rand(4) * 0.2;
    rh = g1_float_rand(9) * 0.2;
    explosion->setup(x+rx,y+ry,h+rh, g1_explosion1_class::HIT_OBJECT);
  }
}

i4_bool g1_guided_missile_class::move(i4_float x_amount,
                                      i4_float y_amount,
                                      i4_float z_amount)
{
  sw32 res;
  g1_object_class *hit = NULL;

  i4_3d_vector pos(x,y,h),ray(x_amount, y_amount, z_amount);
  res = g1_get_map()->check_non_player_collision(player_num,pos, ray, hit);
  
  if (res)
  {
    g1_apply_damage(this, who_fired_me()->value(), hit, ray);

    if (res != -1)
      add_explode();

    unoccupy_location();
    request_remove();

    return i4_F;
  }

  fuel() -= ray.length();
  pos += ray;
  
  unoccupy_location();
  x = pos.x;
  y = pos.y;
  h = pos.z;

  if (!occupy_location())
    return i4_F;
  else
  {
    update_smoke();
    g1_add_to_sound_average(G1_RUMBLE_MISSILE, pos, ray);
  }

  return i4_T;  
}
