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
#include "objs/super_mortar.hh"
#include "tile.hh"
#include "objs/shockwave.hh"
#include "saver.hh"
#include "objs/map_piece.hh"
#include "objs/smoke_trail.hh"
#include "map.hh"
#include "map_man.hh"
#include "resources.hh"
#include "object_definer.hh"
#include "sound/sfx_id.hh"



static g1_object_type smoke_trail_type, shockwave;
void g1_super_mortar_init()
{
  smoke_trail_type = g1_get_object_type("smoke_trail");
  shockwave = g1_get_object_type("shockwave");
}

g1_object_definer<g1_super_mortar_class>
g1_super_mortar_def("super_mortar", 0, g1_super_mortar_init);


void g1_super_mortar_class::draw(g1_draw_context_class *context)
{
  g1_model_draw(this, draw_params, context);
  if (smoke_trail.valid())
    smoke_trail->draw(context);
}

g1_super_mortar_class::g1_super_mortar_class(g1_object_type id,
                                             g1_loader_class *fp)
  : g1_object_class(id, fp)
{
  draw_params.setup("jetbomb");

  w32 i;

  if (fp && fp->check_version(DATA_VERSION))
  {    
    fp->read_reference(who_fired_me);   
    fp->read_reference(smoke_trail);
    fp->read_16();                   // old damage amount
    fp->end_version(I4_LF);
  }

  radar_type=G1_RADAR_WEAPON;  
  set_flag(AERIAL        | 
           HIT_GROUND    |
           SHADOWED, 1);
  
  //  rumble_sound = g1_sound_man.alloc_dynamic_3d_sound(g1_sfx_misc_turbine1);
}

void g1_super_mortar_class::save(g1_saver_class *fp)
{
  // save data associated with base classes
  g1_object_class::save(fp);

  fp->start_version(DATA_VERSION);
  
  fp->write_reference(who_fired_me); 
  fp->write_reference(smoke_trail);
  fp->write_16(0);                     // old damage_amount

  fp->end_version();
}

void g1_super_mortar_class::think()
{    
  i4_rotate_to(pitch,i4_pi(),0.2);


  if (move(vel))
  {
    request_think();
    vel.z -= g1_resources.gravity;
//     if (rumble_sound)
//       g1_sound_man.update_dynamic_3d_sound(rumble_sound, i4_3d_vector(x,y,h), vel,1.0);
  }
  else
  {    
    who_fired_me=0;
    delete_smoke();
    unoccupy_location();
    request_remove();
//     if (rumble_sound)
//     {
//       g1_sound_man.free_dynamic_3d_sound(rumble_sound);
//       rumble_sound = 0;
//     }
  }
}

void g1_super_mortar_class::setup(const i4_3d_vector &pos,
                                  const i4_3d_vector &dir,
                                  g1_object_class *this_guy_fired_me) 
{
  w32 i;
  x=lx=pos.x;
  y=ly=pos.y;
  h=lh=pos.z;
  
  vel = dir;

  who_fired_me = this_guy_fired_me;
  player_num = this_guy_fired_me->player_num;

  ltheta=theta = i4_atan2(dir.y, dir.x);   
  pitch=lpitch = i4_atan2(-dir.z, sqrt(dir.x * dir.x +
                                       dir.y * dir.y));
  lroll=roll   = 0;
  

  occupy_location();
  request_think();
}

void g1_super_mortar_class::delete_smoke()
{
  if (smoke_trail.valid())
  {
    g1_object_class *s=smoke_trail.get();
    s->unoccupy_location();
    s->request_remove();
    smoke_trail=0;
  }
}


i4_bool g1_super_mortar_class::move(i4_3d_vector &vel)
{
  g1_smoke_trail_class *s;
  if (!smoke_trail.valid())
  {
    s=(g1_smoke_trail_class *)g1_create_object(smoke_trail_type);
    if (s)
    {
      s->setup(x, y, h, 0.02, 0.05, 0x0000ff, 0xffffff);   // blue to white
      s->occupy_location();
      smoke_trail=s;
    }
  } else s=(g1_smoke_trail_class *)smoke_trail.get();

  i4_3d_vector pos(x,y,h);
  g1_object_class *tmp_hit;
  int hit;
  
  hit = g1_get_map()->check_non_player_collision(player_num, pos, vel, tmp_hit);
  pos += vel;

  if (hit<0)
    // off the map
    return i4_F;
  else if (hit>0 && !tmp_hit)
  {
    // hit ground
    g1_shockwave_class *shock = NULL;
    shock = (g1_shockwave_class *)g1_create_object(shockwave);
    if (shock)
      shock->setup(pos, 0.5);

    g1_apply_damage(this, who_fired_me.get(), tmp_hit, vel);

    return i4_F;    
  }

  unoccupy_location();
  x = pos.x;
  y = pos.y;
  h = pos.z;
  occupy_location();

  if (s)
    s->update_head(x,y,h);

  return i4_T;  
}

