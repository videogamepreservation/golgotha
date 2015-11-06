/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "g1_object.hh"
#include "lisp/lisp.hh"
#include "sound_man.hh"
#include "objs/bullet.hh"
#include "objs/buster_rocket.hh"
#include "objs/beam_weapon.hh"
#include "objs/guided_missile.hh"
#include "objs/super_mortar.hh"
#include "objs/dropped_bomb.hh"
#include "objs/bolt.hh"
#include "resources.hh"
#include "objs/defaults.hh"
#include "g1_rand.hh"
#include "sound/sfx_id.hh"
#include "player.hh"

static li_symbol_ref b90mm("b90mm"), b120mm("b120mm"), 
  buster_rocket("buster_rocket"), heavy_rocket("heavy_rocket"),
  chain_gun("chain_gun"), plasma("plasma"), beam("beam"),
  napalm("napalm"), acid("acid"),
  vortex("vortex_missile"), nuke("nuke_missile"),
  guided_missile("guided_missile"),
  dropped_bomb("dropped_bomb"),
  super_mortar("super_mortar"),
  bolt("bolt"),
  light_type("lightbulb");


S1_SFX(small_fire_sfx, "fire/peon_tank.wav", S1_3D, 30);
S1_SFX(missile_fire_sfx, "fire/missile_truck.wav", S1_3D, 30);
S1_SFX(chain_fire_sfx, "fire/machine_gun_fire_22khz_lp.wav", S1_3D, 50);
S1_SFX(b120mm_sfx, "fire/supertank/main1.wav", S1_3D, 50);
S1_SFX(napalm_sfx, "fire/supertank_plazma_fireball_22khz.wav", S1_3D, 50);

S1_SFX(supergun_fire, "fire/big_begrtha_gun_fire_22khz.wav", S1_STREAMED, 200);
S1_SFX(acid_sfx, "fire/supertank_acid_main_barrel_interrior_22khz.wav", S1_3D, 200);
S1_SFX(stank_missile_sfx, "fire/supertank_fires_rocket_22khz.wav", S1_3D, 200);

static r1_texture_ref chain_gun_texture("auto1");

g1_object_class *g1_fire(li_symbol *fire_type,
                         g1_object_class *who_is_firing,
                         g1_object_class *who_firing_at,
                         const i4_3d_vector &start_pos,
                         const i4_3d_vector &direction,
                         g1_object_class *last_object_fired)
{
  int otype=g1_get_object_type(fire_type);
  if (!otype)
    return 0;

  g1_object_class *o=0;  
  if (fire_type==bolt.get() && last_object_fired)
    o=last_object_fired;
  
  if (!o)
  {
    o=g1_create_object(otype);
    if (who_is_firing && o)
      o->player_num=who_is_firing->player_num;
  }
  
  if (!o)
    return 0;

  g1_damage_map_struct *dmap=o->get_type()->get_damage_map();  
  float range=dmap->range;


  if (fire_type==b90mm.get() ||
      fire_type==b120mm.get() ||
      fire_type==napalm.get() ||
      fire_type==acid.get())
  {
    if (fire_type==acid.get())
      acid_sfx.play(start_pos.x, start_pos.y, start_pos.z);
    else if (fire_type==b120mm.get())
      b120mm_sfx.play(start_pos.x, start_pos.y, start_pos.z);
    else if (fire_type==napalm.get())
      napalm_sfx.play(start_pos.x, start_pos.y, start_pos.z);
    else
      small_fire_sfx.play(start_pos.x, start_pos.y, start_pos.z);

    i4_3d_vector vel=direction;
    vel.normalize();              
    vel*=dmap->speed;


    float r,g,bl;
    if (fire_type==b90mm.get() || fire_type==b120mm.get())
      r=g=bl=1;
    else  if (fire_type==napalm.get())
    { r=1; g=0.5; bl=0; }
    else if (fire_type==acid.get())
    { r=0; g=1; bl=0; }
        


    g1_light_object_class *light;
    light = (g1_light_object_class *)g1_create_object(g1_get_object_type(light_type.get()));

    light->setup(start_pos.x, start_pos.y, start_pos.z, r,g,bl, 1);
    light->occupy_location();


    g1_bullet_class *b=((g1_bullet_class *)o);
    b->setup(start_pos, vel, who_is_firing, range, g1_bullet_class::SPINS, 0, light);

  }
  else if (fire_type==buster_rocket.get())
  {
    ((g1_buster_rocket_class *)o)->setup(start_pos, direction, who_is_firing, who_firing_at);
    
  } else if (
           fire_type==heavy_rocket.get() ||
           fire_type==vortex.get() ||
           fire_type==nuke.get())
  {
    stank_missile_sfx.play(start_pos.x, start_pos.y, start_pos.z);
    ((g1_buster_rocket_class *)o)->setup(start_pos, direction, who_is_firing, who_firing_at);
  }

  else if (fire_type==beam.get() ||
           fire_type==plasma.get())
    ((g1_beam_weapon_class *)o)->setup(start_pos, direction, who_is_firing, 6.0);
  else if (fire_type==chain_gun.get())
  {
    i4_3d_vector pos=start_pos;

    float spread=0.1, spread_sub=0.05;
    
    pos.x+=g1_float_rand(2)*spread - spread_sub;
    pos.y+=g1_float_rand(3)*spread - spread_sub;
    pos.z+=g1_float_rand(8)*spread - spread_sub;
    
    i4_3d_vector vel=direction;
    vel.normalize();              
    vel*=dmap->speed;

  
    ((g1_bullet_class *)o)->setup(pos, vel, who_is_firing, range,
                                  g1_bullet_class::IS_SPRITE, chain_gun_texture.get());
    chain_fire_sfx.play(start_pos.x, start_pos.y, start_pos.z);
  }
  else if (fire_type==guided_missile.get())    
  {
    missile_fire_sfx.play(start_pos.x, start_pos.y, start_pos.z);
    //    g1_sound_man.play(STATIC_3D_SOUND,g1_sfx_fire_missile_truck, start_pos);
    ((g1_guided_missile_class *)o)->setup(start_pos, direction,
                                          who_is_firing, who_firing_at);
  }
  else if (fire_type==dropped_bomb.get())
  {
    ((g1_dropped_bomb_class *)o)->setup(start_pos, who_is_firing);
  }
  else if (fire_type==super_mortar.get())
  {
    ((g1_super_mortar_class *)o)->setup(start_pos, direction, who_is_firing);  
    supergun_fire.play();
  }
  else if (fire_type==bolt.get())
  {
    if (who_firing_at)
      ((g1_bolt_class *)o)->setup(start_pos, i4_3d_vector(0,0,0), who_is_firing, who_firing_at);
  }
  else
  {
    delete o;
    o=0;
  }
  
  return o;
}  
