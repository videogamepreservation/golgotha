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
#include "input.hh"
#include "math/pi.hh"
#include "math/trig.hh"
#include "math/angle.hh"
#include "objs/bullet.hh"
#include "resources.hh"
#include "saver.hh"
#include "map_cell.hh"
#include "map.hh"
#include "map_man.hh"
#include "sound/sfx_id.hh"
#include "objs/eleccar.hh"
#include "math/random.hh"
#include "g1_render.hh"
#include "object_definer.hh"
#include "draw_context.hh"

#include "image_man.hh"
static g1_team_icon_ref radar_im("bitmaps/radar/peon_tank.tga");


static g1_model_ref model_ref("elec_body"),
  shadow_ref("elec_shadow"),
  wheel_ref("elec_wheel"),
  gun_ref("elec_gun"),
  lod_ref("elec_body_lod");

static i4_3d_vector wheel_attach, wheel_offset, gun_l_attach, gun_r_attach, gun_offset;

void g1_electric_car_init()
{
  //defaults until we get the real ones in the model
  wheel_attach.set(0,0.1,0.03);
  wheel_offset.set(0,0.1,0.03);
  gun_r_attach.set(0.0422,0,-0.0494);
  gun_l_attach.set(0.0422,0,0.0494);
  gun_offset.set(0.0422,0,0.0494);

  model_ref()->get_mount_point("Wheel", wheel_attach);
  model_ref()->get_mount_point("GunLeft", gun_l_attach);
  model_ref()->get_mount_point("GunRight", gun_r_attach);

  wheel_ref()->get_mount_point("Wheel", wheel_offset);
  wheel_offset.reverse();
  
  gun_ref()->get_mount_point("Gun", gun_offset);
  gun_offset.reverse();
}

g1_object_definer<g1_electric_car_class>
g1_electric_car_def("electric_car",
                    g1_object_definition_class::TO_MAP_PIECE |
                    g1_object_definition_class::EDITOR_SELECTABLE |
                    g1_object_definition_class::MOVABLE,
                    g1_electric_car_init);


