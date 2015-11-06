/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include <math.h>
#include "objs/model_id.hh"
#include "objs/model_draw.hh"
#include "lisp/li_class.hh"
#include "math/pi.hh"
#include "math/trig.hh"
#include "math/angle.hh"
#include "g1_rand.hh"
#include "saver.hh"
#include "map_cell.hh"
#include "map.hh"
#include "map_man.hh"
#include "player.hh"
#include "object_definer.hh"
#include "objs/moneycrate.hh"
#include "objs/bank.hh"
#include "objs/bases.hh"
#include "objs/moneyplane.hh"
#include "lisp/li_class.hh"
#include "li_objref.hh"
#include "resources.hh"

#include "image_man.hh"
static g1_team_icon_ref radar_im("bitmaps/radar/moneyplane.tga");

enum
{
  RETURNING=0,
  TOBASE,
  DROPPING,
  LAUNCHING,
  TOBANK,
  GRABBING,
  LIFTING,
  DYING,
};

static li_symbol_ref stank_factory("mainbasepad");
static li_g1_ref_class_member li_crate("crate");
static li_int_class_member li_mode("mode");

const i4_float VSPEED = 0.04;
const i4_float FLY_HEIGHT = 3.0;
const i4_float DROP_HEIGHT = 2.7;

S1_SFX(mo_money, "narrative/money_recieved_22KHz.wav", S1_STREAMED, 200);

static g1_model_ref model_ref("moneyplane"), lod_ref("moneyplane_lod");
static i4_3d_vector crate_attach;

static void g1_moneyplane_init();
g1_object_definer<g1_moneyplane_class>
g1_moneyplane_def("moneyplane",
                  g1_object_definition_class::TO_MAP_PIECE |
                  g1_object_definition_class::EDITOR_SELECTABLE |
                  g1_object_definition_class::MOVABLE, 
                  g1_moneyplane_init);

i4_float damp_distance;

static void g1_moneyplane_init()
{
  crate_attach.set(0,0,0);
  model_ref()->get_mount_point("mountcrate",crate_attach);
  crate_attach -= g1_moneycrate_class::crate_attach();

  damp_distance = g1_moneyplane_def.defaults->speed/-log(0.9);
}

g1_moneyplane_class::g1_moneyplane_class(g1_object_type id,
                           g1_loader_class *fp)
  : g1_map_piece_class(id, fp)
{  
  draw_params.setup(model_ref.id(), 0, lod_ref.id());

  init_rumble_sound(G1_RUMBLE_JET);
  radar_type=G1_RADAR_VEHICLE;  
  radar_image=&radar_im;
  set_flag(BLOCKING      |
           TARGETABLE    |
           AERIAL        | 
           DANGEROUS, 1);

}
   
g1_moneycrate_class *g1_moneyplane_class::crate()
{
  li_class_context c(vars);
  return (g1_moneycrate_class *)(li_crate()->value());
}

void g1_moneyplane_class::set_crate(g1_moneycrate_class *c)
{
  vars->set(li_crate, new li_g1_ref(c));
}

i4_bool g1_moneyplane_class::move(const i4_3d_vector &d)
{
  i4_3d_vector np(x,y,h);

  np += d;

  unoccupy_location();
  x = np.x; 
  y = np.y;
  h = np.z;
  
  if (!occupy_location())
    return i4_F;

  if (crate())
  {
    i4_transform_class t;
    i4_3d_vector p;
    calc_world_transform(1.0, &t);
    t.transform(crate_attach, p);
    crate()->follow(p, i4_3d_vector(roll, pitch, theta));
  }

  return i4_T;
}

void g1_moneyplane_class::damage(g1_object_class *obj, int hp, i4_3d_vector _damage_dir)
{
  li_class_context c(vars);

  if (li_mode() != DYING)
  {
    g1_map_piece_class::damage(obj,hp,_damage_dir);
    if (health<20)
    {
      i4_float &roll_speed  = dest_x;
      i4_float &pitch_speed = dest_y;
      
      health = 20;
      set_flag(DANGEROUS,0);
      roll_speed  = 0;
      pitch_speed = 0;
      li_mode() = DYING;
      if (crate())
      {
        crate()->release();
        set_crate(0);
      }
    }
  }

}

