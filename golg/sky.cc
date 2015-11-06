/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "sky.hh"
#include "init/init.hh"
#include "lisp/lisp.hh"
#include "g1_render.hh"
#include "r1_api.hh"
#include "tmanage.hh"
#include "map_man.hh"
#include "map.hh"
#include "loaders/load.hh"
#include "window/window.hh"
#include "resources.hh"
#include "objs/model_id.hh"
#include "math/vector.hh"
#include "math/angle.hh"
#include "math/pi.hh"
#include "cwin_man.hh"
#include "gtext_load.hh"
#include "camera.hh"
#include "draw_context.hh"
#include "time/profile.hh"
#include "r1_clip.hh"
#include "tick_count.hh"
#include "lisp/li_class.hh"
#include "map_vars.hh"
#include "g1_tint.hh"

li_object_class_member top_cloud_layer("top_cloud_layer"), bottom_cloud_layer("bottom_cloud_layer");
li_float_class_member li_red("red"), li_green("green"), li_blue("blue"), li_alpha("alpha");

i4_profile_class pf_draw_sky("g1_draw_sky");

i4_array<i4_str *> g1_sky_list(0,16);
li_object *g1_def_skys(li_object *o, li_environment *env);
static r1_texture_ref sky_texture("cloud2");



void scale_copy(i4_image_class *src, i4_image_class *dst, int sx1, int sy1, int sx2, int sy2)
{

  int dest_x, dest_y;
  float source_x, source_y;
  float source_xstep=(float)(sx2-sx1+1) / (float)dst->width();
  float source_ystep=(float)(sy2-sy1+1) / dst->height();

  i4_draw_context_class c1(0,0, src->width()-1, src->height()-1);
  i4_draw_context_class c2(0,0, dst->width()-1, dst->height()-1);

  int dest_h=dst->height(), dest_w=dst->width();

  for (source_y=sy1, dest_y=0; dest_y<dest_h; dest_y++,  source_y+=source_ystep)
  {
    for (source_x=sx1, dest_x=0; dest_x<dest_w; dest_x++,  source_x+=source_xstep)
    {
      w32 color=src->get_pixel(i4_f_to_i(source_x), i4_f_to_i(source_y), c1);
      dst->put_pixel(dest_x, dest_y, color, c2);
    }
  }

}

class g1_sky_class : public i4_init_class
{
public:
  i4_str *current_sky_name;
  i4_image_class *sky_im;
  g1_quad_object_class *sky_model;
  i4_time_class sky_time, start_time;

  void init() 
  { 
    current_sky_name=0;
    li_add_function("def_skys", g1_def_skys);     
  }

  void reset()
  {
    for (int i=0; i<g1_sky_list.size(); i++)
      delete g1_sky_list[i];
  
    g1_sky_list.clear();

    if (current_sky_name)
      delete current_sky_name;

    current_sky_name=0;
  }

  void uninit() 
  {     
    if (current_sky_name)
      delete current_sky_name;
    current_sky_name=0;
    for (int i=0; i<g1_sky_list.size(); i++)
      delete g1_sky_list[i];
    g1_sky_list.uninit(); 
  }

  i4_file_class *find_sky_file(i4_const_str fname)
  {
    if (fname.null()) 
      return 0;

    i4_filename_struct fn;
    i4_split_path(fname, fn);
    
    // try the skys directory first
    char aname[256];
    sprintf(aname, "skys/%s.jpg", fn.filename);
    i4_file_class *fp=i4_open(aname);

    if (fp) return fp;

    
    // try the driectory the level is in
    i4_filename_struct lev_split;
    i4_split_path(g1_get_map()->get_filename(), lev_split);
   
    sprintf(aname, "%s/%s.jpg", lev_split.path, fn.filename);
    fp=i4_open(aname);


    if (fp) return fp;


    return fp;
  }


