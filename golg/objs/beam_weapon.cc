/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "objs/beam_weapon.hh"
#include "objs/shrapnel.hh"
#include "sound/sfx_id.hh"
#include "map.hh"
#include "map_man.hh"
#include "objs/defaults.hh"
#include "lisp/lisp.hh"
#include "math/random.hh"
#include "g1_render.hh"
#include "object_definer.hh"
#include "g1_texture_id.hh"
#include "math/pi.hh"
#include "draw_context.hh"
#include "r1_clip.hh"
#include "tick_count.hh"

li_symbol_ref li_shrapnel("shrapnel"), beam("beam"), plasma("plasma");

g1_beam_weapon_class::g1_beam_weapon_class(g1_object_type id,
                                       g1_loader_class *fp)
  : g1_object_class(id,fp)
{    
  set_flag(AERIAL | HIT_GROUND, 1);
  range=0;
  idle_ticks=0;
  should_draw=0;
}

void g1_beam_weapon_class::setup(const i4_3d_vector &pos,
                               const i4_3d_vector &direction,
                               g1_object_class *this_guy_fired_me,
                               float range)
{
  x=pos.x;
  y=pos.y;
  h=pos.z;

  who_fired_me=this_guy_fired_me;

  if (this_guy_fired_me)
  {
    lx=pos.x + (this_guy_fired_me->lx - this_guy_fired_me->x);
    ly=pos.y + (this_guy_fired_me->ly - this_guy_fired_me->y);
    lh=pos.z + (this_guy_fired_me->lh - this_guy_fired_me->h);
  }
  else
  {
    lx=x;
    ly=y;
    lh=h;
  }

  if (!get_flag(MAP_OCCUPIED))
    occupy_location();
  
  i4_3d_vector dir=direction;
  dir.normalize();

  // sfxfix  
//   if (!fire_sound)
//   {
//     fire_sound=g1_sound_man.alloc_dynamic_3d_sound(g1_sfx_fire_supertank_auto1);
//     g1_sound_man.update_dynamic_3d_sound(fire_sound, pos, dir, 1.0);
//   }

  idle_ticks=0;
  should_draw=1;

  dir*=range;
  
  g1_object_class *hit=0;
  
  if (g1_get_map()->check_non_player_collision(player_num,pos,dir,hit))
  {
    if (hit)
      g1_apply_damage(this, this_guy_fired_me, hit, dir);
        
    g1_shrapnel_class *shrapnel = NULL;
    shrapnel = (g1_shrapnel_class *)g1_create_object(g1_get_object_type(li_shrapnel.get()));
    i4_3d_vector pos=i4_3d_vector(x,y,h);
    pos+=dir;
    
    if (shrapnel)
    {
      i4_float rx,ry,rh;
      rx = (i4_rand()&0xFFFF)/((i4_float)0xFFFF) * 0.2;
      ry = (i4_rand()&0xFFFF)/((i4_float)0xFFFF) * 0.2;
      rh = (i4_rand()&0xFFFF)/((i4_float)0xFFFF) * 0.2;
      shrapnel->setup(pos.x+rx, pos.y+ry, pos.z + rh,1, i4_F);
    }
  }

  end_point=pos;
  end_point+=dir;
  request_think();
}
  
void g1_beam_weapon_class::think()
{
  if (idle_ticks>5)
  {
    unoccupy_location();
    request_remove();
  }
  else
  {
    request_think();
    idle_ticks++;
  }
}

void g1_beam_weapon_class::unoccupy_location()
{
//   if (fire_sound) sfxfix
//   {
//     g1_sound_man.free_dynamic_3d_sound(fire_sound);
//     fire_sound=0;
//   }

  g1_object_class::unoccupy_location();
}


i4_bool g1_beam_weapon_class::occupy_location()
{
  float w=g1_get_map()->width(), h=g1_get_map()->height();
  
  if (!(x>=0 && x<w && y>=0 && y<h))
  {
    request_remove();
    return i4_F;
  }

  if (!(end_point.x>=0 && end_point.y>=0 && end_point.x<w && end_point.y<h))
  {
    request_remove();
    return i4_F;
  }

  g1_map_class *map=g1_get_map();
  map->add_object(*new_occupied_square(), i4_f_to_i(x), i4_f_to_i(y));
  map->add_object(*new_occupied_square(), i4_f_to_i(end_point.x), i4_f_to_i(end_point.y));  

  set_flag(MAP_OCCUPIED,1);
  
  return i4_T;
}



static void setup_p(r1_vert &rv, const i4_3d_vector &p,  float a, float r, float g, float b)
{
  rv.v.x=p.x;
  rv.v.y=p.y;
  rv.v.z=p.z;
  rv.a=a;
  rv.r=r;
  rv.g=g;
  rv.b=b; 
}
                    

static void draw_sprite(const i4_3d_vector &p, r1_texture_handle tex, float sprite_scale)
{

  if (p.z > r1_near_clip_z)
  {
    i4_float ooz = 1 / p.z;
    i4_float xs = g1_render.center_x * ooz * g1_render.scale_x;
    i4_float ys = g1_render.center_y * ooz * g1_render.scale_y;

    float cx=g1_render.center_x + p.x*xs;
    float cy=g1_render.center_y + p.y*ys;
    float w=sprite_scale * g1_render.center_x * 2 * ooz;
        
    r1_clip_render_textured_rect(i4_f_to_i(cx-w/2), i4_f_to_i(cy-w/2), 
                                 i4_f_to_i(cx+w/2), i4_f_to_i(cy+w/2), p.z,
                                 1.0,
                                 i4_f_to_i(g1_render.center_x*2),
                                 i4_f_to_i(g1_render.center_y*2),
                                 tex, 0, g1_render.r_api);        
  }
}

void g1_beam_weapon_class::draw(g1_draw_context_class *context)
{
  if (should_draw)
  {
    r1_vert      points[4];
    i4_3d_vector norm;

    float fr=g1_render.frame_ratio;
    i4_3d_vector start, step, t_start, up, side;
    start.interpolate(i4_3d_vector(x,y,h), i4_3d_vector(lx,ly,lh), fr);
    up=i4_3d_vector(0,0,1);
    
     
    step=end_point;
    step-=start;
    side.cross(up, step);
    side.normalize();
    side*=0.2;

    r1_render_api_class *api=g1_render.r_api;
    
    float cx=g1_render.center_x, cy=g1_render.center_y;

    r1_texture_handle texture=g1_get_texture("red_flare");
    float theta=g1_tick_counter*3.0 + fr;
    float theta_step=2*i4_pi()/10.0;

    
    for (float t=0; t<1; t+=(1.1-t)*0.3)
    {
      i4_3d_vector s=i4_3d_vector(start.x + step.x*t + side.x*cos(theta),
                                  start.y + step.y*t + side.y*sin(theta),
                                  start.z + step.z*t + side.z*cos(theta+0.5)*0.5);
    
      theta+=theta_step;
      
      context->transform->transform(s, t_start);            
      draw_sprite(t_start, texture, 0.1);
    }
  }
}

      

g1_object_definer<g1_beam_weapon_class> 
  g1_beam_def("beam", g1_object_definition_class::EDITOR_SELECTABLE);

g1_object_definer<g1_beam_weapon_class> 
  g1_plasma_def("plasma", g1_object_definition_class::EDITOR_SELECTABLE);