g1_electric_car_class::g1_electric_car_class(g1_object_type id, 
                                             g1_loader_class *fp)
  : g1_map_piece_class(id,fp)
{    
  draw_params.setup(model_ref.id(),shadow_ref.id(), lod_ref.id());  


  allocate_mini_objects(4,"Electric Car Mini-Objects");

  wheel     = &mini_objects[0];
  wheel->defmodeltype     = wheel_ref.id();
  wheel->position(wheel_attach);
  wheel->offset = wheel_offset;

  gun_left  = &mini_objects[1];
  gun_left->defmodeltype  = gun_ref.id();
  gun_left->position(gun_l_attach);
  gun_left->offset = gun_offset;

  gun_right = &mini_objects[2];
  gun_right->defmodeltype = gun_ref.id();
  gun_right->position(gun_r_attach);
  gun_right->offset = gun_offset;

  //read other data from file
  w16 ver,data_size;
  w32 i;
  if (fp)
  {
    fp->get_version(ver,data_size);
    if (ver==DATA_VERSION)
    {
      for (i=0;i<NUM_ARC_POINTS;i++)
      {
        arc_points1[i].position.x  = fp->read_float();
        arc_points1[i].position.y  = fp->read_float();
        arc_points1[i].position.z  = fp->read_float();
        arc_points1[i].lposition.x = fp->read_float();
        arc_points1[i].lposition.y = fp->read_float();
        arc_points1[i].lposition.z = fp->read_float();
        
        arc_points2[i].position.x  = fp->read_float();
        arc_points2[i].position.y  = fp->read_float();
        arc_points2[i].position.z  = fp->read_float();
        arc_points2[i].lposition.x = fp->read_float();
        arc_points2[i].lposition.y = fp->read_float();
        arc_points2[i].lposition.z = fp->read_float();            
      }

      firing = fp->read_8();

      wheel->rotation.x = fp->read_float();
      wheel->rotation.y = fp->read_float();
      wheel->rotation.z = fp->read_float();
      wheel->lrotation.x = fp->read_float();
      wheel->lrotation.y = fp->read_float();
      wheel->lrotation.z = fp->read_float();

      gun_right->rotation.x = fp->read_float();
      gun_right->rotation.y = fp->read_float();
      gun_right->rotation.z = fp->read_float();
      gun_right->lrotation.x = fp->read_float();
      gun_right->lrotation.y = fp->read_float();
      gun_right->lrotation.z = fp->read_float();

      gun_left->rotation.x = fp->read_float();
      gun_left->rotation.y = fp->read_float();
      gun_left->rotation.z = fp->read_float();
      gun_left->lrotation.x = fp->read_float();
      gun_left->lrotation.y = fp->read_float();
      gun_left->lrotation.z = fp->read_float();
    }
    else
    if (ver==1)
    {      
      wheel->rotation.x = fp->read_float();
      wheel->rotation.y = fp->read_float();
      wheel->rotation.z = fp->read_float();
      wheel->lrotation.x = fp->read_float();
      wheel->lrotation.y = fp->read_float();
      wheel->lrotation.z = fp->read_float();

      gun_right->rotation.x = fp->read_float();
      gun_right->rotation.y = fp->read_float();
      gun_right->rotation.z = fp->read_float();
      gun_right->lrotation.x = fp->read_float();
      gun_right->lrotation.y = fp->read_float();
      gun_right->lrotation.z = fp->read_float();

      gun_left->rotation.x = fp->read_float();
      gun_left->rotation.y = fp->read_float();
      gun_left->rotation.z = fp->read_float();
      gun_left->lrotation.x = fp->read_float();
      gun_left->lrotation.y = fp->read_float();
      gun_left->lrotation.z = fp->read_float();
      
      firing = i4_F;
      memset(arc_points1,0,sizeof(arc_point) * NUM_ARC_POINTS);
      memset(arc_points2,0,sizeof(arc_point) * NUM_ARC_POINTS);
    }
    else
    {
      fp->seek(fp->tell() + data_size);
      strategy = g1_electric_car_class::SIT;
    
      wheel->rotation = i4_3d_vector(0,0,0);
      wheel->lrotation = wheel->rotation;
    
      gun_left->rotation = i4_3d_vector(0,0,0);
      gun_left->lrotation = gun_left->rotation;
    
      gun_right->rotation = i4_3d_vector(0,0,0);
      gun_right->lrotation = gun_right->rotation;
    
      firing = i4_F;
      memset(arc_points1,0,sizeof(arc_point) * NUM_ARC_POINTS);
      memset(arc_points2,0,sizeof(arc_point) * NUM_ARC_POINTS);
    
    }

    fp->end_version(I4_LF);
  }
  else
  {    
    strategy = g1_electric_car_class::SIT;
    
    wheel->rotation = i4_3d_vector(0,0,0);
    wheel->lrotation = wheel->rotation;
    
    gun_left->rotation = i4_3d_vector(0,0,0);
    gun_left->lrotation = gun_left->rotation;
    
    gun_right->rotation = i4_3d_vector(0,0,0);
    gun_right->lrotation = gun_right->rotation;
    
    firing = i4_F;
    memset(arc_points1,0,sizeof(arc_point) * NUM_ARC_POINTS);
    memset(arc_points2,0,sizeof(arc_point) * NUM_ARC_POINTS);
    
  } 
  
  init_rumble_sound(G1_RUMBLE_GROUND);
  //  attack_sound = g1_sound_man.alloc_dynamic_3d_sound(g1_sfx_misc_electric_car_charged); sfxfix

  radar_image=&radar_im;
  radar_type=G1_RADAR_VEHICLE;  
  set_flag(BLOCKING      |
           TARGETABLE    |
           GROUND        | 
           HIT_GROUND    |
           SHADOWED      |
           DANGEROUS,   1);

}
   
g1_electric_car_class::~g1_electric_car_class()
{
  //make sure the sound gets deallocated
//   if (attack_sound)  sfxfix
//   {
//     g1_sound_man.free_dynamic_3d_sound(attack_sound);
//     attack_sound = 0;
//   }    
}


void g1_electric_car_class::save(g1_saver_class *fp)
{
  g1_map_piece_class::save(fp);

  fp->start_version(DATA_VERSION);

  w32 i;
  for (i=0;i<NUM_ARC_POINTS;i++)
  {
    fp->write_float(arc_points1[i].position.x);
    fp->write_float(arc_points1[i].position.y);
    fp->write_float(arc_points1[i].position.z);
    fp->write_float(arc_points1[i].lposition.x);
    fp->write_float(arc_points1[i].lposition.y);
    fp->write_float(arc_points1[i].lposition.z);
    
    fp->write_float(arc_points2[i].position.x);
    fp->write_float(arc_points2[i].position.y);
    fp->write_float(arc_points2[i].position.z);
    fp->write_float(arc_points2[i].lposition.x);
    fp->write_float(arc_points2[i].lposition.y);
    fp->write_float(arc_points2[i].lposition.z);
  }

  fp->write_8(firing);

  fp->write_float(wheel->rotation.x);
  fp->write_float(wheel->rotation.y);
  fp->write_float(wheel->rotation.z);
  fp->write_float(wheel->lrotation.x);
  fp->write_float(wheel->lrotation.y);
  fp->write_float(wheel->lrotation.z);

  fp->write_float(gun_right->rotation.x);
  fp->write_float(gun_right->rotation.y);
  fp->write_float(gun_right->rotation.z);
  fp->write_float(gun_right->lrotation.x);
  fp->write_float(gun_right->lrotation.y);
  fp->write_float(gun_right->lrotation.z);

  fp->write_float(gun_left->rotation.x);
  fp->write_float(gun_left->rotation.y);
  fp->write_float(gun_left->rotation.z);
  fp->write_float(gun_left->lrotation.x);
  fp->write_float(gun_left->lrotation.y);
  fp->write_float(gun_left->lrotation.z);
  fp->end_version();
}