  i4_bool update(i4_const_str &sky_name, i4_window_class *w, int use_blits)
  {     
    r1_render_api_class *api=g1_render.r_api;

    if (use_blits && sky_im && (sky_im->width()!=w->width() || sky_im->height()!=w->height()*2))
    {
      delete current_sky_name;
      current_sky_name=0;
    }
    
    // see if the sky name has changed, if not then no need to try to load stuff
    if (current_sky_name && *current_sky_name==sky_name)
    {
      if (use_blits)
        return sky_im ? i4_T : i4_F;
      else
        return sky_model ? i4_T : i4_F;
    }

    // assign the new name and clean up old stuff
    if (current_sky_name)
      delete current_sky_name;
    current_sky_name=new i4_str(sky_name);


    if (sky_im)
    {
      delete sky_im;
      sky_im=0;
    }
    sky_model=0;



    if (use_blits)
    {
      i4_file_class *fp=find_sky_file(*current_sky_name);
      i4_image_class *im=0;

      if (!fp)
      {
        i4_image_class *images[10];
        w32 id=r1_get_texture_id(*current_sky_name);

        int t=r1_load_gtext(id, images);
        if (t)
        {
          im=images[0];
          for (int i=1; i<t; i++) 
            delete images[i];
        }
        else return i4_F;

      }
      
      if (!im)
        im=i4_load_image(fp);

      if (!im)
        return i4_F;
      
      
      sky_im = api->create_compatible_image(w->width(), w->height()*2);
      if (!sky_im)
      {
        delete im;
        im=0;
        return i4_F;
      }

      api->lock_image(sky_im);


      scale_copy(im, sky_im, 0,0, im->width()-1, im->height()-1);

      // add black lines for interlace mode
      if (g1_resources.render_window_expand_mode==R1_COPY_1x1_SCANLINE_SKIP)
      {
        i4_draw_context_class context(0,0, sky_im->width()-1, sky_im->height()-1);

        for (int i=0; i<=im->height()-1; i+=2)
          sky_im->bar(0, i, sky_im->width()-1, i, 0, context);
      }

      api->unlock_image(sky_im);
      delete im;
    }
    else
    {
      char sky_name[200];
      i4_os_string(*current_sky_name, sky_name,200);

      g1_model_id_type model_id=g1_model_list_man.find_handle("sky");      
      if (!model_id)
        return i4_F;

      sky_model=g1_model_list_man.get_model(model_id);

      i4_bool loaded;
      char sname[256], sname1[256], sname2[256];
      i4_os_string(*current_sky_name, sname, 256);

      sprintf(sname1, "%s1", sname);
      sprintf(sname2, "%s2", sname);


      r1_texture_handle sky1, sky2;


      sky1=api->get_tmanager()->register_texture(sname1, "sky", &loaded);
      if (!loaded)
        sky1=sky2=api->get_tmanager()->register_texture(sname, "sky", &loaded);
      else 
      {
        sky2=api->get_tmanager()->register_texture(sname2, "sky", &loaded);
        if (!loaded)
          sky2=sky1;
      }
    

      if (!sky1)    
        return i4_F;

      for (int i=0; i<sky_model->num_quad; i++)
      {
        int tinted=sky_model->quad[i].get_flags(g1_quad_class::TINT);
        
        if (!tinted)
          sky_model->quad[i].material_ref=sky1;
        else
          sky_model->quad[i].material_ref=sky2;    
      }
    }

    return i4_T;
  }

} g1_sky;


li_object *g1_def_skys(li_object *o, li_environment *env)
{
  g1_sky.reset();

  for (o=li_cdr(o,env); o; o=li_cdr(o,env))
  {
    char *name=li_string::get(li_eval(li_car(o,env), env),env)->value();
    g1_render.r_api->get_tmanager()->register_texture(name, "sky name");
    g1_sky_list.add(new i4_str(name));
  }

  return 0;
}


int force_blits=0;


