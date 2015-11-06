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
#include "math/angle.hh"
#include "math/trig.hh"
#include "g1_rand.hh"
#include "saver.hh"
#include "map_cell.hh"
#include "map.hh"
#include "map_man.hh"
#include "sfx_id.hh"
#include "object_definer.hh"
#include "lisp/lisp.hh"
#include "objs/fire.hh"

#include "sound/sound.hh"
#include "objs/map_piece.hh"
#include "objs/bolt.hh"
#include "sound/sfx_id.hh"
#include "image_man.hh"
#include "objs/light_o.hh"
#include "player.hh"

static g1_team_icon_ref radar_im("bitmaps/radar/tower_electric.tga");

enum {DATA_VERSION=1};

#define SPIN_ACCEL 0.02
#define ATTACK_SPIN 0.8
  
static li_symbol_ref bolt_obj("bolt");

static g1_model_ref model_ref("tower_electric_base"),
  pod_ref("tower_electric_pod"), prod_ref("tower_electric_fingers"),
  bottom_ref("tower_electric_bottom");

static i4_3d_vector pod_attach, pod_offset, prod_offset, electric[4];

S1_SFX(charge_sfx, "fire/electric_tower_charge_up_22khz.wav", S1_STREAMED | S1_3D, 70);
S1_SFX(shutdown_sfx, "fire/electric_tower_power_down_22khz.wav", S1_STREAMED | S1_3D, 70);
S1_SFX(tower_secured, "narrative/turret_captured_22khz.wav", S1_STREAMED, 200);
S1_SFX(tower_lost, "narrative/turret_lost_22khz.wav", S1_STREAMED, 200);

void g1_tower_electric_init()
{
  pod_attach.set(0,0,0);
  model_ref()->get_mount_point("Axle", pod_attach);

  pod_offset.set(0,0,0);
  pod_ref()->get_mount_point("Axle", pod_offset);
  pod_offset.reverse();

  prod_offset.set(0,0,0);
  prod_ref()->get_mount_point("Axle", prod_offset);
  prod_offset.reverse();

  prod_ref()->get_mount_point("Electric1", electric[0]);
  prod_ref()->get_mount_point("Electric2", electric[1]);
  prod_ref()->get_mount_point("Electric3", electric[2]);
  prod_ref()->get_mount_point("Electric4", electric[3]);
}

class g1_tower_electric_class : public g1_map_piece_class
{
protected:
  g1_mini_object        *pod, *prod, *bottom;
  i4_float guard_angle, guard_pitch;
  g1_typed_reference_class<g1_bolt_class> bolt[4];

  i4_float spin_vel;
  int charge;
  int guard_time;
public:
  g1_tower_electric_class(g1_object_type id, g1_loader_class *fp)
  //{{{
    : g1_map_piece_class(id,fp)
  {
    defaults = get_type()->defaults;
    draw_params.setup(model_ref.id());
    
    allocate_mini_objects(3,"tower_electric mini objects");

    pod = &mini_objects[0];
    pod->defmodeltype = pod_ref.id();
    pod->position(pod_attach);
    pod->offset = pod_offset;
    pod->grab_old();
  
    prod = &mini_objects[1];
    prod->defmodeltype = prod_ref.id();
    prod->position(pod_attach);
    prod->offset = prod_offset;
    prod->grab_old();

    bottom = &mini_objects[2];
    bottom->defmodeltype = bottom_ref.id();
    bottom->grab_old();

    fire_delay = 0;
    guard_angle = 0;
    guard_pitch = 0;
    guard_time = 0;
    charge = 20;
    spin_vel = 0;

    grab_old();

    radar_type=G1_RADAR_BUILDING;
    radar_image=&radar_im;

    set_flag(BLOCKING      |
             SELECTABLE    |
             TARGETABLE    |
             GROUND        | 
             HIT_GROUND    |
             HIT_AERIAL    |
             DANGEROUS, 1);
  }
  //}}}

  virtual void save(g1_saver_class *fp)
  //{{{
  {
    g1_map_piece_class::save(fp);
  }
  //}}}

