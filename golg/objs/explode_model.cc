/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "objs/explode_model.hh"
#include "objs/model_id.hh"
#include "math/pi.hh"
#include "math/trig.hh"
#include "math/angle.hh"
#include "g1_rand.hh"
#include "resources.hh"
#include "saver.hh"
#include "g1_render.hh"
#include "r1_clip.hh"
#include "r1_api.hh"
#include "g1_tint.hh"
#include "object_definer.hh"
#include "time/profile.hh"
#include "map_man.hh"
#include "map.hh"
#include "objs/particle_emitter.hh"
#include "tick_count.hh"
#include "draw_context.hh"

i4_profile_class pf_exp_model_setup("explode_model setup");
i4_profile_class pf_exp_model_think("explode_model think");
i4_profile_class pf_exp_model_draw("explode_model draw");

g1_object_definer<g1_explode_model_class>
g1_explode_model_def("explode_model");

inline void check_on_map(const i4_3d_vector &v)
{
  int vx=i4_f_to_i(v.x), vy=i4_f_to_i(v.y);
  if (vx<0 || vy<0 || vx>=g1_get_map()->width() || vy>=g1_get_map()->height())
    i4_error("off map");
}



g1_explode_model_class::g1_explode_model_class(g1_object_type id,
                                               g1_loader_class *fp)
  : g1_object_class(id,fp)
{
  //does not save or load. too complicated and not really necessary
  vertices = 0;
  num_vertices = 0;

  faces = 0;
  num_faces = 0;
}

void g1_explode_model_class::save(g1_saver_class *fp)
{
  g1_object_class::save(fp);
}

g1_explode_model_class::~g1_explode_model_class()
{
  if (vertices)
    i4_free(vertices);
  
  if (faces)
    i4_free(faces);
}  


void g1_explode_model_class::grab_model(g1_object_class *o, i4_3d_vector &obj_center)
{
  g1_quad_object_class *base_m = o->draw_params.model;

  //face/vertex setup. count all the faces and vertices in base and sub-objects
    
  num_faces    = base_m->num_quad;
  num_vertices = 0;

  obj_center=i4_3d_vector(0,0,0);
  
  int i,j;
  for (i=0; i<base_m->num_quad; i++)
    num_vertices += base_m->quad[i].num_verts();

  //allocate space
  faces    = (g1_explode_face_class *)   i4_malloc(sizeof(g1_explode_face_class)   * num_faces,"");
  vertices = (g1_explode_vertex_class *)i4_malloc(sizeof(g1_explode_vertex_class)*num_vertices,"");

  //copy information. this is ugly, has to use transforms
  //to get everything into world space, etc..  
  
  i4_transform_class base_transform,mini_transform,spare_transform,*cur_transform;

  //make sure there is a transform ready for the base model
  o->calc_world_transform(g1_render.frame_ratio, &base_transform);

  cur_transform = &base_transform;      
  
  g1_quad_object_class    *m = base_m;
  g1_explode_face_class   *f = faces;
  g1_explode_vertex_class *v = vertices;
  
  sw32 vertex_count = 0;  
  sw32 k = 0;

  sw32 total_face_count=0;

  
  //  while (m)
  {
    g1_vert_class *src_verts=m->get_verts(o->draw_params.animation, o->draw_params.frame);
    
    for (i=0; i<m->num_quad; i++, f++)
    {
      memcpy(f->indices, m->quad[i].vertex_ref, sizeof(f->indices));

      i4_3d_vector center(0,0,0);
      int tv=f->num_verts();
      for (j=0; j<tv; j++)
      {
        i4_3d_vector t_vert;        
        cur_transform->transform(src_verts[m->quad[i].vertex_ref[j]].v, t_vert);

        center += t_vert;
        v[j].v          = t_vert;
        v[j].s          = m->quad[i].u[j];
        v[j].t          = m->quad[i].v[j];
        
        f->indices[j] = vertex_count++;
      }
     
      center/=(float)tv;                 // center is the average position of verts
      f->pos = f->l_pos = center;        // set 'position' of face to center location
      for (j=0; j<tv; j++)               // move points relative to center so they spind about it
        v[j].v -= center;

      v+=tv;
      
      obj_center+=center;
    }

    if (m->num_quad)
      obj_center/=(float)m->num_quad;
  }
}

