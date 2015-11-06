/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "objs/flak.hh"
#include "map.hh"
#include "map_man.hh"
#include "objs/light_o.hh"
#include "g1_render.hh"
#include "time/profile.hh"
#include "object_definer.hh"
#include "r1_api.hh"
#include "math/pi.hh"
#include "objs/particle_emitter.hh"
#include "sound_man.hh"
#include "sound/sfx_id.hh"
#include "resources.hh"
#include "g1_rand.hh"
#include "lisp/li_class.hh"
#include "draw_context.hh"
#include "r1_clip.hh"

S1_SFX(flak_sfx, "misc/total_miss_1.wav", S1_3D, 70);

static r1_texture_ref flak("lil_flak");

g1_object_definer<g1_flak_class> 
g1_flak_def("flak", 0);

g1_flak_class::g1_flak_class(g1_object_type id, g1_loader_class *fp)
  : g1_object_class(id, fp)
{
  time = 0;
  x = -1;
}

void g1_flak_class::setup(const i4_3d_vector &pos, i4_float _size)
{
  x=lx=pos.x;
  y=ly=pos.y;
  h=lh=pos.z;
  size = _size;

  if (!occupy_location())
    return;

  request_think();
  flak_sfx.play(x,y,h);
}

void g1_flak_class::think()
{
  time++;

  if (time<4)
    request_think();
  else
  {
    unoccupy_location();
    request_remove();
  }
}

void g1_flak_class::draw(g1_draw_context_class *context)
{
  i4_3d_vector screen_pos;
  context->transform->transform(i4_3d_point_class(x,y,h), screen_pos);

  i4_float center_x=g1_render.center_x, center_y=g1_render.center_y;
  i4_float w  = 1 / screen_pos.z;
  i4_float xs = center_x * w * g1_render.scale_x;
  i4_float ys = center_y * w * g1_render.scale_y;

  if (screen_pos.z > r1_near_clip_z)
  {
    i4_float 
      cx=center_x + screen_pos.x*xs,
      cy=center_y + screen_pos.y*ys,
      sz=size * center_x * w;
    int frame = time;
    i4_float 
      s = i4_float(frame%2)*0.5,
      t = i4_float(frame/2)*0.5;
        
    r1_clip_render_textured_rect(i4_f_to_i(cx-sz), i4_f_to_i(cy-sz), 
                                 i4_f_to_i(cx+sz), i4_f_to_i(cy+sz), 
                                 screen_pos.z, 0.7,
                                 i4_f_to_i(center_x*2), i4_f_to_i(center_y*2),
                                 flak.get(), 0, g1_render.r_api,
                                 s,t,s+0.5,t+0.5);
  }
}
