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
#include "objs/rocktank.hh"
#include "objs/verybiggun.hh"
#include "input.hh"
#include "math/pi.hh"
#include "math/trig.hh"
#include "math/angle.hh"
#include "objs/bullet.hh"
#include "resources.hh"
#include "saver.hh"
#include "map_cell.hh"
#include "map.hh"
#include "cell_id.hh"
#include "sfx_id.hh"
#include "object_definer.hh"

static g1_object_type bullet;

void g1_very_big_gun_init()
{
  bullet = g1_get_object_type("bullet");
}

g1_object_definer<g1_very_big_gun_class>
g1_very_big_gun_def("verybiggun", SELECTABLE, g1_very_big_gun_init);

g1_fire_range_type g1_very_big_gun_class::range()
{
  return (g1_fire_range_type)g1_very_big_gun_def.defaults->fire_range;
}

g1_very_big_gun_class::g1_very_big_gun_class(g1_object_type id,
                                             g1_loader_class *fp)
  : g1_map_piece_class(id,fp)
{  
  draw_params.setup("turret");
  defaults = g1_very_big_gun_def.defaults;
  
  w16 ver,data_size;
  if (fp)
  {  
    fp->get_version(ver,data_size);
    if (ver==DATA_VERSION)
    {
    }
    else
    {
      fp->seek(fp->tell() + data_size);
      fire_delay = 15;
      health     = 20;
    }

    fp->end_version(I4_LF);
  }
  else
  {
    fire_delay = 15;
    health     = 20;
  }

  health = 200;

  //no sound for now
  //init_rumble_sound(g1_moving_engineering_vehicle_wav);  
  
  maxspeed   = 0; //doesnt move
  turn_speed = defaults->turn_speed;

  radar_type=G1_RADAR_BUILDING;
  set_flag(BLOCKING      |
           SELECTABLE    |
           TARGETABLE    |
           GROUND        | 
           HIT_GROUND    |
           SHADOWED, 1);

}

void g1_very_big_gun_class::save(g1_saver_class *fp)
{
  g1_map_piece_class::save(fp);

  
  fp->start_version(DATA_VERSION);
    
  fp->end_version();
  
}


void g1_very_big_gun_class::fire()
{
  if (attack_target.valid())
  {    
    i4_float tmp_x,tmp_y,tmp_z;
  
    fire_delay = defaults->fire_delay;

    tmp_x = 0.4;
    tmp_y = 0;    
    tmp_z = 0.2;

    i4_transform_class btrans,tmp1;  

    i4_angle ang = theta;
  
    btrans.translate(0,0,0);

    tmp1.rotate_z(ang);
    btrans.multiply(tmp1);

    tmp1.rotate_y(pitch);
    btrans.multiply(tmp1);

    tmp1.rotate_x(roll);
    btrans.multiply(tmp1);

    i4_3d_point_class tpoint;
 
    btrans.transform(i4_3d_point_class(tmp_x,tmp_y,tmp_z),tpoint);
    i4_float bx,by,bz;

    bx = tpoint.x + x;
    by = tpoint.y + y;
    bz = tpoint.z + h;    

    i4_float b_dx,b_dy,b_dz;

    btrans.transform(i4_3d_point_class(g1_resources.bullet_speed,0,0),tpoint);

    b_dx = tpoint.x;
    b_dy = tpoint.y;
    b_dz = tpoint.z;    

    g1_bullet_class *b = (g1_bullet_class *)g1_create_object(bullet);
    if (b)
    {
      b->setup(bx, by, bz, b_dx, b_dy, b_dz, ang,pitch,roll,player_num,
               i4_F,
               defaults->damage,
               defaults->fire_range
               );

      b->set_owner(this);    
    }
  }
}


void g1_very_big_gun_class::think()
{  
  check_life();

  if (find_new_target)
    find_target();

  if (health < 200)
    health++;

  pitch = groundpitch*cos(theta) - groundroll *sin(theta);
  roll  = groundroll *cos(theta) + groundpitch*sin(theta);

  if (fire_delay>0)
    fire_delay--;

  //aim the turet
  if (attack_target.valid()) 
  {
    request_think();

    i4_float dx,dy,angle;

    //this will obviously only be true if attack_target.ref != NULL    
    dx = (attack_target->x - x);
    dy = (attack_target->y - y);

    //aim the turet
    
    angle = i4_atan2(dy,dx);
    
    //snap it
    i4_normalize_angle(angle);    
    
    if (!i4_rotate_to(theta,angle,turn_speed))
      if (!fire_delay)
        fire();
  }  
}