void g1_explode_model_class::setup(g1_object_class *source_obj,
                                   const i4_3d_vector &center,
                                   g1_explode_params &_params)
{
  x=lx=center.x;
  y=ly=center.y;
  h=lh=center.z;

  if (x<0 || y<0 || x>=g1_get_map()->width() || y>=g1_get_map()->height())
    request_remove();
  else
  {
    i4_3d_vector c;
    grab_model(source_obj, c);
    params=_params;
    
    if (center.x==0)
    {
      x=lx=c.x;
      y=ly=c.y;
      h=lh=c.z;
    }  

      
   
    int i;
    for (i=0; i<num_faces; i++)
    {
      faces[i].vel = params.initial_vel;
      faces[i].angular_vel = faces[i].angles = faces[i].l_angles = i4_3d_vector(0,0,0);
    }
    occupy_location();
    request_think();
  }  
}

void g1_explode_model_class::think()
{
  if (params.stages[params.current_stage].ticks<=0)
  {
    params.current_stage++;
    if (params.current_stage>=params.t_stages)
    {
      unoccupy_location();
      request_remove();
      return ;
    }
  }

  params.stages[params.current_stage].ticks--;
  float force=params.stages[params.current_stage].force;
  float air_friction=params.stages[params.current_stage].air_friction;
  float f;
  i4_3d_vector d;
  
  for (int i=0; i<num_faces; i++)
  {
    g1_explode_face_class *face=faces+i;

   
    if (params.stages[params.current_stage].type==G1_APPLY_SING || i==0)
    {
      d=i4_3d_vector(face->pos.x-x, face->pos.y-y, face->pos.z-h);
      float d_len=d.length();
      d/=d_len;
    
      f=force/(d_len*d_len);
      d*=f;
      d.z-=params.gravity;
    }

    if (params.stages[params.current_stage].type==G1_APPLY_SING || (((g1_tick_counter+i)&31)==0))
    {
      
      float xr=(g1_float_rand(9)-0.5) *0.2;
      float yr=(g1_float_rand(3)-0.5) *0.2;
      float zr=(g1_float_rand(4)-0.5) *0.2;



      faces[i].angular_vel.x = xr;
      faces[i].angular_vel.y = yr;
      faces[i].angular_vel.z = zr;
    }
    

    face->vel *= air_friction;
    face->vel += d;
    face->l_pos = face->pos;

    i4_3d_vector ray=face->vel;
    
    g1_object_class *hit;
//     if (g1_get_map()->check_non_player_collision(0xff, face->pos, ray, hit))
//       face->vel.z *=-0.5;      // not accurate, but who cares :)
  
    face->pos += ray;
    
    face->l_angles = face->angles;
    face->angles += face->angular_vel;   
    i4_normalize_angle(face->angles.x);
    i4_normalize_angle(face->angles.y);
    i4_normalize_angle(face->angles.z);
  }
  request_think();
}


i4_profile_class pf_exp_model_draw_transform("explode_model_draw::transform");
i4_profile_class pf_exp_model_draw_get_ambient("explode_model_draw::get_ambient");

r1_texture_ref g1_burnt_texture("solid_black");

void fast_transform(i4_transform_class *t, const i4_3d_vector &src, r1_3d_point_class &dst);

