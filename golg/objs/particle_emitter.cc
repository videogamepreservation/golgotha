/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "objs/particle_emitter.hh"
#include "object_definer.hh"
#include "g1_render.hh"
#include "r1_api.hh"
#include "g1_texture_id.hh"
#include "r1_clip.hh"
#include "g1_rand.hh"
#include "math/random.hh"
#include "map_man.hh"
#include "map.hh"
#include "time/profile.hh"
#include "lisp/li_class.hh"
#include "saver.hh"
#include "draw_context.hh"

i4_profile_class pf_part_emit("particle_emit::think");


g1_object_definer<g1_particle_emitter_class> 
g1_particle_emitter_def("particle_emitter", 
                        g1_object_definition_class::EDITOR_SELECTABLE |
                        g1_object_definition_class::HAS_ALPHA);


i4_bool g1_particle_emitter_class::occupy_location()
{
  i4_bool ret=g1_object_class::occupy_location();

  if (radius>2)
    g1_get_map()->add_object(cell_on, i4_f_to_i(x), i4_f_to_i(y));
  
  return ret;
}


void g1_particle_emitter_class::unoccupy_location()
{
  g1_object_class::unoccupy_location();

  if (radius>2)
    g1_get_map()->remove_object(cell_on);
}

void g1_particle_class::load(i4_file_class *fp)
{
  fp->read_format("ffffffff", &x,&y,&z, &xv,&yv,&zv, &grow_speed, &size);
  ticks_left=fp->read_32();
  lx=x; ly=y; lz=z;
}

void g1_particle_class::save(i4_file_class *fp)
{
  fp->write_format("ffffffff", &x,&y,&z, &xv,&yv,&zv, &grow_speed, &size);
  fp->write_32(ticks_left);

}

g1_particle_emitter_class::g1_particle_emitter_class(g1_object_type id, g1_loader_class *fp)
  : g1_object_class(id, fp)
{
  cell_on.object=this;
  stopping=i4_F;
  t_in_use=0;
  x1=y1=x2=y2=0;
  radius=0;

  params.emitter_lifetime=0;
  for (int i=0; i<MAX_PARTICLES; i++)
    particles[i].in_use=i4_F;

}

void g1_particle_emitter_class::save(g1_saver_class *fp)
{
  g1_object_class::save(fp);
}


r1_texture_ref g1_default_particle_texture("red_flare");

  
void g1_particle_emitter_params::defaults()
{
  texture=g1_default_particle_texture.get();

  start_size=0.2;
  grow_speed=0.01;
  grow_accel=0;
  
  creation_probability=0.75;
  max_speed=0.1;
  air_friction=0.95;
  particle_lifetime=20;
  emitter_lifetime = -1;
  num_create_attempts_per_tick=2;
  start_size_random_range=0;
  gravity=0;
  speed=i4_3d_vector(0,0,0);
}

void g1_particle_emitter_class::setup(float _x, float _y, float _h, 
                                      g1_particle_emitter_params &_params)
{
  params=_params;

  x=_x; y=_y; h=_h;
  grab_old();

  for (int i=0; i<MAX_PARTICLES; i++)
    particles[i].in_use=i4_F;   
  t_in_use=0;

  occupy_location();
  think();
}



void g1_particle_emitter_class::move(float new_x, float new_y, float new_h)
{
  li_class_context v_context(vars);
  unoccupy_location();
  grab_old();

  params.speed.x=new_x-x;
  params.speed.y=new_y-y;
  params.speed.z=new_h-h;
  
  x=new_x; y=new_y; h=new_h;
  occupy_location();
}

void g1_particle_emitter_class::draw(g1_draw_context_class *context)
{
  if (!t_in_use) return;

  float center_x=g1_render.center_x, center_y=g1_render.center_y;

  
  if (!params.texture) return;

  g1_render.r_api->set_filter_mode(R1_BILINEAR_FILTERING);

  float fr=g1_render.frame_ratio; 


  for (int i=0; i<MAX_PARTICLES; i++)
  {
    if (particles[i].in_use)
    {
      // calculate interpolated position of the particle
      float rx=(particles[i].x-particles[i].lx)*fr + particles[i].lx, 
        ry=(particles[i].y-particles[i].ly)*fr + particles[i].ly, 
        rh=(particles[i].z-particles[i].lz)*fr + particles[i].lz;
    
      i4_3d_vector screen_pos;
      context->transform->transform(i4_3d_point_class(rx,ry,rh), screen_pos);

      i4_float ooz = 1 / screen_pos.z;
      i4_float xs = center_x * ooz * g1_render.scale_x;
      i4_float ys = center_y * ooz * g1_render.scale_y;

      if (screen_pos.z > r1_near_clip_z)
      {
        float cx=center_x + screen_pos.x*xs;
        float cy=center_y + screen_pos.y*ys;
        float w=particles[i].size * center_x * 2 * ooz;
        
        r1_clip_render_textured_rect(i4_f_to_i(cx-w/2), i4_f_to_i(cy-w/2), 
                                     i4_f_to_i(cx+w/2), i4_f_to_i(cy+w/2), screen_pos.z,
                                     particles[i].ticks_left/(float)params.particle_lifetime,
                                     i4_f_to_i(center_x*2), i4_f_to_i(center_y*2),
                                     params.texture, 0, g1_render.r_api);        
      }
    }
  }

  g1_render.r_api->set_filter_mode(R1_NO_FILTERING);
}
  