  virtual void fire()
  //{{{
  {
    if (spin_vel<ATTACK_SPIN)
      return;

    i4_3d_point_class pos, spot;
    i4_transform_class t, main, l2w;
    prod->calc_transform(1.0, &t);    
    calc_world_transform(1.0, &main);
    l2w.multiply(main, t);

    pos = electric[3];

    if (charge>0)
    {
      pos.x += 0.5;
      l2w.transform(pos, spot);

      if (!bolt[3].valid())
        bolt[3] = (g1_bolt_class *)g1_create_object(g1_get_object_type(bolt_obj.get()));
      
      if (bolt[3].valid())
      {
        bolt[3]->setup_look(0.8, 0.05, 0.05, 0xff88ff, 0x8800ff, 1.0, 1.0, i4_T);
        bolt[3]->setup(spot, i4_3d_point_class(0,0,0),this,attack_target.get());
      }
    }
    else
      l2w.transform(pos, spot);

    for (int i=0; i<3; i++)
    {
      l2w.transform(electric[i], pos);
      
      if (!bolt[i].valid())
        bolt[i] = (g1_bolt_class *)g1_create_object(g1_get_object_type(bolt_obj.get()));

      if (bolt[i].valid())
      {
        bolt[i]->setup_look(0.2, 0.02, (charge>0)? 0.05:0.02, 0xffffff, 0xff88ff);
        bolt[i]->setup(pos, spot, this,0);
      }
    }

    if (charge>0)
    {
      fire_delay = defaults->fire_delay;
      charge--;
    }
  }
  //}}}

  virtual void change_player_num(int new_player_num)
  //{{{
  {
    if (new_player_num==g1_player_man.local_player)
      tower_secured.play();
      
    if (player_num==g1_player_man.local_player)
      tower_lost.play();

    g1_object_class::change_player_num(new_player_num);
    if (health<defaults->health)
      health += defaults->health;
    set_flag(DANGEROUS,1);
    request_think();
  }
  //}}}

  virtual void think()
  //{{{
  {
    if (health<=1)
      return;

    find_target();
    
    if (fire_delay>0)
    {
      fire_delay--;
      if (fire_delay==0)
        charge = 20;
    }
    
    pitch = 0;//groundpitch*cos(theta) - groundroll *sin(theta);
    roll  = 0;//groundroll *cos(theta) + groundpitch*sin(theta);
    h = terrain_height;

    prod->rotation.x += spin_vel;
  
    //aim the turet
    if (attack_target.valid()) 
    {
      request_think();
      
      i4_3d_point_class d,pos;

      lead_target(d);

      pos.set(x,y,h+pod_attach.z);
      d -= pos;

      //aim the turet
    
      guard_angle = i4_atan2(d.y,d.x);
      guard_pitch = i4_atan2(-d.z,sqrt(d.x*d.x+d.y*d.y));
    
      i4_normalize_angle(guard_angle);
      i4_normalize_angle(guard_pitch);

      int aimed=i4_F;
      i4_float dangle;

      dangle = i4_rotate_to(theta,guard_angle,defaults->turn_speed);
      aimed = (dangle<defaults->turn_speed && dangle>-defaults->turn_speed);
      bottom->rotation.z = -theta;
      dangle = i4_rotate_to(pod->rotation.y,guard_pitch,defaults->turn_speed);
      aimed &= (dangle<defaults->turn_speed && dangle>-defaults->turn_speed);
      prod->rotation.y = pod->rotation.y;
      
      if (aimed)
        fire();

      guard_time = 30;

      if (spin_vel<ATTACK_SPIN)
      {
        if (spin_vel==0)
          charge_sfx.play(x,y,h);

        spin_vel += SPIN_ACCEL;
      }
    }
    else
    {
      request_think();            // move this to draw function

      if (guard_time<=0)
      {
        guard_angle = g1_float_rand(2)*i4_2pi();
        guard_pitch = -g1_float_rand(3)*i4_pi()*0.45;
        i4_normalize_angle(guard_pitch);
        guard_time = 20;
      }

      int aimed;

      aimed = (i4_rotate_to(theta,guard_angle,defaults->turn_speed)==0.0);
      bottom->rotation.z = -theta;
      aimed &= (i4_rotate_to(pod->rotation.y,guard_pitch,defaults->turn_speed)==0.0);
      prod->rotation.y = pod->rotation.y;
      if (aimed)
        if (guard_time>0)
          guard_time--;

      if (spin_vel>0)
        spin_vel -= SPIN_ACCEL;
      else
        spin_vel = 0;
    }
  }
  //}}}

  virtual void damage(g1_object_class *obj, int hp, i4_3d_vector _damage_dir)
  //{{{
  {
    if (health>1)
    {
      health -= hp;
      if (health<=1)
      {
        health=1;
        set_flag(DANGEROUS,0);
        shutdown_sfx.play(x,y,h);
      }
    }

    request_think();
  }
  //}}}

  virtual i4_bool check_collision(const i4_3d_vector &start, i4_3d_vector &ray)
  //{{{
  {
    return g1_model_collide_radial(this, draw_params, start, ray);
  }
  //}}}
};

g1_object_definer<g1_tower_electric_class>
g1_tower_electric_def("tower_electric", g1_object_definition_class::EDITOR_SELECTABLE, 
                     g1_tower_electric_init);

//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}