void g1_explode_model_class::draw(g1_draw_context_class *context)
{
  pf_exp_model_draw.start();

  pf_exp_model_draw_transform.start();

  i4_transform_class out,face_trans;  

  int i,j,k;

  i4_float scale_x = g1_render.scale_x;
  i4_float scale_y = g1_render.scale_y;    
  
  r1_vert t_vertices[2048];
  r1_vert clip_buf_1[64];
  r1_vert clip_buf_2[64];

  g1_explode_vertex_class *src_v = vertices;
  r1_vert *v                     = t_vertices;

  w8 ANDCODE = 0xFF;
  w8 ORCODE  = 0;    

  g1_explode_face_class *f = faces;  

  w8 vertex_count = 0;

  
  
  g1_render.r_api->use_texture(g1_burnt_texture.get(), 0, 0);

  
  for (i=0; i<num_faces; i++,f++)
  {
    i4_3d_vector interp_rot;

    interp_rot.x = i4_interpolate_angle(f->l_angles.x, f->angles.x, g1_render.frame_ratio);
    interp_rot.y = i4_interpolate_angle(f->l_angles.y, f->angles.y, g1_render.frame_ratio);
    interp_rot.z = i4_interpolate_angle(f->l_angles.z, f->angles.z, g1_render.frame_ratio);
  
    face_trans.rotate_x_y_z(interp_rot.x,interp_rot.y,interp_rot.z,i4_T);

    face_trans.t.interpolate(f->l_pos, f->pos, g1_render.frame_ratio);
    
    out.multiply(*context->transform,face_trans);
     
    w8 face_and = 0xFF;    
    g1_explode_vertex_class *src_face_v = src_v;
    r1_vert *face_v                     = v;

    int tv=faces[i].num_verts();
    for (j=0; j<tv; j++,src_v++,v++)
    {
      fast_transform(&out, src_v->v, v->v);

      v->v.x *= scale_x;
      v->v.y *= scale_y;
      
      w8 code = r1_calc_outcode(v);
      ANDCODE &= code;
      ORCODE  |= code;
      
      face_and &= code;
      
      if (!code)
      {
        //valid point
        i4_float ooz = 1.f / v->v.z;
      
        v->px = v->v.x * ooz * g1_render.center_x + g1_render.center_x;
        v->py = v->v.y * ooz * g1_render.center_y + g1_render.center_y;
        v->w  = ooz;
      }
    }
    
    if (face_and==0)
    {
      for (j=0; j<tv; j++,src_face_v++,face_v++,vertex_count++)
      {
        face_v->r = 0;
        face_v->g = 0;
        face_v->b = 0;
        face_v->a = 1.0;
        face_v->s = 0;
        face_v->t = 0;
      }
    }
    else
      vertex_count += tv;
  }

  pf_exp_model_draw_transform.stop();

  if (ANDCODE)
  {
    pf_exp_model_draw.stop();
    return;
  }

  f = faces;

  for (i=0; i<num_faces; i++, f++)
  {
    int tv=f->num_verts();

    if (ORCODE==0)
    {
      float nearest_w = 0;

      for (j=0; j<tv; j++)
      {
        r1_vert *temp_vert = &t_vertices[f->indices[j]];
        
        float ooz = temp_vert->w;
        
        if (ooz > nearest_w)
          nearest_w=ooz;
      }
        
      g1_render.r_api->render_poly(tv, t_vertices+f->indices[0]);
    }
    else
    {
      sw32 num_poly_verts = tv;
        
      r1_vert temp_buf_1[64];
      r1_vert temp_buf_2[64];
      r1_vert *clipped_poly;
        
      clipped_poly = g1_render.r_api->clip_poly(&num_poly_verts,
                                         t_vertices,
                                         (w16 *)f->indices,
                                         temp_buf_1,
                                         temp_buf_2,
                                         R1_CLIP_NO_CALC_OUTCODE);
        
      if (clipped_poly && num_poly_verts>=3)      
      {
        float nearest_w = 0;

        r1_vert *temp_vert = clipped_poly;

        for (j=0; j<num_poly_verts; j++, temp_vert++)
        {        
          i4_float ooz = 1 / temp_vert->v.z;

          temp_vert->w  = ooz;
          temp_vert->px = temp_vert->v.x * ooz * g1_render.center_x + g1_render.center_x;
          temp_vert->py = temp_vert->v.y * ooz * g1_render.center_y + g1_render.center_y;
  
          if (ooz > nearest_w)
            nearest_w=ooz;                            
        }                  
          

        g1_render.r_api->render_poly(num_poly_verts,clipped_poly);
      } 
    }
  }
  

  pf_exp_model_draw.stop();
}  

g1_explode_params::g1_explode_params()
{
  stages[0].setup(1,0.01, G1_APPLY_WHOLE);  // apply force for 2 ticks
  stages[1].setup(3, 0,    G1_APPLY_WHOLE);  // wait 20 ticks with no force to watch results
  stages[2].setup(2, 1.0, G1_APPLY_SING);  // wait 20 ticks with no force to watch results
  stages[3].setup(10, 0,  G1_APPLY_SING);  // wait 20 ticks with no force to watch results
  t_stages=4;
  current_stage=0;
  gravity=0.02;
  initial_vel=i4_3d_vector(0,0,0);
}  
