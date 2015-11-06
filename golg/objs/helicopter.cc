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
#include "objs/vehic_sounds.hh"
#include "sound/sfx_id.hh"
#include "objs/helicopter.hh"
#include "objs/fire.hh"
#include "object_definer.hh"
#include "objs/path_object.hh"
#include "camera.hh"

#include "image_man.hh"
static g1_team_icon_ref radar_im("bitmaps/radar/helicopter.tga");

static g1_model_ref model_ref("heli_body"),
  shadow_ref("heli_body_shadow"),
  blades_ref("heli_blades-alpha");

static i4_3d_vector blades_attach, blades_offset;

enum {DATA_VERSION=2};
enum helicopter_mode
{
  TAKE_OFF=0, 
  TAKE_OFF2, 
  FLYING, 
  DYING
};
  
const i4_float fly_height = 1.5;
const i4_float max_bladespeed = 0.75;

static g1_object_type missile;
void g1_helicopter_init()
{
  missile = g1_get_object_type("guided_missile");

  blades_attach.set(0,0,0);
  model_ref()->get_mount_point("Blades", blades_attach);
  blades_offset.set(0,0,0);
  blades_ref()->get_mount_point("Blades", blades_offset);
}

g1_object_definer<g1_helicopter_class>
g1_helicopter_def("helicopter",
                  g1_object_definition_class::TO_MAP_PIECE |
                  g1_object_definition_class::EDITOR_SELECTABLE |
                  g1_object_definition_class::MOVABLE,
                  g1_helicopter_init);

g1_helicopter_class::g1_helicopter_class(g1_object_type id,
                                         g1_loader_class *fp)
  : g1_map_piece_class(id,fp)
{  
  radar_image=&radar_im;
  radar_type=G1_RADAR_VEHICLE;

  set_flag(BLOCKING      |
           TARGETABLE    |
           AERIAL        | 
           HIT_AERIAL    |
           HIT_GROUND    |
           DANGEROUS,
           1);

  draw_params.setup("heli_body","heli_body_shadow");  

  //allocate 1 mini object
  allocate_mini_objects(1,"Helicopter Mini-Objects");  
  blades = &mini_objects[0];
  
  //setup blades
  blades->defmodeltype = blades_ref.id();
  blades->position(blades_attach);
  blades->offset = blades_offset;
  bladespeed = 0;

  w16 ver,data_size;
  if (fp)
    fp->get_version(ver,data_size);
  else
    ver =0;

  switch (ver)
  {
    case DATA_VERSION:
      mode = 0;
      fp->read_format("1ffffff",
                      &mode,

                      &blades->rotation.x,&blades->rotation.x,&blades->rotation.x,
                      &bladespeed,
                      &vspeed,
                      &upaccel);
      grab_old();
      break;
    case 1:
      mode = 0;
      if (fp->read_8()) mode=TAKE_OFF;  // take_off
      if (fp->read_8()) mode=TAKE_OFF;  // taking_off
      if (fp->read_8()) mode=FLYING;    // flying
      fp->read_8();
      fp->read_8();

      blades->rotation.x = fp->read_float();
      blades->rotation.y = fp->read_float();
      blades->rotation.z = fp->read_float();
      fp->read_float();
      fp->read_float();
      fp->read_float();

      bladespeed = fp->read_float();

      vspeed  = fp->read_float();
      upaccel = fp->read_float();
      break;
    default:
      if (fp) fp->seek(fp->tell() + data_size);
      mode = TAKE_OFF;
      blades->rotation.x = blades->lrotation.x = 0;
      blades->rotation.y = blades->lrotation.y = 0;
      blades->rotation.z = blades->lrotation.z = 0;
      bladespeed = 0;
      vspeed = upaccel = 0;     
      break;
  }
  
  if (fp)
    fp->end_version(I4_LF);

  fire_delay = 0;

  blades->defmodeltype = g1_model_list_man.find_handle("heli_blades-alpha");
  blades->lod_model = blades->defmodeltype;

  draw_params.setup("heli_body","heli_body_shadow", "heli_body_lod");  


  init_rumble_sound(G1_RUMBLE_HELI);

  damping_fraction = 0.02;

  bladespeed=max_bladespeed;
}
   
static char *chunk_list[3]={"chunk_chopper_blade","chunk_chopper_body", "chunk_chopper_tail"};

int g1_helicopter_class::get_chunk_names(char **&list) { list=chunk_list; return 3; }


void g1_helicopter_class::save(g1_saver_class *fp)
{
  g1_map_piece_class::save(fp);

  fp->start_version(DATA_VERSION);
  
  fp->write_format("1ffffff",
                   &mode,
                   &blades->rotation.x,&blades->rotation.x,&blades->rotation.x,
                   &bladespeed,
                   &vspeed,
                   &upaccel);

  fp->end_version();
}