void generate_poly(i4_3d_vector *points, w16 *indexes,                   
                   i4_transform_class &transform,
                   float s, float t,
                   r1_vert *v, 
                   float r, float g, float b, float a)
{
  for (int i=0; i<4; i++)
  {
    int x=indexes[i];

    i4_3d_vector p=points[x];
    transform.transform(p, v[i].v);
    v[i].v.x*=g1_render.scale_x;
    v[i].v.y*=g1_render.scale_y;
    v[i].r=r;
    v[i].g=g;
    v[i].b=b;
    v[i].a=a;
  }
    
  v[0].s=s;      v[0].t=t;
  v[1].s=s+0.5;  v[1].t=t;
  v[2].s=s+0.5;  v[2].t=t+0.5;
  v[3].s=s;      v[3].t=t+0.5;
}
                   


static float cam_z=8;
void draw_clouds(g1_camera_info_struct &current_camera,
                 i4_transform_class &transform,
                 g1_draw_context_class *context)

{ 
  int i,j;
  r1_vert v[2*2+20], *p;
  
  
  i4_3d_vector pts[4*4], *pt;

  float cloud_scale=15;

  int repeat_length = (int)(cloud_scale*2*16);
  float start_t=fmod((g1_tick_counter+g1_render.frame_ratio)/2000.0+current_camera.gy/200.0,0.5);
  float start_s=fmod(current_camera.gx*(1/200.0), 0.5);

  float r[2],g[2],b[2],a[2];

  li_class *bottom_layer=(li_class *)g1_map_vars.vars()->get(bottom_cloud_layer);

  r[0]=bottom_layer->get(li_red);
  g[0]=bottom_layer->get(li_green);
  b[0]=bottom_layer->get(li_blue);
  a[0]=bottom_layer->get(li_alpha);
   
  li_class *top_layer=(li_class *)g1_map_vars.vars()->get(top_cloud_layer);
  r[1]=top_layer->get(li_red);
  g[1]=top_layer->get(li_green);
  b[1]=top_layer->get(li_blue);
  a[1]=top_layer->get(li_alpha);

  for (int k=1; k>=0; k--)
  {

    pt=pts;

    for (float y=-1.5; y<=1.5; y+=1)
      for (float x=-1.5; x<=1.5; x+=1, pt++)
        *pt=i4_3d_vector(x * cloud_scale + current_camera.gx, 
                         y * cloud_scale + current_camera.gy, 
                         cam_z+current_camera.gz*0.9-(fabs(x)+fabs(y))*cloud_scale/8.0+k);
    

    g1_render.r_api->use_texture(sky_texture.get(), 256,0);

    for (j=0; j<3; j++)
      for (i=0; i<3; i++)
      {
        w16 indexes[4];
        indexes[0]=i+j*4;
        indexes[1]=indexes[0]+1;
        indexes[2]=indexes[0]+5;
        indexes[3]=indexes[0]+4;

        generate_poly(pts, indexes, transform, start_s, start_t, v, r[k], g[k], b[k], a[k]);

      
        if (i==0) { v[0].a=0; v[3].a=0; }
        else if (i==2) { v[1].a=0; v[2].a=0; }

        if (j==0) { v[0].a=0; v[1].a=0; }
        else if (j==2) { v[2].a=0; v[3].a=0; }


        


        r1_vert buf1[20], buf2[20], *pv;
        sw32 t_verts=4;
        w16 v_index[4]={0,1,2,3};
        pv=g1_render.r_api->clip_poly(&t_verts, v, v_index, buf1, buf2, 0);
                    
        if (t_verts)
        {

          for (int j=0; j<t_verts; j++)
          {
            r1_vert *v = &pv[j];

            float ooz = r1_ooz(v->v.z);            
          
            v->px = v->v.x * ooz * g1_render.center_x + g1_render.center_x;
            v->py = g1_render.center_y + v->v.y * ooz * g1_render.center_y;
            v->w  = ooz;                  
          }

          g1_render.r_api->render_poly(t_verts, pv);
        }
      }
    

  }
}



