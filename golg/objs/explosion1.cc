/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "objs/explosion1.hh"
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

S1_SFX(explode_sfx, "explosion/generic.wav", S1_3D, 70);


static g1_object_type lightbulb, particle_emitter_type;
void g1_explosion_init()
{
  lightbulb = g1_get_object_type("lightbulb");
  particle_emitter_type=g1_get_object_type("particle_emitter");
}

g1_object_definer<g1_explosion1_class> 
g1_explosion_def("explosion1", 0, g1_explosion_init);

void g1_explosion1_class::create_light()
{
  g1_light_object_class *l = (g1_light_object_class *)g1_create_object(lightbulb);  
  if (l)
  {
    l->setup(x,y,h, 1.0, 0.7, 0, 1);
    l->occupy_location();
    light = l;
  }
}

void g1_explosion1_class::destroy_light()
{
  if (light.valid())
  {
    if (light->get_flag(MAP_OCCUPIED))
    {
      light->unoccupy_location();
      light->request_remove();
      light = 0;
    }
  }
}

g1_explosion1_class::g1_explosion1_class(g1_object_type id,
                                         g1_loader_class *fp)
  : g1_object_class(id, fp)
{
  light   = 0;
  emitter = 0;
  x = -1;
}

g1_explosion1_class::~g1_explosion1_class()
{
  destroy_light(); //make sure the light gets deleted
  
  if (emitter.valid())
    emitter->stop();
}

void g1_explosion1_class::think()
{
  theta += i4_pi()/64;
  roll  += i4_pi()/128;
  pitch += i4_pi()/32;


  if (exp_frame>=num_exp_frames) 
  {    
    unoccupy_location();
    
    if (emitter.get())
      emitter->stop();

    request_remove();
    destroy_light();
  }
  else
  {
    if (emitter.get())
    {
      i4_float terrain_height = g1_get_map()->terrain_height(x,y);
      
      h += zv;
      
      if (h<terrain_height)
        h = terrain_height;
    
      //      emitter->move(x,y,h);

      zv -= g1_resources.gravity;
    }

    request_think();
    exp_frame++;
  }
}

void fast_transform(i4_transform_class *t,const i4_3d_vector &src, r1_3d_point_class &dst);

void g1_explosion1_class::draw(g1_draw_context_class *context)
{
  if (model_id && exp_frame<num_exp_frames)
  {
    g1_quad_object_class *obj = g1_model_list_man.get_model(model_id);   
    
    i4_transform_class world_transform;
    calc_world_transform(g1_render.frame_ratio, &world_transform);

    i4_float growth_ratio = (exp_frame + g1_render.frame_ratio) / (float)num_exp_frames;
    
    i4_float angle = (i4_pi() * growth_ratio);

    i4_transform_class out;
    out.multiply(*context->transform, world_transform);

    i4_float texture_scale = 10*sin(angle);

    out.mult_uniscale(texture_scale);

    int i,j,k,num_vertices;
    r1_vert t_vertices[512], clip_buf_1[64], clip_buf_2[64];
    int src_quad[4];
    i4_transform_class view_transform;
  
    r1_render_api_class *r_api = g1_render.r_api;

    num_vertices            = obj->num_vertex;
    r1_vert *v              = t_vertices;
    g1_vert_class *src_vert = obj->get_verts(0,0);
    
    w8 ANDCODE = 0xFF;
    w8 ORCODE  = 0;

    g1_vert_class *src_v=src_vert;

    //get this vector before we warp the transform
    
    i4_3d_vector cam_in_object_space, light_in_object_space;
    
    out.inverse_transform(i4_3d_vector(0,0,0),cam_in_object_space);    
    
    world_transform.inverse_transform_3x3(i4_3d_vector(0,0,-1),light_in_object_space);

    view_transform.x.x = out.x.x * g1_render.scale_x;
    view_transform.x.y = out.x.y * g1_render.scale_y;
    view_transform.x.z = out.x.z;
    view_transform.y.x = out.y.x * g1_render.scale_x;
    view_transform.y.y = out.y.y * g1_render.scale_y;
    view_transform.y.z = out.y.z;
    view_transform.z.x = out.z.x * g1_render.scale_x;
    view_transform.z.y = out.z.y * g1_render.scale_y;
    view_transform.z.z = out.z.z;
    view_transform.t.x = out.t.x * g1_render.scale_x;
    view_transform.t.y = out.t.y * g1_render.scale_y;
    view_transform.t.z = out.t.z;
  
    for (i=0; i<num_vertices; i++, src_v++, v++)
    {
      fast_transform(&view_transform,src_v->v,v->v);

      w8 code = r1_calc_outcode(v);
      ANDCODE &= code;
      ORCODE  |= code;
      
      if (!code)
      {
        //valid point
        i4_float ooz = 1.f / v->v.z;
      
        v->px = v->v.x * ooz * g1_render.center_x + g1_render.center_x;
        v->py = v->v.y * ooz * g1_render.center_y + g1_render.center_y;

        v->w  = ooz;
      }
    }

    if (ANDCODE) 
      return;

    r_api->disable_texture();
    r_api->set_shading_mode(R1_COLORED_SHADING);
    //r_api->set_alpha_mode(R1_ALPHA_LINEAR);

    //set their lighting values to 1
    
    src_v = src_vert;
    v     = t_vertices;    
    
    for (i=0; i<num_vertices; i++, v++, src_v++)
    {
      //v->r = 0.75f;
      //v->b = v->g = (growth_ratio);

      v->r = v->g = v->b = 1.f;
      
/*
      i4_float dot = -src_vert[i].normal.dot(light_in_object_space);
      
      dot += 1.f;
      dot *= (0.5f * 0.75f);

      if (dot < 0.f)
        dot = 0.f;
      else
      if (dot > 1.f)
        dot = 1.f;

      v->r = v->g = v->b = 0.25f + dot;
      */
      
      v->a = 1;//-(growth_ratio*growth_ratio);
    }  
  
    g1_quad_class *q_ptr = obj->quad, *q;
    for (i=0; i<obj->num_quad; i++, q_ptr++)
    { 
      i4_3d_vector cam_to_pt = src_vert[q_ptr->vertex_ref[0]].v;
      cam_to_pt -= cam_in_object_space;

      float dot = cam_to_pt.dot(q_ptr->normal);

      if (1)//dot<0)
      {
        q=q_ptr;
          
        for (j=0; j<4; j++)      
        {
          int ref=q->vertex_ref[j];
          src_quad[j]=ref;
  
          v = &t_vertices[ref];
          v->s = q->u[j];
          v->t = q->v[j];
        }
      

        if (ORCODE==0)
        {
          r_api->render_poly(4,t_vertices,q->vertex_ref);
        }
        else
        {
          sw32 num_poly_verts = 4;
          r1_vert *clipped_poly;
          
          clipped_poly = r_api->clip_poly(&num_poly_verts,
                                           t_vertices,
                                           q->vertex_ref,
                                           clip_buf_1,
                                           clip_buf_2,
                                           R1_CLIP_NO_CALC_OUTCODE
                                           );

          if (clipped_poly && num_poly_verts>=3)      
          {
            r1_vert *temp_vert = clipped_poly;

            for (j=0; j<num_poly_verts; j++, temp_vert++)
            {        
              i4_float ooz = 1 / temp_vert->v.z;

              temp_vert->w  = ooz;
              temp_vert->px = temp_vert->v.x * ooz * g1_render.center_x + g1_render.center_x;
              temp_vert->py = temp_vert->v.y * ooz * g1_render.center_y + g1_render.center_y;
            }                  
            
            r_api->render_poly(num_poly_verts,clipped_poly);
          }
        } 
      }
    }
    
    r_api->set_alpha_mode(R1_ALPHA_DISABLED);
  }
}

