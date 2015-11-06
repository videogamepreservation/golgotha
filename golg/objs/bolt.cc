/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "math/pi.hh"
#include "math/trig.hh"
#include "math/angle.hh"
#include "g1_rand.hh"
#include "saver.hh"
#include "map_cell.hh"
#include "map.hh"
#include "map_man.hh"
#include "g1_render.hh"
#include "object_definer.hh"
#include "objs/light_o.hh"
#include "objs/bolt.hh"

#include "time/profile.hh"

#include "camera.hh"
#include "resources.hh"
#include "lisp/lisp.hh"

static i4_profile_class pf_bolt("bolt_think");


S1_SFX(fire_sfx, "fire/electric_tower_firing_three_22khz_lp.wav", S1_3D, 30);

static li_symbol_ref light_type("lightbulb");


g1_object_definer<g1_bolt_class>
g1_bolt_def("bolt");

g1_bolt_class::g1_bolt_class(g1_object_type id, g1_loader_class *fp)
  : g1_object_class(id,fp)
//{{{
{    
  memset(arc,0,sizeof(arc_point) * NUM_ARCS);
  first = 1;
  size = 0.8;
  w1 = w2 = 0.02;
  a1 = a2 = 1.0;
  c1 = 0xffffff;
  c2 = 0x8800ff;
  lit = i4_F;
}
//}}}

void g1_bolt_class::setup(const i4_3d_vector &start_pos,
                          const i4_3d_vector &_target_pos,
                          g1_object_class *_originator,
                          g1_object_class *_target)
//{{{
{
  move(start_pos);

  ticks = 1;
  originator = _originator;
  target = _target;

  if (target.valid())
    target_pos.set(target->x,target->y,target->h);
  else
    target_pos = _target_pos;

  if (!get_flag(MAP_OCCUPIED))
  {
    occupy_location();
    request_think();
  }

  if (lit && get_flag(MAP_OCCUPIED))
    create_light();
}
//}}}
   
g1_bolt_class::~g1_bolt_class()
//{{{
{
}
//}}}

void g1_bolt_class::save(g1_saver_class *fp)
//{{{
{
  g1_object_class::save(fp);
}
//}}}

void g1_bolt_class::draw(g1_draw_context_class *context)
//{{{
{  
  int i;
  i4_3d_point_class pts[g1_bolt_class::NUM_ARCS];

  for (i=0;i<g1_bolt_class::NUM_ARCS;i++)
    pts[i].interpolate(arc[i].lposition,arc[i].position,g1_render.frame_ratio);
  
  g1_render.add_translucent_trail(context->transform,pts,
                                  g1_bolt_class::NUM_ARCS,
                                  w1,w2,a1,a2,c1,c2);
}
//}}}

void g1_bolt_class::copy_old_points()
//{{{
{
  sw32 i;
  for (i=0;i<NUM_ARCS;i++)
    arc[i].lposition = arc[i].position;
}
//}}}

void g1_bolt_class::arc_to(const i4_3d_vector &target)
//{{{
{
  i4_3d_vector r,p;
  i4_float map_point_height;

  i4_3d_vector point, ray;
  
  point.set(x,y,h);
  ray = target;
  ray -= point;
  ray /= NUM_ARCS-1;

  sw32 i;

  for (i=0; i<NUM_ARCS; i++)
  {
    i4_float sin_factor = sin(i4_pi()*i/(NUM_ARCS-1));

    r.x = ((g1_rand(23)&0xFFFF)/((i4_float)0xffff) - 0.5) * size*sin_factor;
    r.y = ((g1_rand(32)&0xFFFF)/((i4_float)0xffff) - 0.5) * size*sin_factor;
    r.z = ((g1_rand(45)&0xFFFF)/((i4_float)0xffff) - 0.5) * size*sin_factor;
    
    p.set(point.x + ray.x*i + r.x,
          point.y + ray.y*i + r.y,
          point.z + ray.z*i + r.z);
    
    map_point_height = g1_get_map()->map_height(p.x,p.y,p.z) + 0.05;
  
    if (p.z < map_point_height)
      p.z = map_point_height;
  
    arc[i].position = p;
  }

  if (end_light.get())
    end_light->move(target.x, target.y, target.z);
}
//}}}

void g1_bolt_class::think()
//{{{
{
  pf_bolt.start();
  if (target.valid())
    g1_apply_damage(this, originator.get(), target.get(), i4_3d_vector(0,0,0));
  pf_bolt.stop();
}
//}}}

void g1_bolt_class::request_remove()
{
  s1_end_looping(sfx_loop);
}

void g1_bolt_class::post_think()
//{{{
{
  if (ticks<=0)
  {
    unoccupy_location();
    destroy_light();
    request_remove();
    return ;
  }

  copy_old_points();

  if (target.valid())
    target_pos.set(target->x,target->y,target->h);

  arc_to(target_pos);

  if (first)
    copy_old_points();

  first=0;

  ticks--;
  request_think();
}
//}}}

void g1_bolt_class::move(const i4_3d_vector &pos)
//{{{
{
  x = pos.x;
  y = pos.y;
  h = pos.z;

  if (light.valid())
    light->move(x,y,h);
}
//}}}

void g1_bolt_class::create_light()
{
  sfx_loop.setup(x,y,h,0,0,0,1);
  fire_sfx.play_looping(sfx_loop);

  if (!light.get())
  {
    float r=g1_resources.visual_radius();
    if (g1_current_view_state()->dist_sqrd(i4_3d_vector(x,y,h))<r*r)
    {
      light = (g1_light_object_class *)g1_create_object(g1_get_object_type(light_type.get()));
      light->setup(x,y,h+0.3, 
                   i4_float(c1>>16&0xff)/256.0, 
                   i4_float(c1>>8&0xff)/256.0, 
                   i4_float(c1&0xff)/256.0, 
                   1);
      light->occupy_location();
      
      end_light = (g1_light_object_class *)g1_create_object(g1_get_object_type(light_type.get()));
      end_light->setup(target_pos.x, target_pos.y, target_pos.z, 
                       i4_float(c2>>16&0xff)/256.0, 
                       i4_float(c2>>8&0xff)/256.0, 
                       i4_float(c2&0xff)/256.0, 
                       1);
      end_light->occupy_location();
    }
  }
}

void g1_bolt_class::destroy_light()
{
  s1_end_looping(sfx_loop);

  if (light.get())
  {
    light->unoccupy_location();
    light->request_remove();
    light=0;
  }
    
  if (end_light.get())
  {
    end_light->unoccupy_location();
    end_light->request_remove();
    end_light=0;
  }
}

//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}
