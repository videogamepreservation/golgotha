/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "g1_object.hh"
#include "math/num_type.hh"
#include "objs/shrapnel.hh"
#include "objs/explosion1.hh"
#include "saver.hh"
#include "map.hh"
#include "map_man.hh"
#include "math/angle.hh"
#include "math/trig.hh"
#include "g1_rand.hh"
#include "math/random.hh"
#include "resources.hh"
#include "objs/particle_emitter.hh"
#include "g1_render.hh"
#include "object_definer.hh"
#include "draw_context.hh"
#include "r1_clip.hh"
#include "r1_api.hh"

static g1_object_type particle_emitter_type;
static r1_texture_ref gradient("fire_gradient");

enum { DATA_VERSION=2 };

const int MAX_SHRAPNEL_TIME=5;
  
void g1_shrapnel_init()
{
  particle_emitter_type = g1_get_object_type("particle_emitter");
}

g1_object_definer<g1_shrapnel_class>
g1_shrapnel_def("shrapnel", 0, g1_shrapnel_init);

g1_shrapnel_class::g1_shrapnel_class(g1_object_type id,
                                     g1_loader_class *fp)
  : g1_object_class(id,fp)
{  
  
  w16 ver,data_size;
  w32 i;

  num_shrapnel_pieces = 0;
  shrapnel_time = 0;
  x = -1;
}

void g1_shrapnel_class::save(g1_saver_class *fp)
{
  // save data associated with base classes
  g1_object_class::save(fp);
  
  fp->start_version(DATA_VERSION);
  fp->end_version();
}

i4_float t_grad_width=0.2;

void g1_shrapnel_class::draw(g1_draw_context_class *context)
{    
  w32 i;
  
  shrapnel_piece *p=shrapnel_pieces;
  p = shrapnel_pieces;

  i4_3d_vector i_pos;

  i4_transform_class trans = *context->transform;
  r1_vert      v[3];
  i4_3d_vector ip,op;
  i4_float offs = i4_float(shrapnel_time)*(1.0-t_grad_width)/MAX_SHRAPNEL_TIME;

  v[0].a=v[1].a=v[2].a = 1;
  v[0].r=v[0].g=v[0].b = 1;
  v[0].s=offs;
  v[0].t=0;
  v[1].r=v[1].g=v[1].b = 1; 
  v[1].s=offs+t_grad_width;
  v[1].t=0.5;
  v[2].r=v[2].g=v[2].b = 1;
  v[2].s=offs;
  v[2].t=1.0;

  trans.mult_translate(x,y,h);

  g1_render.r_api->use_texture(gradient.get(), 32, 0);

  for (i=0; i<num_shrapnel_pieces; i++,p++)
  {        
    i4_3d_vector d(p->position);
    d -= p->lposition;
    d *= 1.0 - g1_render.frame_ratio;
    ip = p->position;
    op = p->lposition;
    ip -= d;
    op -= d;

    trans.transform(ip, *v[0].point());
    trans.transform(op, *v[1].point());

    if (!r1_calc_outcode(&v[0]) && !r1_calc_outcode(&v[1]))
    {
      v[0].v.x *= g1_render.scale_x;
      v[0].v.y *= g1_render.scale_y;
      v[0].w = 1/v[0].v.z;
      v[0].px = v[0].v.x * v[0].w;
      v[0].py = v[0].v.y * v[0].w;

      v[1].v.x *= g1_render.scale_x;
      v[1].v.y *= g1_render.scale_y;
      v[1].w = 1/v[1].v.z;
      v[1].px = v[1].v.x * v[1].w;
      v[1].py = v[1].v.y * v[1].w;

      const i4_float SIZE=0.01;

      v[2].v = v[0].v;
      v[2].w = v[0].w;

      i4_float dir = ((v[0].px>v[1].px) ^ (v[0].py>v[1].py))? 1 : -1;

      v[2].px = v[0].px-i4_float_rand()*SIZE*v[0].w; 
      v[2].py = v[0].py-i4_float_rand()*SIZE*v[0].w*dir; 
      v[0].px = v[0].px+i4_float_rand()*SIZE*v[0].w; 
      v[0].py = v[0].py+i4_float_rand()*SIZE*v[0].w*dir; 
      
      i4_bool ok=i4_T;
      for (int j=0; j<3 && ok; j++)
      {
        if (v[j].px<=-1.0 || v[j].px>=1.0 || v[j].py<=-1.0 || v[j].py>=1.0)
          ok = i4_F;
        else
        {
          v[j].px = v[j].px*g1_render.center_x + g1_render.center_x;
          v[j].py = v[j].py*g1_render.center_y + g1_render.center_y;
        }
      }
      if (ok)
        g1_render.r_api->render_poly(3,v);
    }
  }    
}

void g1_shrapnel_class::think()
{    
  w32 i;

  if (shrapnel_time<MAX_SHRAPNEL_TIME)
  {
    shrapnel_time++;
    shrapnel_piece *p = shrapnel_pieces;
    for (i=0;i<num_shrapnel_pieces;i++,p++)
    {      
      p->lposition = p->position;
      p->position += p->velocity;
      p->velocity.z -= g1_resources.gravity;
    }
    request_think();
  } 
  else
  {
    unoccupy_location();
    request_remove();  
  }
}

static r1_texture_ref explosion2("explosions2");

void g1_shrapnel_class::setup(i4_float sx, i4_float sy, i4_float sz,
                              w32 _num_pieces, int type)
{
  x=lx=sx;
  y=ly=sy;
  h=lh=sz;

  if (!occupy_location())
    return;

  shrapnel_time = 0;
  num_shrapnel_pieces = _num_pieces;

  if (num_shrapnel_pieces > MAX_SHRAPNEL_PIECES)
    num_shrapnel_pieces=MAX_SHRAPNEL_PIECES;
  
  shrapnel_piece *p = shrapnel_pieces;
    
  for (int i=0;i<num_shrapnel_pieces;i++,p++)
  {
    i4_float f=0.2;
    i4_float
      rx = i4_float_rand() * f - f/2.0,
      ry = i4_float_rand() * f - f/2.0,
      rz = i4_float_rand() * f;  // - f/2.0;
    
    p->type=g1_rand(39)&3;

    p->lposition = p->position = i4_3d_vector(0,0,0);
    p->velocity = i4_3d_vector(rx,ry,rz);
  }

  request_think();
}