void g1_moneyplane_class::think()
{
  int mode = li_mode();

  check_life(i4_F);
  
  i4_float height = g1_get_map()->terrain_height(x,y);
  switch (li_mode())
  {
    case RETURNING:
    {
      if (h<height + DROP_HEIGHT)
        h += ((height + DROP_HEIGHT + 0.05)-h)*0.1;
      else 
      {
        i4_isl_list<g1_factory_class>::iterator p = g1_factory_list.begin();
      
        while (p!=g1_factory_list.end())
        {
          if (p->player_num==player_num &&
              p->id==g1_get_object_type(stank_factory.get()))
            attack_target = &*p;
          p++;
        }

        if (attack_target.valid())
        {
          dest_x = attack_target->x;
          dest_y = attack_target->y;
          dest_theta = 0;
          mode = TOBASE;
        }
      }
    } break;
    case TOBANK:
    case TOBASE:
    {
      i4_3d_vector d(dest_x - x,dest_y - y,0);
      i4_float
        dist = d.x*d.x+d.y*d.y;

      i4_float dangle;

      if (mode == TOBANK)
        d.z = height+FLY_HEIGHT - h;
      else
        d.z = height+DROP_HEIGHT - h;

      if (dist<0.0025)
      {
        if (theta!=dest_theta)
          dangle = i4_rotate_to(theta, dest_theta, defaults->turn_speed);
        else if (mode == TOBANK)
          mode = GRABBING;
        else
          mode = DROPPING;
        d.set(0,0,0);
      }
      else
      {
        i4_float angle = i4_atan2(d.y,d.x);
        i4_normalize_angle(angle);    

        if (dist>damp_distance)
        {
          dangle = i4_rotate_to(theta, angle, defaults->turn_speed);
          if (speed<defaults->speed)
            speed += defaults->accel;
          else
            speed = defaults->speed;
        }
        else
        {
          speed = (dist+0.1)*-log(0.9);
          if (theta!=dest_theta)
            dangle = i4_rotate_to(theta, dest_theta, defaults->turn_speed);
        }
        
        d.x = cos(angle)*speed;
        d.y = sin(angle)*speed;
      }

      if (dangle<-0.01)
        roll += (0.7 - roll)*0.1;
      else if (dangle>0.01)
        roll += (-0.7 - roll)*0.1;
      else
        roll += -roll*0.1;

      pitch += -pitch*0.1;

      move(d);
    } break;
    case DROPPING:
      if (crate())
      {
        g1_player_man.get(player_num)->money() += crate()->value();
        char msg[100];
        sprintf(msg, "Money Received : $%d", crate()->value());
        g1_player_man.show_message(msg, 0x00ff00, player_num);

        mo_money.play();
        crate()->release();
        set_crate(0);
      }
      mode = LAUNCHING;
      break;
    case LAUNCHING:
      if (h<height + FLY_HEIGHT)
        h += (height+FLY_HEIGHT+0.05 - h)*0.1;
      else if (g1_bank_list_count[player_num]>0)
      {
        int n=g1_rand(43)%g1_bank_list_count[player_num];
        
        i4_isl_list<g1_bank_class>::iterator p = g1_bank_list[player_num].begin();
        for (int i=0; i<n; i++, p++) ;
          
        attack_target = &*p;

        if (attack_target.valid())
        {
          i4_3d_vector pos;
          p->crate_location(pos);
          pos -= crate_attach;
          dest_x = pos.x;
          dest_y = pos.y;
          dest_z = pos.z;
          dest_theta = p->theta;
          mode = TOBANK;
        }
      }
      break;
    case GRABBING:
      if (!attack_target.valid())
        mode = RETURNING;
      else
      {
        i4_bool ok=i4_T;

        ok = (i4_rotate_to(theta,0,defaults->turn_speed)==0) && ok;
        ok = (i4_rotate_to(pitch,0,defaults->turn_speed)==0) && ok;
        ok = (i4_rotate_to(roll, 0,defaults->turn_speed)==0) && ok;

        if (h>dest_z)
        {
          h -= VSPEED;
          ok = i4_F;
        }

        if (ok)
        {
          g1_bank_class *bank = (g1_bank_class *)attack_target.get();
          set_crate(bank->crate());
          bank->detach_crate();

          mode = LIFTING;
        }
      }
      break;
    case LIFTING:
    {
      if (h<height+DROP_HEIGHT)
      {
        i4_3d_vector d(0,0,VSPEED*0.2);
        pitch += (0.4 - pitch)*0.1;
        move(d);
      }
      else
        mode = RETURNING;
    } break;
    case DYING:
    {
      i4_float &roll_speed  = dest_x;
      i4_float &pitch_speed = dest_y;
      
      pitch_speed += 0.004;
      pitch += pitch_speed;

      roll_speed += 0.008;
      roll += roll_speed;

      vspeed -= (g1_resources.gravity * 0.1);

      i4_3d_vector d;
      d.set(speed*cos(theta), speed*sin(theta), vspeed);
      move(d);

      if (h<=terrain_height)
        g1_map_piece_class::damage(0,health,i4_3d_vector(0,0,1));               // die somehow!!!
    } break;
  }

  groundpitch = lgroundpitch = 0;
  groundroll = groundroll = 0;
  request_think();

  li_mode() = mode;
}