void g1_draw_sky(i4_window_class *window,
                 g1_camera_info_struct &current_camera,
                 i4_transform_class &transform,
                 g1_draw_context_class *context)
                
{  
  pf_draw_sky.start();

  r1_render_api_class *api=g1_render.r_api;

  int use_blits=(api->get_render_device_flags() & R1_SOFTWARE) ? 1 : 0;
  if (force_blits) use_blits=1;
  if (g1_get_map()->sky_name &&
      g1_get_map()->sky_name->null())
  {
    delete g1_get_map()->sky_name;
    g1_get_map()->sky_name=0;
  }
  
  if (g1_get_map()->sky_name && 
      g1_sky.update(*g1_get_map()->sky_name, window, use_blits))
  {

    if (use_blits)  
    {
      int window_xoff=context->context->xoff, window_yoff=context->context->yoff;

      i4_image_class *sky_im = g1_sky.sky_im;

      i4_normalize_angle(current_camera.ground_rotate);
      i4_float horz_cap = -current_camera.horizon_rotate;    

      i4_normalize_angle(horz_cap);

      if (horz_cap > 3*i4_pi()/2) horz_cap = 0;
      if (horz_cap > i4_pi()/2)   horz_cap = i4_pi()/2;
    
      sw32 h = sky_im->height() / 2;

      sw32 x_offs = i4_f_to_i(current_camera.ground_rotate * 4 * 
                              sky_im->width() / i4_2pi()) % sky_im->width();

      sw32 y_offs = i4_f_to_i(horz_cap * h/(i4_pi()/2));
    
      if (y_offs > h) y_offs = h;
      if (y_offs < 0) y_offs = 0;

      //y_offs -= (h/4);

      y_offs = h - y_offs;
      
      sw32 y2 = y_offs+h-1;
      if (y2>sky_im->height()-1) y2 = sky_im->height()-1;

      //y cffset must be even for interlaced
      if (g1_resources.render_window_expand_mode==R1_COPY_1x1_SCANLINE_SKIP)        
        y_offs = y_offs & (~1);

      if (g1_hurt_tint>0)
      {
        // solid red fill
        i4_color col =
          (i4_f_to_i(g1_hurt_tint_data[g1_hurt_tint].r*255)<<16) |
          (i4_f_to_i(g1_hurt_tint_data[g1_hurt_tint].g*255)<<8) |
          (i4_f_to_i(g1_hurt_tint_data[g1_hurt_tint].b*255));
        
        api->clear_area(0,0, window->width()-1, window->height()-1, col, g1_far_z_range());  
      }
      else
      {
        // blit sky image
        api->put_image(sky_im,
                       x_offs,
                       0,
                       0, y_offs,
                       sky_im->width()-x_offs-1, y2);
        
        if (x_offs>0)
          api->put_image(sky_im,
                         0,0,
                         sky_im->width()-x_offs, y_offs,
                         sky_im->width()-1, y2);
      }
    }
    else
    {
      i4_transform_class out, scale, trans;
      out.identity();

      trans.translate(current_camera.gx, current_camera.gy, -4);
      out=transform;
      out.multiply(trans);

      api->set_filter_mode(R1_BILINEAR_FILTERING);
      api->set_write_mode(  R1_WRITE_COLOR | R1_COMPARE_W );
      g1_render.render_object(g1_sky.sky_model,
                              &out,
                              0,
                              100,                   
                              g1_default_player,
                              0,
                              0,
                              0);

      api->set_write_mode(  R1_WRITE_COLOR | R1_WRITE_W | R1_COMPARE_W );


      draw_clouds(current_camera, transform, context);
      
      api->set_filter_mode(R1_NO_FILTERING);
    }
  }
  else
    api->clear_area(0,0, window->width()-1, window->height()-1, 0, g1_far_z_range());  

  pf_draw_sky.stop();
}

