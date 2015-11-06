/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "g1_object.hh"
#include "math/num_type.hh"
#include "math/pi.hh"
#include "g1_rand.hh"
#include "objs/debris.hh"
#include "saver.hh"
#include "map.hh"
#include "g1_render.hh"
#include "object_definer.hh"
#include "dll/dll.hh"
#include "map_man.hh"
#include "math/pi.hh"
#include "draw_context.hh"
#include "r1_clip.hh"
#include "r1_api.hh"

const i4_float FUNNEL_HEIGHT=7.0;
const i4_float FUNNEL_RADIUS=1.0;
const int DATA_VERSION=1;
const int MAX_VALS=256;
const i4_float RADIUS=0.01;
static i4_float rand_val[MAX_VALS];
static int rand_i;
inline i4_float fast_rand() { rand_i=(rand_i+1)&(MAX_VALS-1); return rand_val[rand_i]; }

void g1_debris_init()
{
  for (int i=0; i<MAX_VALS; i++)
    rand_val[i] = g1_float_rand(i)*RADIUS;
  rand_i=0;
}

g1_object_definer<g1_debris_class> g1_debris_def("debris",
                                                 g1_object_definition_class::EDITOR_SELECTABLE,
                                                 g1_debris_init);


g1_debris_class::g1_debris_class(g1_object_type id, g1_loader_class *fp)
  : g1_object_class(id, fp)
{  
  w16 ver,data_size;

  if (fp)
    fp->get_version(ver,data_size);
  else
    ver = 0;

  switch (ver)
  {
    case DATA_VERSION:
      break;
    default:
      break;
  }
  type = RING_CHUNKS;
  rad = 0.5;
  strength = 2.0;
}

void g1_debris_class::save(g1_saver_class *fp)
{
  g1_object_class::save(fp);  
  fp->start_version(DATA_VERSION);  
  fp->end_version();
}

void g1_debris_class::setup(i4_float sx, i4_float sy, i4_float sz, int _type)
{
  g1_map_class *map=g1_get_map();

  if (sx<1) sx=1;
  if (sy<1) sy=1;
  if (sx>map->width()-1) sx=map->width()-1;
  if (sy>map->height()-1) sy=map->height()-1;

  x=lx=sx;
  y=ly=sy;
  h=lh=sz;
  type = _type;

  int i;
  switch (type)
  {
    case TORNADO:
      for (i=0; i<MAX_DEBRIS; i++)
      {
        debris[i].x = g1_float_rand(i)*FUNNEL_RADIUS*2.0-FUNNEL_RADIUS;
        debris[i].y = g1_float_rand(i+2)*FUNNEL_RADIUS*2.0-FUNNEL_RADIUS;
        debris[i].z = g1_float_rand(i+4)*FUNNEL_HEIGHT;
        ldebris[i] = debris[i];
      }
      break;
    case RING_CHUNKS:
      for (i=0; i<MAX_DEBRIS; i++)
      {
        debris[i].x = g1_float_rand(i+4)*rad*2.0-rad;
        debris[i].y = g1_float_rand(i+6)*rad*2.0-rad;
        debris[i].z = g1_float_rand(i+8)*strength;
        ldebris[i] = debris[i];
      }
      break;
  }
  occupy_location();
  request_think();
}


void g1_debris_class::think()
{
  i4_float 
    spin_speed = 0.3,
    cs = cos(spin_speed)*1.05,
    sn = sin(spin_speed)*1.05;

  unoccupy_location();
  int i;
  switch (type)
  {
    case TORNADO:
      for (i=0; i<MAX_DEBRIS; i++)
      {
        ldebris[i] = debris[i];
        debris[i].z += 0.4;
        debris[i].x = debris[i].x*cs - debris[i].y*sn + fast_rand();
        debris[i].y = debris[i].x*sn + debris[i].y*cs + fast_rand();
        if (debris[i].z>FUNNEL_HEIGHT)
        {
          debris[i].x = g1_float_rand(i+0)*2*FUNNEL_RADIUS-FUNNEL_RADIUS;
          debris[i].y = g1_float_rand(i+1)*2*FUNNEL_RADIUS-FUNNEL_RADIUS;
          debris[i].z = g1_float_rand(i+2)*0.3;
          ldebris[i] = debris[i];
        }
      }
      break;
    case RING_CHUNKS:
      rad += 0.2;
      strength *= 0.94;
      if (rad>10.0)
      {
        request_remove();
        return;
      }
      for (i=0; i<MAX_DEBRIS; i++)
      {
        ldebris[i] = debris[i];
        debris[i].z += 0.2;
        if (debris[i].z>strength)
        {
          i4_float th = g1_float_rand(i+0)*2*i4_pi();
          debris[i].x = cos(th)*rad;
          debris[i].y = sin(th)*rad;
          debris[i].z = g1_float_rand(i+8)*0.2;
          ldebris[i] = debris[i];
        }
      }
      break;
  }
  occupy_location();
  request_think();
}

void g1_debris_class::draw(g1_draw_context_class *context)
{    
  sw32 i,j;

  r1_render_api_class *r_api = g1_render.r_api;

  i4_transform_class tmp,trans = *context->transform;
  r1_vert      p, vert[3];
  i4_3d_vector ip;

  vert[0].a=vert[1].a=vert[2].a = 1;
  vert[0].r=vert[0].g=vert[0].b = 0;
  vert[1].r=vert[1].g=vert[1].b = 0; 
  vert[2].r=vert[2].g=vert[2].b = 0;

  tmp.translate(x,y,h);
  trans.multiply(tmp);

  i4_float tx[3],ty[3];
  i4_bool ok;
  i4_float fr=g1_render.frame_ratio; 

  r_api->set_write_mode(R1_WRITE_COLOR | R1_COMPARE_W | R1_WRITE_W);
  r_api->set_shading_mode(R1_COLORED_SHADING);
  r_api->set_alpha_mode(R1_ALPHA_DISABLED);
  r_api->disable_texture();
  for (i=0; i<MAX_DEBRIS; i++)
  {
    ip.set((debris[i].x-ldebris[i].x)*fr + ldebris[i].x, 
           (debris[i].y-ldebris[i].y)*fr + ldebris[i].y, 
           (debris[i].z-ldebris[i].z)*fr + ldebris[i].z);
    trans.transform(ip, *p.point());
    if (!r1_calc_outcode(&p))
    {
      p.v.x *= g1_render.scale_x;
      p.v.y *= g1_render.scale_y;
      p.w = 1/p.v.z;
      p.px = p.v.x * p.w;
      p.py = p.v.y * p.w;
      ok=i4_T;
      tx[0] = p.px+fast_rand()-RADIUS/2; ty[0] = p.py-fast_rand();
      tx[1] = p.px-fast_rand(); ty[1] = p.py+fast_rand();
      tx[2] = p.px+fast_rand(); ty[2] = p.py+fast_rand();
      for (j=0; j<3 && ok; j++)
      {
        if (tx[j]<=-1.0 || tx[j]>=1.0 || ty[j]<=-1.0 || ty[j]>=1.0)
          ok = i4_F;
        else
        {
          vert[j].w = p.w;
          vert[j].v.x = p.v.x; vert[j].v.y = p.v.y; vert[j].v.z = p.v.z;
          
          vert[j].px = tx[j]*g1_render.center_x + g1_render.center_x;
          vert[j].py = ty[j]*g1_render.center_y + g1_render.center_y;
        }
      }
      if (ok)
        r_api->render_poly(3,vert);
    }
  }  
}

