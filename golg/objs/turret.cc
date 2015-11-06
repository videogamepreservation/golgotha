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
#include "objs/turret.hh"
#include "input.hh"
#include "math/pi.hh"
#include "math/angle.hh"
#include "math/trig.hh"
#include "g1_rand.hh"
#include "resources.hh"
#include "saver.hh"
#include "map_cell.hh"
#include "map.hh"
#include "map_man.hh"
#include "object_definer.hh"
#include "lisp/lisp.hh"
#include "objs/fire.hh"

enum {DATA_VERSION=1};
  
static g1_model_ref model_ref("aagun_base"),
  turret_ref("aagun_turret"),
  barrels_ref("aagun_barrels");

static g1_object_type bullet;
static i4_3d_vector turret_attach, muzzle_attach;

void g1_turret_init()
{
  bullet = g1_get_object_type("bullet");

  turret_attach.set(0,0,0);
  model_ref()->get_mount_point("Turret", turret_attach);
  muzzle_attach.set(0,0,0.15);
  model_ref()->get_mount_point("Muzzle", muzzle_attach);
}

g1_object_definer<g1_turret_class>
g1_turret_def("turret", g1_object_definition_class::EDITOR_SELECTABLE, g1_turret_init);

void g1_turret_class::setup(i4_float _x, i4_float _y, g1_object_class *creator)
{
  x = x;
  y = y;
  player_num = creator->player_num;
  occupy_location();
  request_think();
  grab_old();
}

g1_turret_class::g1_turret_class(g1_object_type id,
                                 g1_loader_class *fp)
  : g1_map_piece_class(id,fp)
{  
  defaults = g1_turret_def.defaults;
  draw_params.setup(model_ref.id());
  
  allocate_mini_objects(2,"turret mini objects");

  muzzle = &mini_objects[0];
  muzzle->defmodeltype = barrels_ref.id();
  muzzle->position(muzzle_attach);
  muzzle->rotation.set(0,0,0);
  
  top = &mini_objects[1];
  top->defmodeltype = turret_ref.id();
  top->position(turret_attach);
  top->rotation.set(0,0,0);
  top->grab_old();
  
  if (fp && fp->check_version(DATA_VERSION))
  {
    fp->end_version(I4_LF);
  }
  else
  {
    health = defaults->health;
  }

  guard_angle = 0;
  guard_time = 0;

  radar_type=G1_RADAR_BUILDING;
  set_flag(BLOCKING      |
           SELECTABLE    |
           TARGETABLE    |
           GROUND        | 
           HIT_GROUND    |
           HIT_AERIAL    |
           DANGEROUS     |
           SHADOWED, 1);
}

void g1_turret_class::save(g1_saver_class *fp)
{
  g1_map_piece_class::save(fp);
  
  fp->start_version(DATA_VERSION);

  fp->end_version();
}


void g1_turret_class::fire()
{
  fire_delay = defaults->fire_delay;
  
  //recoil
  if (muzzle->offset.x != -0.2)
    muzzle->offset.x = -0.2;

  i4_3d_point_class tpos, bpos, bvel;
  i4_transform_class btrans;
  
  btrans.rotate_x_y_z(muzzle->rotation.x,muzzle->rotation.y,muzzle->rotation.z,i4_T);
  btrans.t = i4_3d_vector(muzzle->x,muzzle->y,muzzle->h);
  btrans.t += i4_3d_vector(x,y,h);

  btrans.transform(i4_3d_point_class(1.0,order?-0.15:0.15,0),bpos);
  btrans.transform_3x3(i4_3d_point_class(g1_resources.bullet_speed,0,0),bvel);
  order = !order;

  g1_fire(defaults->fire_type, this, attack_target.get(), bpos, bvel);
}


void g1_turret_class::think()
{  
  //undo the recoil
  if (muzzle->offset.x < 0)
    muzzle->offset.x += 0.01;
  
  if (muzzle->offset.x > 0)
    muzzle->offset.x = 0;

  find_target();

  if (health < defaults->health)
    health++;

  if (fire_delay>0)
    fire_delay--;

  pitch = 0;//groundpitch*cos(theta) - groundroll *sin(theta);
  roll  = 0;//groundroll *cos(theta) + groundpitch*sin(theta);
  h = terrain_height;

  //aim the turet
  if (attack_target.valid()) 
  {
    request_think();

    i4_3d_point_class d,pos;

    lead_target(d);

    pos.set(x,y,h);
    d -= pos;

    //aim the turet
    
    guard_angle = i4_atan2(d.y,d.x);
    guard_pitch = i4_atan2(-d.z,sqrt(d.x*d.x+d.y*d.y));
    
    i4_normalize_angle(guard_angle);
    i4_normalize_angle(guard_pitch);

    int aimed=i4_F;
    i4_float dangle;

    dangle = i4_rotate_to(muzzle->rotation.z,guard_angle,defaults->turn_speed);
    aimed = (dangle<defaults->turn_speed && dangle>-defaults->turn_speed);
    dangle = i4_rotate_to(muzzle->rotation.y,guard_pitch,defaults->turn_speed);
    aimed &= (dangle<defaults->turn_speed && dangle>-defaults->turn_speed);
    top->rotation.z = muzzle->rotation.z;
    if (aimed)
      if (!fire_delay)
        fire();
    guard_time = 30;
  }
  else
  {
    request_think();            // move this to draw function

    if (guard_time<=0)
    {
      guard_angle = g1_float_rand(4)*i4_2pi();
      guard_pitch = -g1_float_rand(5)*i4_pi()*0.5;
      guard_time = 20;
    }

    int aimed;

    aimed = (i4_rotate_to(muzzle->rotation.z,guard_angle,defaults->turn_speed)==0.0);
    aimed &= (i4_rotate_to(muzzle->rotation.y,guard_pitch,defaults->turn_speed)==0.0);
    top->rotation.z = muzzle->rotation.z;
    if (aimed)
      if (guard_time>0)
        guard_time--;
  }
}
