/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "sound_man.hh"
#include "objs/model_id.hh"
#include "objs/model_draw.hh"
#include "objs/supergun.hh"
#include "math/pi.hh"
#include "math/angle.hh"
#include "math/trig.hh"
#include "g1_rand.hh"
#include "resources.hh"
#include "saver.hh"
#include "map_cell.hh"
#include "map.hh"
#include "map_man.hh"
#include "sfx_id.hh"
#include "object_definer.hh"
#include "objs/target.hh"
#include "lisp/lisp.hh"
#include "objs/fire.hh"

enum {DATA_VERSION=1};
enum
{
  ACQUIRE,
  TARGET,
  FIRING,
};
  
static li_symbol_ref super_mortar("super_mortar");

g1_object_definer<g1_supergun_class>
g1_supergun_def("supergun", g1_object_definition_class::EDITOR_SELECTABLE);




void g1_supergun_class::setup(i4_float _x, i4_float _y, g1_object_class *creator)
{
  x = x;
  y = y;
  player_num = creator->player_num;
  occupy_location();
  request_think();
  grab_old();
}

g1_supergun_class::g1_supergun_class(g1_object_type id,
                                 g1_loader_class *fp)
  : g1_map_piece_class(id,fp)
{  
  defaults = g1_supergun_def.defaults;
  draw_params.setup("supergun_base");
  
  allocate_mini_objects(2,"supergun_barrel");
  muzzle = &mini_objects[0];
  muzzle->x = muzzle->lx = g1_resources.supergun_muzzle_attach.x;
  muzzle->y = muzzle->ly = g1_resources.supergun_muzzle_attach.y;
  muzzle->h = muzzle->lh = g1_resources.supergun_muzzle_attach.z;
  muzzle->rotation.set(0,0,0);
  muzzle->defmodeltype = g1_model_list_man.find_handle("supergun_barrel");
  muzzle->grab_old();
  
  w16 ver,data_size;
  if (fp)
    fp->get_version(ver,data_size);
  else
    ver=0;

  switch (ver)
  {
    case DATA_VERSION:
      break;
    default:
      if (fp) fp->seek(fp->tell() + data_size);
      health = defaults->health;
      break;
  }

  fire_delay = 0;
  time = 0;
  mode = ACQUIRE;

  radar_type=G1_RADAR_BUILDING;
  set_flag(BLOCKING      |
           SELECTABLE    |
           GROUND        | 
           HIT_GROUND    |
           DANGEROUS     |
           SHADOWED, 1);
}

void g1_supergun_class::save(g1_saver_class *fp)
{
  g1_map_piece_class::save(fp);
  
  fp->start_version(DATA_VERSION);

  fp->end_version();
}

i4_bool g1_supergun_class::suggest_new_attack_target(g1_object_class *new_target, 
                                                     g1_fire_range_type range)
{
  attack_target = new_target;
  mode = ACQUIRE;
  return i4_T;
}


void g1_supergun_class::fire()
{
  fire_delay = defaults->fire_delay;
  muzzle->offset.x = -0.2;
  
  i4_3d_point_class bpos, bdir;
  i4_transform_class btrans, tmp1, tmp2;
  
  tmp1.rotate_y(target_pitch);
  tmp1.t.set(muzzle->x,muzzle->y,muzzle->h);

  tmp2.rotate_z(target_angle);
  tmp2.t.set(x,y,h);

  btrans.multiply(tmp2,tmp1);

  btrans.transform(i4_3d_point_class(1.5,0,0),bpos);

  bdir.set(target_groundspeed*cos(theta), target_groundspeed*sin(theta), target_zspeed);
  g1_fire(super_mortar.get(), this, 0,  bpos, bdir, 0);
}

i4_bool g1_supergun_class::deploy_to(float tx, float ty)
{
  i4_3d_vector d(tx, ty, g1_get_map()->terrain_height(tx,ty));

  d -= i4_3d_vector(x,y,h);

  //aim the turet
  target_angle = i4_atan2(d.y,d.x);
  i4_normalize_angle(target_angle);
      
  target_pitch = -(g1_float_rand(4)*i4_pi()/12.0 + i4_pi()/6);
  i4_normalize_angle(target_pitch);

  //                     --------------------
  //  speed = d          |  -a
  //         ------- * _ | -----------------
  //         cos(th)    |/  2*(d*tan(th) - h)
  
  i4_float dist = sqrt(d.x*d.x + d.y*d.y) - 2.0;
  i4_float den = (dist*tan(-target_pitch) - d.z)*2.0;

  if (den>0 && dist>3.0)
  {
    target_groundspeed = dist*sqrt(g1_resources.gravity / den);
    target_zspeed = target_groundspeed*tan(-target_pitch);
    mode = TARGET;
  }

  return i4_T;
}

void g1_supergun_class::think()
{  
  if (!check_life())  
    return;

  if (health>0 && health < defaults->health)
    health+=200;

  if (fire_delay>0)
    fire_delay--;

  pitch = 0;
  roll  = 0;
  h = terrain_height;

  request_think();
  switch(mode)
  {
    case ACQUIRE:
    {
      break;

      i4_3d_vector d;

#if 1
      d.x = g1_float_rand(5)*g1_get_map()->width();
      d.y = g1_float_rand(4)*g1_get_map()->height();
#else
      d.x = g1_get_map()->width()/2.0;
      d.y = g1_get_map()->height()/2.0;
#endif
      if (attack_target.valid())
      {
        d.x = attack_target->x;
        d.y = attack_target->y;
      }

      deploy_to(d.x,d.y);
    } break;

    case TARGET:
    {
      i4_bool aimed;

      aimed = (i4_rotate_to(theta,target_angle,defaults->turn_speed)==0.0);
      aimed = (i4_rotate_to(muzzle->rotation.y,target_pitch,defaults->turn_speed)==0.0) && aimed;
      if (aimed)
      {
        mode = FIRING;
        fire();
      }
    } break;

    case FIRING:
    {
      if (muzzle->offset.x<0.0)
      {
        muzzle->offset.x += 0.01;
        if (muzzle->offset.x>0.0) muzzle->offset.x=0.0;
      }
      if (fire_delay==0 && muzzle->offset.x==0.0)
        mode = ACQUIRE;
    } break;
  }
}