void g1_electric_car_class::draw(g1_draw_context_class *context)
{  
  g1_map_piece_class::draw(context);

  sw32 i;

  if (firing && health>0)
  {
    i4_3d_point_class pts[g1_electric_car_class::NUM_ARC_POINTS];
    for (i=0;i<g1_electric_car_class::NUM_ARC_POINTS;i++)
      pts[i].interpolate(arc_points1[i].lposition,arc_points1[i].position,g1_render.frame_ratio);

    g1_render.add_translucent_trail(context->transform,pts,
                                    g1_electric_car_class::NUM_ARC_POINTS,
                                    0.02,0.02,1,1,0xFFFFFF,0x0088FF);
  
    for (i=0;i<g1_electric_car_class::NUM_ARC_POINTS;i++)
      pts[i].interpolate(arc_points2[i].lposition,arc_points2[i].position,g1_render.frame_ratio);

    g1_render.add_translucent_trail(context->transform,pts,
                                    g1_electric_car_class::NUM_ARC_POINTS,
                                    0.02,0.02,1,1,0xFFFFFF,0x0088FF);    
  }
}

void g1_electric_car_class::copy_old_points()
{
  sw32 i;
  for (i=0;i<NUM_ARC_POINTS;i++)
  {
    arc_points1[i].lposition = arc_points1[i].position;
    arc_points2[i].lposition = arc_points2[i].position;
  }
}

void g1_electric_car_class::new_arc_points(const i4_3d_vector &laser1, 
                                           const i4_3d_vector &laser2, 
                                           const i4_3d_vector &target)
{
  i4_float rx,ry,rz,px,py,pz;
  i4_float map_point_height;
  
  i4_3d_vector vec1,vec2;

  vec1 = target;
  vec1 -= laser1;

  vec2 = target;
  vec2 -= laser2;

  sw32 i;

  for (i=0; i<NUM_ARC_POINTS; i++)
  {
    i4_float sin_factor = sin(i4_pi()*i/(NUM_ARC_POINTS-1));

    //ARC #1
    rx = (i4_rand()&0xFFFF)/((i4_float)0xffff) * 0.8 - 0.2;
    ry = (i4_rand()&0xFFFF)/((i4_float)0xffff) * 0.8 - 0.2;
    rz = (i4_rand()&0xFFFF)/((i4_float)0xffff) * 0.8 - 0.2;
    
    px = laser1.x + vec1.x*i/(NUM_ARC_POINTS-1) + rx*sin_factor;
    py = laser1.y + vec1.y*i/(NUM_ARC_POINTS-1) + ry*sin_factor;
    pz = laser1.z + vec1.z*i/(NUM_ARC_POINTS-1) + rz*sin_factor;
    
    map_point_height = g1_get_map()->map_height(px,py,pz) + 0.05;
  
    if (pz < map_point_height)
      pz = map_point_height;
  
    arc_points1[i].position = i4_3d_vector(px,py,pz);
    
    //ARC #2
    rx = (i4_rand()&0xFFFF)/((i4_float)0xffff) * 0.8 - 0.2;
    ry = (i4_rand()&0xFFFF)/((i4_float)0xffff) * 0.8 - 0.2;
    rz = (i4_rand()&0xFFFF)/((i4_float)0xffff) * 0.8 - 0.2;
    
    px = laser2.x + vec2.x*i/(NUM_ARC_POINTS-1) + rx*sin_factor;
    py = laser2.y + vec2.y*i/(NUM_ARC_POINTS-1) + ry*sin_factor;
    pz = laser2.z + vec2.z*i/(NUM_ARC_POINTS-1) + rz*sin_factor;
      
    map_point_height = g1_get_map()->map_height(px,py,pz) + 0.05;
  
    if (pz < map_point_height)
      pz = map_point_height;
  
    arc_points2[i].position = i4_3d_vector(px,py,pz);
  }
}