static r1_texture_ref explosion2("explosions2"), bullet_particle("bullet_particle");

void g1_explosion1_class::setup(i4_float sx, i4_float sy, i4_float sh, w32 type)
{
  
  if ((sw32)sx>=0 && (sw32)sx<g1_get_map()->width() && 
      (sw32)sy>=0 && (sw32)sy<g1_get_map()->height())
  {
    x=lx=sx;
    y=ly=sy;
    h=lh=sh;

    if (emitter.get())
      emitter->stop();

    g1_particle_emitter_params p;
    p.defaults();
    
    if (type & MAKE_SPHERE)
    {
      model_id = g1_model_list_man.find_handle("testsphere");
      
      num_exp_frames = 50;
      exp_frame = 0;
    }
    else
      model_id = 0;

    i4_bool make_emitter = i4_T;

    if (type==HIT_OBJECT)
    {
      num_exp_frames = 10;
      exp_frame = 0;

      p.start_size=0.2;
      p.grow_speed=0.05;
      p.max_speed=0.02;
      p.air_friction=0.90;
      p.particle_lifetime = 10;
      p.start_size_random_range=0.1;

      p.num_create_attempts_per_tick=1;
      p.creation_probability=1;
      p.gravity=g1_resources.gravity*-0.25;

      p.texture=explosion2.get();
      p.emitter_lifetime=2;
    }
    else if (type==HIT_GROUND)
    {
      num_exp_frames = 10;
      exp_frame = 0;

      p.start_size=0.02;
      p.grow_speed=0.0005;
      p.max_speed=0.2;
      p.air_friction=0.70;
      p.particle_lifetime=15;
      p.num_create_attempts_per_tick=1;
      p.creation_probability=0.4;
      p.texture=bullet_particle.get();
    }
    else
      make_emitter=i4_F;
    
    if (make_emitter)
    {
      emitter = (g1_particle_emitter_class *)g1_create_object(particle_emitter_type);
      {
        li_class_context sub(emitter->vars);
        emitter->setup(x,y,h, p);
        emitter->think();
      }

      if (type==HIT_GROUND)
        emitter->stop();
      
      zv = 0.1 * g1_float_rand(9);
    }
    else
      zv = 0;
    

    occupy_location();
    request_think();


    explode_sfx.play(x,y,h);

    if (!(type & MAKE_SPHERE))
      create_light();
  }  
}
