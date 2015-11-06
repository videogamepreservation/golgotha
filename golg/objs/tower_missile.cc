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
#include "resources.hh"
#include "saver.hh"
#include "map_cell.hh"
#include "map.hh"
#include "map_man.hh"
#include "object_definer.hh"
#include "lisp/lisp.hh"
#include "objs/fire.hh"

#include "sound/sound.hh"
#include "objs/map_piece.hh"

#include "image_man.hh"
#include "player.hh"

static g1_team_icon_ref radar_im("bitmaps/radar/tower_missile.tga");

enum {DATA_VERSION=1};
  
static g1_model_ref model_ref("tower_missile_base"),
  launcher_ref("tower_missile_launcher"),
  bottom_ref("tower_missile_bottom");

static i4_3d_vector launcher_attach, launcher_offset, launch;

S1_SFX(tower_secured, "narrative/turret_captured_22khz.wav", S1_STREAMED, 200);
S1_SFX(tower_lost, "narrative/turret_lost_22khz.wav", S1_STREAMED, 200);

void g1_tower_missile_init()
{
  launcher_attach.set(0,0,0);
  model_ref()->get_mount_point("Mount", launcher_attach);

  launcher_offset.set(0,0,0);
  launcher_ref()->get_mount_point("Mount", launcher_offset);
  launcher_offset.reverse();

  launch.set(0,1.0,0);
  launcher_ref()->get_mount_point("Missiles", launch);
}

class g1_tower_missile_class : public g1_map_piece_class
{
protected:
  g1_mini_object        *launcher,*bottom;
  i4_float guard_angle, guard_pitch;
  int guard_time;
  int missiles;
public:
  g1_tower_missile_class(g1_object_type id, g1_loader_class *fp)
  //{{{
    : g1_map_piece_class(id,fp)
  {
    defaults = get_type()->defaults;
    draw_params.setup(model_ref.id());
    
    allocate_mini_objects(2,"tower_missile mini objects");

    launcher = &mini_objects[0];
    launcher->defmodeltype = launcher_ref.id();
    launcher->position(launcher_attach);
    launcher->offset = launcher_offset;
    launcher->grab_old();
  
    bottom = &mini_objects[1];
    bottom->defmodeltype = bottom_ref.id();
    bottom->grab_old();
  
    fire_delay = 0;
    guard_angle = 0;
    guard_pitch = 0;
    guard_time = 0;
    missiles = 6;

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
    if (missiles==0)
      return;

    fire_delay = defaults->fire_delay;

    missiles--;
  
    i4_3d_point_class p, pos, dir;

    i4_transform_class t, main, l2w;
    launcher->calc_transform(1.0, &t);    
    calc_world_transform(1.0, &main);
    l2w.multiply(main, t);

    static i4_float px[] = {     0, -0.05, +0.05, -0.05, +0.05,     0 };
    static i4_float py[] = { -0.03, +0.03, +0.03, -0.03, -0.03, +0.03 };

    p = launch;
    p.x += px[missiles];
    p.y += py[missiles];

    l2w.transform(p,pos);
    l2w.transform_3x3(i4_3d_point_class(0.4,px[missiles],py[missiles]),dir);

    g1_fire(defaults->fire_type, this, attack_target.get(), pos, dir);
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
        missiles = 6;
    }
    
    pitch = 0;//groundpitch*cos(theta) - groundroll *sin(theta);
    roll  = 0;//groundroll *cos(theta) + groundpitch*sin(theta);
    h = terrain_height;
    
    //aim the turet
    if (attack_target.valid()) 
    {
      request_think();
      
      i4_3d_point_class d,pos;

      lead_target(d);

      pos.set(x,y,h+launcher_attach.z);
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
      dangle = i4_rotate_to(launcher->rotation.y,guard_pitch,defaults->turn_speed);
      aimed &= (dangle<defaults->turn_speed && dangle>-defaults->turn_speed);
      if (aimed)
        fire();
      guard_time = 30;
    }
    else
    {
      request_think();            // move this to draw function

      if (guard_time<=0)
      {
        guard_angle = g1_float_rand(1)*i4_2pi();
        guard_pitch = (-g1_float_rand(2)*0.5 + 0.25)*i4_pi();
        i4_normalize_angle(guard_pitch);
        guard_time = 20;
      }

      int aimed;

      aimed = (i4_rotate_to(theta,guard_angle,defaults->turn_speed)==0.0);
      bottom->rotation.z = -theta;
      aimed &= (i4_rotate_to(launcher->rotation.y,guard_pitch,defaults->turn_speed)==0.0);
      if (aimed)
        if (guard_time>0)
          guard_time--;
    }
  }
  //}}}

  virtual i4_bool check_collision(const i4_3d_vector &start, i4_3d_vector &ray)
  //{{{
  {
    i4_3d_vector normal;
    return g1_model_collide_polygonal(this, draw_params, start, ray, normal);
  }
  //}}}

  virtual void damage(g1_object_class *obj, int hp, i4_3d_vector _damage_dir)
  //{{{
  {
    health -= hp;
    if (health<1)
    {
      health=1;
      set_flag(DANGEROUS,0);
    }
    request_think();
  }
  //}}}
};

g1_object_definer<g1_tower_missile_class>
g1_tower_missile_def("tower_missile", g1_object_definition_class::EDITOR_SELECTABLE, 
                     g1_tower_missile_init);

//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}