void g1_helicopter_class::fire()
{
  g1_object_class *target=attack_target.get();

  if (target)
  {    
    i4_transform_class btrans,tmp1;
    i4_3d_vector pos1, pos2, dir;
  
    btrans.translate(x,y,h);

    tmp1.rotate_x(groundroll);
    btrans.multiply(tmp1);

    tmp1.rotate_y(groundpitch);
    btrans.multiply(tmp1);

    tmp1.rotate_z(theta);
    btrans.multiply(tmp1);

    tmp1.rotate_y(pitch);
    btrans.multiply(tmp1);

    tmp1.rotate_x(roll);
    btrans.multiply(tmp1);

    btrans.transform(i4_3d_vector(0.2, -0.11, -0.02), pos1);
    btrans.transform(i4_3d_vector(0.2,  0.11, -0.02), pos2);
    btrans.transform(i4_3d_vector(0.4, -0.11, -0.02), dir);
    dir-=pos1;
    

    g1_fire(defaults->fire_type,  this, attack_target.get(), pos1, dir);
    g1_fire(defaults->fire_type,  this, attack_target.get(), pos2, dir);

    fire_delay = defaults->fire_delay;
  }
}

i4_bool g1_helicopter_class::move(i4_float x_amount, i4_float y_amount)
{
  
  unoccupy_location();
  x+=x_amount;
  y+=y_amount;

  g1_add_to_sound_average(rumble_type, i4_3d_vector(x,y,h));
  if (occupy_location())
    return i4_T;

  return i4_F;
}


void g1_helicopter_class::think()
{  
  if (!check_life(i4_F))      
    mode = DYING;


  //physics
  blades->rotation.z -= bladespeed;
  h                  += vspeed;

  if (fire_delay>0) fire_delay--;


  switch (mode)
  {
    case TAKE_OFF:
    {
      g1_camera_event cev;
      cev.type=G1_WATCH_EVENT;
      cev.follow_object=this;
      g1_current_view_state()->suggest_camera_event(cev);
      mode=TAKE_OFF2;
    } break;
    
    case TAKE_OFF2:
    {
      if (next_path.valid())
      {
        dest_x = next_path->x - path_cos*path_len;
        dest_y = next_path->y - path_sin*path_len;
        dest_z = next_path->h - path_tan_phi*path_len;
      }

      //then raise the helicopter
      i4_float dist_to_go = dest_z - h;
      if (dist_to_go>0.05) 
      {
        //if he is more than halfway away, accelerate down
        //otherwise accelerate up        
        if (dist_to_go > (fly_height * 0.5)) 
          vspeed = (vspeed<0.05) ? vspeed+0.005 : vspeed;
      } 
      else 
      {
        //lock these there were any small errors
        h = dest_z;
        vspeed = 0;
        upaccel = 0;      
        
        //think one more time so the l* variables catch up
        //(otherwise he'll bob up and down)
        if (h == lh)
          mode = FLYING;
      }
    } 
    
    case FLYING:
    {
      find_target();

      if (next_path.valid())
      {
        dest_x = next_path->x;
        dest_y = next_path->y;
        dest_z = next_path->h;
      }
      
      i4_float dist, dtheta;
      i4_3d_vector d;
      suggest_air_move(dist, dtheta, d);
      
      move(d.x,d.y);
      h += d.z;

      if (dist<speed)
        advance_path();
      
      i4_float roll_to = -i4_pi()/4 * dtheta;
      
      i4_normalize_angle(roll_to);
      
      if (roll_to)
        i4_rotate_to(roll,roll_to,defaults->turn_speed/4);  
      else
        i4_rotate_to(roll,0,defaults->turn_speed/2);
      
      if (attack_target.valid() && !fire_delay)
        fire();
    
      groundpitch = 0; //no ground when in the air (duh)
      groundroll  = 0;
    } break;

    case DYING:
    {
      i4_float &roll_speed  = dest_x;
      i4_float &theta_speed = dest_y;
      
      theta_speed += 0.02;
      theta += theta_speed;

      roll_speed += 0.02;
      roll += roll_speed;

      vspeed -= (g1_resources.gravity * 0.15);

      if (h<=terrain_height)
        g1_map_piece_class::damage(0,health,i4_3d_vector(0,0,1));               // die somehow!!!
      
    } break;
  }

  request_think();  
}

void g1_helicopter_class::damage(g1_object_class *obj, int hp, i4_3d_vector _damage_dir)
{
  //we dont want to explode if ppl shoot us while we're dying.. we want to
  //smash into the ground and create a nice explosion
  if (mode != DYING)
  {
    g1_map_piece_class::damage(obj,hp,_damage_dir);  
    if (health<20)
    {
      i4_float &roll_speed  = dest_x;
      i4_float &theta_speed = dest_y;
      
      roll_speed  = 0;
      theta_speed = 0;
      health      = 20;
      set_flag(DANGEROUS,0);
      mode = DYING;
    }
  }
}
