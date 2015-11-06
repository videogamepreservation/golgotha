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
#include "g1_rand.hh"
#include "resources.hh"
#include "saver.hh"
#include "map_cell.hh"
#include "map.hh"
#include "map_man.hh"
#include "sfx_id.hh"
#include "objs/bomber.hh"
#include "objs/fire.hh"
#include "object_definer.hh"
#include "objs/path_object.hh"

#include "image_man.hh"
static g1_team_icon_ref radar_im("bitmaps/radar/plane.tga");

enum { DATA_VERSION=1 };

enum 
{
  TAKE_OFF=0, 
  FLYING, 
  DYING 
};

const i4_float FLY_HEIGHT=1.5;

void g1_bomber_init()
//{{{
{
}
//}}}

g1_object_definer<g1_bomber_class>
g1_bomber_def("bomber",
              g1_object_definition_class::TO_MAP_PIECE |
              g1_object_definition_class::EDITOR_SELECTABLE |
              g1_object_definition_class::MOVABLE,
              g1_bomber_init);


g1_bomber_class::g1_bomber_class(g1_object_type id,
                           g1_loader_class *fp)
  : g1_map_piece_class(id, fp)
//{{{
{  
  draw_params.setup("bomber","bomber_shadow", "bomber_lod");

  damping_fraction = 0.02;

  radar_type=G1_RADAR_VEHICLE;
  radar_image=&radar_im;
  set_flag(BLOCKING      |
           TARGETABLE    |
           AERIAL        | 
           HIT_GROUND    |
           DANGEROUS,1);

  w16 ver,data_size;
  if (fp)
    fp->get_version(ver,data_size);
  else
    ver =0;
  switch (ver)
  {
    case DATA_VERSION:
      fp->read_format("1f4",
                      &mode,
                      &vspeed,
                      &sway);
      break;
    default:
      if (fp) fp->seek(fp->tell() + data_size);
      mode = 0;
      vspeed = 0;
      sway = 0;
      break;
  }
      
  if (fp)
    fp->end_version(I4_LF);
}
//}}}
   
void g1_bomber_class::save(g1_saver_class *fp)
//{{{
{
  g1_map_piece_class::save(fp);

  fp->start_version(DATA_VERSION);
  
  fp->write_format("1f4",
                   &mode,
                   &vspeed,
                   &sway);
  
  fp->end_version();
}
//}}}

void g1_bomber_class::fire()
//{{{
{
  fire_delay = g1_bomber_def.defaults->fire_delay;
  
  i4_3d_vector pos(x,y,h);

  pos.x += g1_float_rand(3)*0.4-0.2;
  pos.y += g1_float_rand(2)*0.4-0.2;

  g1_fire(defaults->fire_type,
          this, attack_target.get(), pos,
          i4_3d_vector(x-lx,y-ly,h-lh));          
}
//}}}

i4_bool g1_bomber_class::move(i4_float x_amount, i4_float y_amount)
//{{{
{
  i4_float 
    newx = x+x_amount, 
    newy = y+y_amount;

  unoccupy_location();
  if (newx>0 && newx<g1_get_map()->width() && newy>0 && newy<g1_get_map()->height())
  {
    x = newx; 
    y = newy;

    if (occupy_location())
      return i4_T;
  }
  request_remove();
  return i4_F;
}
//}}}

void g1_bomber_class::think()
//{{{
{
  if (!check_life())
    return;

  find_target();

  if (fire_delay>0)
    fire_delay--;

  h += vspeed;
       
  switch (mode)
  {
    case TAKE_OFF:
    {
      if (next_path.valid())
      {
        dest_x = next_path->x - path_cos*path_len;
        dest_y = next_path->y - path_sin*path_len;
        dest_z = next_path->h - path_tan_phi*path_len;
      }

      //then raise the bomber
      i4_float dist_to_go = dest_z - h;
      if (dist_to_go>0.05) 
      {
        //if he is more than halfway away, accelerate down
        //otherwise accelerate up        
        if (dist_to_go > (FLY_HEIGHT * 0.5)) 
          vspeed = (vspeed<0.05) ? vspeed+0.005 : vspeed;
      } 
      else 
      {
        //lock these in case there were any small errors
        h = dest_z;
        vspeed = 0;
        
        //think one more time so the l* variables catch up
        //(otherwise he'll bob up and down)
        if (h == lh)
          mode = FLYING;
      }
    } break;

    case FLYING:
    {
      i4_3d_vector d;
      i4_float angle,t;

      // sway over terrain
      sway++;

      if (attack_target.valid())
        if (fire_delay==0)
          fire();

      i4_float roll_to  = 0.02*sin(i4_pi()*sway/17.0);
      i4_float pitch_to = 0.02*sin(i4_pi()*sway/13.0);
      
      if (next_path.valid())
      {
        dest_x = next_path->x;
        dest_y = next_path->y;
        dest_z = next_path->h;
      }

      i4_float dist, dtheta;
      suggest_air_move(dist, dtheta, d);
      move(d.x,d.y);
      h += d.z + 0.02*sin(i4_pi()*sway/15.0);
      
      if (dist<speed)
        advance_path();
      
      i4_normalize_angle(roll_to);
      
      if (speed>0.001 || dtheta!=0.0)
      {
        roll_to = -i4_pi()/4 * dtheta;
        pitch_to = -i4_pi()/8 * speed;
      }

      i4_rotate_to(roll,roll_to,defaults->turn_speed/4);  
      i4_rotate_to(pitch,pitch_to,defaults->turn_speed/4);  
      
      groundpitch = 0; //no ground in the air (duh)
      groundroll  = 0;
    } break;

    case DYING:
    {
      i4_float &roll_speed  = dest_x;
      i4_float &pitch_speed = dest_y;
      
      pitch_speed += 0.004;
      pitch += pitch_speed;

      roll_speed += 0.01;
      roll += roll_speed;

      vspeed -= (g1_resources.gravity * 0.1);

      i4_float dx,dy;
      dx = speed*cos(theta);
      dy = speed*sin(theta);
      move(dx,dy);

      if (h<=terrain_height)
        g1_map_piece_class::damage(0,health,i4_3d_vector(0,0,1));               // die somehow!!!
    } break;
  }

  // have to keep thinking to sway
  request_think();  
}
//}}}

void g1_bomber_class::damage(g1_object_class *obj, int hp, i4_3d_vector _damage_dir)
//{{{
{
  //we dont want to explode if ppl shoot us while we're dying.. we want to
  //smash into the ground and create a nice explosion
  if (mode != DYING)
  {
    if (health<20)
    {
      i4_float &roll_speed  = dest_x;
      i4_float &pitch_speed = dest_y;
      
      roll_speed  = 0;
      pitch_speed = 0;
      health = 20;
      set_flag(DANGEROUS,0);
      mode = DYING;
    }
    g1_map_piece_class::damage(obj,hp,_damage_dir);  
  }
}
//}}}

//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}
