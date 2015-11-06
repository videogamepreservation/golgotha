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
#include "objs/popup_turret.hh"
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
  
static g1_model_ref model_ref("popup_housing"),
  mount_ref("popup_mount"),
  barrel_ref("popup_barrel");

static g1_object_type bullet;
static i4_3d_vector turret_attach, mount_offset, barrel_offset;

//tunable variables
//  burst of fire.
//  rates of turning on, tracking
//  angle ranges
//  (weapons?  maybe different objects)
//  entry range to fire

const i4_float POPUP_HEIGHT      = 0.4;
const i4_float POPUP_SPEED       = 0.1;
const i4_float POPDOWN_SPEED     = 0.05;
const i4_float POPUP_ROTATESPEED = 0.3;

void g1_popup_turret_init()
{
  bullet = g1_get_object_type("bullet");

  turret_attach.set(0,0,0);
  model_ref()->get_mount_point("Mount Point", turret_attach);

  mount_offset.set(0,0,0.15);
  mount_ref()->get_mount_point("Mount Point", mount_offset);
  mount_offset.reverse();
  barrel_offset.set(0,0,0.15);
  barrel_ref()->get_mount_point("Mount Point", barrel_offset);
  barrel_offset.reverse();
}

g1_object_definer<g1_popup_turret_class>
g1_popup_turret_def("popup_turret", g1_object_definition_class::EDITOR_SELECTABLE, 
                    g1_popup_turret_init);

void g1_popup_turret_class::setup(i4_float _x, i4_float _y, g1_object_class *creator)
{
  x = x;
  y = y;
  player_num = creator->player_num;
  occupy_location();
  request_think();
  grab_old();
}

g1_popup_turret_class::g1_popup_turret_class(g1_object_type id,
                                 g1_loader_class *fp)
  : g1_map_piece_class(id,fp)
{  

  radar_type=G1_RADAR_BUILDING;
  set_flag(BLOCKING      |
           SELECTABLE    |
           TARGETABLE    |
           GROUND        | 
           HIT_GROUND    |
           HIT_AERIAL    |
           DANGEROUS     |
           SHADOWED, 1);

  defaults = g1_popup_turret_def.defaults;
  draw_params.setup(model_ref.id());
  
  allocate_mini_objects(2,"popup_turret mini objects");

  mount = &mini_objects[0];
  mount->defmodeltype = mount_ref.id();
  mount->position(turret_attach);
  mount->rotation.set(0,0,0);
  mount->offset = mount_offset;
  
  barrel = &mini_objects[1];
  barrel->defmodeltype = barrel_ref.id();
  barrel->position(turret_attach);
  barrel->rotation.set(0,i4_pi()/2,0);
  barrel->offset = barrel_offset;
  
  if (fp && fp->check_version(DATA_VERSION))
  {
    fp->read_format("f", 
                    &mount->rotation.y);
    barrel->rotation.y = mount->rotation.y;
    fp->end_version(I4_LF);
  }
  else
  {
    health = defaults->health;
  }

  mount->grab_old();
  barrel->grab_old();
}

void g1_popup_turret_class::save(g1_saver_class *fp)
{
  g1_map_piece_class::save(fp);
  
  fp->start_version(DATA_VERSION);

  fp->write_format("f", 
                   &mount->rotation.y);

  fp->end_version();
}


void g1_popup_turret_class::fire()
{
  fire_delay = defaults->fire_delay;
  
  i4_3d_point_class tpos, bpos, bvel;
  
  i4_transform_class t, main, l2w;
  barrel->calc_transform(1.0, &t);    
  calc_world_transform(1.0, &main);
  l2w.multiply(main, t);
  
  main.transform(turret_attach, bpos);
  l2w.transform_3x3(i4_3d_point_class(1.0, 0, 0), bvel);

  // spin the barrel
  barrel->rotation.x += 0.7;
  i4_normalize_angle(barrel->rotation.x);

  g1_fire(defaults->fire_type, this, attack_target.get(), bpos, bvel);
}


void g1_popup_turret_class::think()
{  
  find_target();

  if (health < defaults->health)
    health++;

  if (fire_delay>0)
    fire_delay--;

  //aim the turet
  if (attack_target.valid()) 
  {
    request_think();

    if (h < terrain_height+POPUP_HEIGHT-0.001)
    {
      h += POPUP_SPEED;
      if (h>terrain_height+POPUP_HEIGHT)
        h = terrain_height+POPUP_HEIGHT;
    }
    else
    {
      i4_3d_point_class d,pos;

      lead_target(d);

      pos.set(x,y,h-POPUP_HEIGHT);
      d -= pos;

      //aim the turet

      i4_float fire_angle,fire_pitch;
    
      fire_angle = i4_atan2(d.y,d.x);
      fire_pitch = i4_atan2(-d.z,sqrt(d.x*d.x+d.y*d.y));
    
      i4_normalize_angle(fire_angle);
      i4_normalize_angle(fire_pitch);

      int aimed=i4_F;
      i4_float dangle;

      dangle = i4_rotate_to(theta,fire_angle,defaults->turn_speed);
      aimed = (dangle<defaults->turn_speed && dangle>-defaults->turn_speed);
      dangle = i4_rotate_to(mount->rotation.y,fire_pitch,POPUP_ROTATESPEED);
      aimed &= (dangle<POPUP_ROTATESPEED && dangle>-POPUP_ROTATESPEED);
      barrel->rotation.y = mount->rotation.y;
      if (aimed)
        if (!fire_delay)
          fire();
    }
  }
  else
  {
    request_think();            // move this to draw function

    int aimed = i4_rotate_to(mount->rotation.y, i4_pi()/2, POPUP_ROTATESPEED)==0.0;
    barrel->rotation.y = mount->rotation.y;

    if (aimed && h>terrain_height)
    {
      h -= POPDOWN_SPEED;
      if (h<terrain_height)
        h = terrain_height;
    }
  }
}