void g1_particle_emitter_class::think()
{
  pf_part_emit.start();

  if (params.emitter_lifetime>0)
    params.emitter_lifetime--;
  else if (params.emitter_lifetime!=-1)
    stopping = i4_T;

  if (t_in_use!=MAX_PARTICLES && !stopping)   // check to see if we should add some more particles
  {
    int t_tries=params.num_create_attempts_per_tick;
    float max_s=params.max_speed;

    for (int j=0; j<t_tries; j++)
    {
      if (i4_float_rand()<params.creation_probability)
      {
        for (int i=0; i<MAX_PARTICLES; i++)
        {
          if (!particles[i].in_use)
          {
            particles[i].x=particles[i].lx=lx;
            particles[i].y=particles[i].ly=ly;
            particles[i].z=particles[i].lz=lh;
          

            particles[i].xv=(g1_float_rand(i+2)*2-1) * params.speed.x;
            if (particles[i].xv> max_s)
              particles[i].xv=max_s;

            particles[i].yv=(g1_float_rand(i+3)*2-1) * params.speed.y;
            if (particles[i].yv> max_s)
              particles[i].yv=max_s;

            particles[i].zv=(g1_float_rand(i+4)*2-1) * params.speed.z;
            if (particles[i].zv> max_s)
              particles[i].zv=max_s;

            particles[i].grow_speed=params.grow_speed;
            particles[i].size=params.start_size + i4_float_rand() * params.start_size_random_range;
            particles[i].ticks_left=params.particle_lifetime;
            particles[i].in_use=i4_T;

            t_in_use++;
            i=MAX_PARTICLES;
          }
        }
      }
    }
  }
  
  float bx1=100000,by1=100000,bx2=-100000,by2=-100000;

  if (t_in_use)
  {
    float airf=params.air_friction;
    for (int i=0; i<MAX_PARTICLES; i++)
    {
      if (particles[i].in_use)
      {
        if (particles[i].ticks_left<=0 || particles[i].size<=0)
        {
          particles[i].in_use=i4_F;
          t_in_use--;
        }
        else
        {
          particles[i].lx=particles[i].x;     // grab old position
          particles[i].ly=particles[i].y;
          particles[i].lz=particles[i].z;

          particles[i].x+=particles[i].xv;    // move with speed
          particles[i].y+=particles[i].yv;
          particles[i].z+=particles[i].zv;
          particles[i].size+=particles[i].grow_speed;

          particles[i].xv*=airf;    // apply air friction
          particles[i].yv*=airf;
          particles[i].zv=airf*particles[i].zv + params.gravity;

          particles[i].grow_speed+=params.grow_accel;

          particles[i].ticks_left--;

          if (particles[i].x<bx1) bx1=particles[i].x;
          if (particles[i].x>bx2) bx2=particles[i].x;
          if (particles[i].y<by1) by1=particles[i].y;
          if (particles[i].y>by2) by2=particles[i].y;
        }
      }
    }


    if (bx1!=100000)
    {
      if (bx1<0) bx1=0; 
      if (bx2>=g1_get_map()->width())  bx2=g1_get_map()->width()-1;
      if (by1<0) by1=0; 
      if (by2>=g1_get_map()->height())  by2=g1_get_map()->height()-1;

      if (bx1!=x1 || by1!=y1 || bx2!=x2 || by2!=y2)  // have bounds changed?
      {
        unoccupy_location();
        x1=bx1;  y1=by1; x2=bx2; y2=by2;

   

        radius=x2-x1;
      
        if (y2-y1>radius)
          radius=y2-y1;
        if (radius<0)
          radius=0;
        
        occupy_location();
      }
    }
  }

  if (!t_in_use && stopping)
  {
    unoccupy_location();
    request_remove();
  }
  else
    request_think();

  pf_part_emit.stop();
}
