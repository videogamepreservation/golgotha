/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "g1_object.hh"
#include "objs/model_draw.hh"
#include "math/num_type.hh"
#include "math/pi.hh"
#include "math/angle.hh"
#include "math/trig.hh"
#include "math/random.hh"
#include "objs/dropped_bomb.hh"
#include "tile.hh"
#include "objs/explosion1.hh"
#include "objs/shockwave.hh"
#include "saver.hh"
#include "objs/map_piece.hh"
#include "objs/smoke_trail.hh"
#include "map.hh"
#include "map_man.hh"
#include "resources.hh"
#include "object_definer.hh"

static g1_object_type smoke_trail_type, explosion_type, shockwave;
void g1_dropped_bomb_init()
{
  smoke_trail_type = g1_get_object_type("smoke_trail");
  explosion_type = g1_get_object_type("explosion1");
  shockwave = g1_get_object_type("shockwave");
}

g1_object_definer<g1_dropped_bomb_class>
g1_dropped_bomb_def("dropped_bomb",
                    g1_object_definition_class::MOVABLE,
                    g1_dropped_bomb_init);

S1_SFX(mo_money, "explosion/ariplane_bomb_explosion_one_22khz.wav", S1_STREAMED | S1_3D, 70);

void g1_dropped_bomb_class::draw(g1_draw_context_class *context)
{
  g1_model_draw(this, draw_params, context);
  if (smoke_trail.valid())
    smoke_trail->draw(context);
}

g1_dropped_bomb_class::g1_dropped_bomb_class(g1_object_type id,
                                             g1_loader_class *fp)
  : g1_object_class(id, fp)
{
  draw_params.setup("jetbomb");

  w32 i;

  if (fp && fp->check_version(DATA_VERSION))
  {    
    fp->read_reference(who_fired_me);   
    fp->read_reference(smoke_trail);
    fp->read_16();                        // old damage amount
    fp->end_version(I4_LF);
  }

  radar_type=G1_RADAR_WEAPON;  
  set_flag(AERIAL        | 
           HIT_GROUND    |
           SHADOWED, 1);
}

void g1_dropped_bomb_class::save(g1_saver_class *fp)
{
  // save data associated with base classes
  g1_object_class::save(fp);

  fp->start_version(DATA_VERSION);
  
  fp->write_reference(who_fired_me); 
  fp->write_reference(smoke_trail);
  fp->write_16(0);                     // old damage amount

  fp->end_version();
}

void g1_dropped_bomb_class::think()
{    
  z_velocity -= g1_resources.gravity*0.5;
  speed *= 0.7;
  i4_rotate_to(pitch,i4_pi(),0.2);
  
  if (move(i4_3d_vector(cos(theta)*speed, sin(theta)*speed, z_velocity)))
    request_think();
}

void g1_dropped_bomb_class::setup(const i4_3d_vector &pos,
                                  g1_object_class *this_guy_fired_me)
{
  w32 i;

  x=lx=pos.x;
  y=ly=pos.y;
  h=lh=pos.z;
  theta = this_guy_fired_me->theta;
  
  z_velocity = 0;
  speed = get_type()->get_damage_map()->speed;

  who_fired_me = this_guy_fired_me;
  player_num = this_guy_fired_me->player_num;
  request_think();
  occupy_location();
}

void g1_dropped_bomb_class::delete_smoke()
{
  if (smoke_trail.valid())
  {
    g1_object_class *s=smoke_trail.get();
    s->unoccupy_location();
    s->request_remove();
    smoke_trail=0;
  }
}


i4_bool g1_dropped_bomb_class::move(const i4_3d_vector &vel)
{
//   g1_smoke_trail_class *s;
    
//   if (!smoke_trail.valid())
//   {
//     s=(g1_smoke_trail_class *)g1_create_object(smoke_trail_type);
//     if (s)
//     {
//       s->setup(x, y, h, 0.02, 0.05, 0x0000ff, 0xffffff);   // blue to white
//       s->occupy_location();
//       smoke_trail=s;
//     }
//   } else s=(g1_smoke_trail_class *)smoke_trail.get();


  unoccupy_location();
      
  i4_3d_vector pos(x,y,h),ray(vel);
  g1_object_class *tmphit;
  int hit = g1_get_map()->check_non_player_collision(player_num,pos,ray,tmphit);
  pos += ray;
  if (hit)
  {
    if (hit>0)
    {
      g1_explosion1_class *exp = (g1_explosion1_class *)g1_create_object(explosion_type);
      if (exp)
        exp->setup(pos.x,pos.y,pos.z);
      g1_apply_damage(this, who_fired_me.get(), 0, vel);
    }
    delete_smoke();
    request_remove();
    return i4_F;
  }

  x = pos.x;
  y = pos.y;
  h = pos.z;
  occupy_location();
//   if (s)
//     s->update_head(x,y,h);

  return i4_T;  
}