void g1_electric_car_class::fire()
{ 
  i4_3d_vector laser1,laser2;
  i4_float     tmp_x,tmp_y,tmp_z;
  
  i4_transform_class btrans,tmp1;
  
  btrans.identity();

  tmp1.rotate_x(groundroll);
  btrans.multiply(tmp1);

  tmp1.rotate_y(groundpitch);
  btrans.multiply(tmp1);

  tmp1.rotate_z(theta);
  btrans.multiply(tmp1);
      
  i4_3d_point_class tpoint;
 
  tmp_x = 0.18;
  tmp_y = 0.05;
  tmp_z = 0.05;

  btrans.transform(i4_3d_point_class(tmp_x,tmp_y,tmp_z),tpoint);
  
  laser1.x = x + tpoint.x;
  laser1.y = y + tpoint.y;
  laser1.z = h + tpoint.z;
  
  tmp_x = 0.18;
  tmp_y = -0.05;
  tmp_z = 0.05;

  btrans.transform(i4_3d_point_class(tmp_x,tmp_y,tmp_z),tpoint);
  
  laser2.x = x + tpoint.x;
  laser2.y = y + tpoint.y;
  laser2.z = h + tpoint.z;
  
  //laser1 and laser2 are the woldspace points where the arcs should be emitted
//   if (attack_sound)  sfxfix
//   {
//     i4_3d_vector delta_pos = i4_3d_vector(0,0,0);

//     g1_sound_man.update_dynamic_3d_sound(attack_sound,laser1,delta_pos,1);
//   }
  
  i4_3d_vector target_point;

  if (firing)
  {
    //he's been firing
    copy_old_points();

    //aim arc_points.position at the current tick's position of the target
    target_point = i4_3d_vector(attack_target->x,
                                attack_target->y,
                                attack_target->h + attack_target->occupancy_radius()/2);

    new_arc_points(laser1, laser2, target_point);
  }
  else
  {
    //he just started firing
    //aim arc_points.lposition at the last tick's position of the target
    target_point = i4_3d_vector(attack_target->lx,
                                attack_target->ly,
                                attack_target->lh + attack_target->occupancy_radius()/2);

    new_arc_points(laser1, laser2, target_point);

    copy_old_points();

    //aim arc_points.position at the current tick's position of the target
    target_point = i4_3d_vector(attack_target->x,
                                attack_target->y,
                                attack_target->h + attack_target->occupancy_radius()/2);
    

    new_arc_points(laser1, laser2, target_point);

  }


  i4_3d_vector dir(attack_target->x-x, attack_target->y, attack_target->h-h);
  dir.normalize();
  
  g1_apply_damage(this, this, attack_target.get(), dir);
  
  firing = i4_T;
}

void g1_electric_car_class::post_think()
{
  //(OLI) should fix semantics of post_think - should post_think only if requested

  if (!alive())
    return;

  g1_object_class *target=attack_target.get();
  if (target)
  {        
    request_think();

    i4_float dx,dy,angle;      

    dx = (attack_target->x - x);
    dy = (attack_target->y - y);

    //aim the vehicle    
    angle = i4_atan2(dy,dx);

    i4_normalize_angle(angle);

    if (i4_angle_diff(angle,theta)<i4_pi()/4)
    {
      fire();
      gun_right->rotation.x += 0.4;
      gun_left->rotation.x  -= 0.4;
    }
    else
    {
      // turn toward stuff, when still
      if (lx == x && ly==y)
      {
        theta = ltheta;
        i4_rotate_to(theta, angle, defaults->turn_speed);
      }

      gun_right->rotation.x = gun_right->lrotation.x = 0;
      gun_left->rotation.x  = gun_left->lrotation.x  = 0;
      firing = i4_F;
    }
  }
  else
  {
    firing = i4_F;
    gun_right->rotation.x = gun_right->lrotation.x = 0;
    gun_left->rotation.x  = gun_left->lrotation.x  = 0;
  }
  
//   if (!firing && attack_sound && attack_sound->is_playing())  sfxfix
//     attack_sound->stop();
}

void g1_electric_car_class::die()
{
//   if (attack_sound)
//   {
//     g1_sound_man.free_dynamic_3d_sound(attack_sound);
//     attack_sound=0;
//   }
}



i4_bool g1_electric_car_class::can_attack(g1_object_class *who) const
{
  if (g1_map_piece_class::can_attack(who))
  {
    i4_float dx,dy,angle;      

    dx = (who->x - x);
    dy = (who->y - y);
    angle = i4_atan2(dy,dx);
    i4_normalize_angle(angle);

    if (i4_angle_diff(angle,theta)<i4_pi()/8)
      return i4_T;
  }

  return i4_F;
}
