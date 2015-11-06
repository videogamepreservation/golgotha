/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "render.hh"
#include "error/error.hh"
#include "time/timedev.hh"
#include "math/pi.hh"
#include "window/win_evt.hh"
#include "window/colorwin.hh"
#include "max_object.hh"
#include "gui/button.hh"
#include "gui/text.hh"
#include "loaders/load.hh"
#include "device/keys.hh"
#include "image/color.hh"
#include "error/alert.hh"
#include "obj3d.hh"
#include "tmanage.hh"
#include "loaders/dir_load.hh"
#include "saver_id.hh"
#include "r1_clip.hh"
#include "r1_win.hh"
#include "m1_info.hh"
#include "st_edit.hh"
#include "app/app.hh"
#include "window/wmanager.hh"
#include "lisp/lisp.hh"
#include "max_load.hh"

#include <stdio.h>

/* render.cc

   Defines a window which will display an object made of polygons.
   Right now it just shows it wireframe
   

*/
#include "id.hh"
#include "g1_tint.hh"

i4_image_class *background_bitmap=0;

i4_event_handler_reference_class<m1_utility_window_class> m1_render_window;

i4_float m1_utility_state_class::center_x() const { return m1_render_window->center_x; }
i4_float m1_utility_state_class::center_y() const { return m1_render_window->center_y; }
i4_float m1_utility_state_class::scale_x()  const { return m1_render_window->scale_x; }
i4_float m1_utility_state_class::scale_y()  const { return m1_render_window->scale_y; }
int m1_utility_state_class::width() const   { return m1_render_window->width(); }
int m1_utility_state_class::height() const  { return m1_render_window->height(); }
int m1_utility_state_class::mouse_x() const { return m1_render_window->mouse_x; }
int m1_utility_state_class::mouse_y() const { return m1_render_window->mouse_y; }
int m1_utility_state_class::last_x() const  { return m1_render_window->last_x; }
int m1_utility_state_class::last_y() const  { return m1_render_window->last_y; }
int m1_utility_state_class::buttons() const { return m1_render_window->grab; }

inline i4_float m1_near_clip_z() { return r1_near_clip_z+0.1; }

void m1_utility_window_class::init()
{
  calc_params();
}

void m1_utility_window_class::set_object(const i4_const_str &filename)
{
  m1_info.obj=m1_load_model(filename, api->get_tmanager());

  if (m1_st_edit.get())
    m1_st_edit->edit_poly_changed();

  if (get_obj())
    i4_alert(i4gets("model_stats"),200, &filename, get_obj()->num_quad, get_obj()->num_vertex);

  request_redraw(i4_F);
}


m1_utility_window_class::m1_utility_window_class(w16 window_width, 
                                                 w16 window_height,
                                                 r1_render_api_class *api,
                                                 i4_window_manager_class *wm,
                                                 i4_float theta,
                                                 i4_float phi,
                                                 i4_float dist)

  : i4_parent_window_class(window_width,window_height), 
    api(api),
    wm(wm),
    theta(theta),
    phi(phi),
    dist(dist),
    grab(0),
    state(0),
    animating(0)
{ 
  pan_x=pan_y=pan_z=0;

  draws_needed=0;

  recalc_view();

  restore_state();
}

void m1_utility_window_class::recenter()
{
  theta=0;
  phi=0;
  dist=40;
  request_redraw(i4_F);
}



void m1_utility_window_class::recalc_view()
{
  i4_transform_class tmp,tmp2,tmptmp;
  i4_float greaterdimension;

  transform.identity();
  tmp.translate(pan_x,pan_y,pan_z);
  transform.multiply(tmp);

  tmp.translate(0,0,dist);
  transform.multiply(tmp);
  tmp.rotate_x(i4_pi() - phi);
  transform.multiply(tmp);
  tmp.rotate_z(theta);
  transform.multiply(tmp);

  //  calc_params();
}


void m1_utility_window_class::pan(i4_float x, i4_float y)
{
  request_redraw(i4_F);

  pan_x-=x*dist;
  pan_y-=y*dist;

  recalc_view();
}

void m1_utility_window_class::select_poly(int poly_num)
{
  if (!m1_info.obj) return;

  if (poly_num==-1)
    return;

  g1_quad_class *q = &m1_info.obj->quad[poly_num];
  q->set_flags(g1_quad_class::SELECTED, ~q->get_flags(g1_quad_class::SELECTED));
  m1_st_edit->edit_poly_changed();

  request_redraw(i4_F);
}


void m1_utility_window_class::drop_files(int t_files, i4_str **filenames)
{
  int i;
  i4_array<i4_str *> gmods(0,32);

  // check to see if any of the files dropped are gmods
  for (i=0; i<t_files; i++)
  {
    i4_filename_struct fn;
    i4_split_path(*filenames[i], fn);
    if (strcmp(fn.extension,"gmod")==0)
      gmods.add(filenames[i]);
  }

  if (gmods.size())
  {
    for (i=0; i<m1_info.models.size(); i++)
      delete m1_info.models[i];

    m1_info.models.clear();

    for (i=0; i<gmods.size(); i++)
      m1_info.models.add(new i4_str(*gmods[i]));

    set_object(*m1_info.models[0]);
  }

}

void m1_utility_window_class::receive_event(i4_event *ev)
//{{{
{
  int pre_grab = grab;

  switch (ev->type())
  {
    case i4_event::WINDOW_MESSAGE:
    {
      CAST_PTR(wev, i4_window_message_class, ev);
      if (wev->sub_type==i4_window_message_class::GOT_DROP)
      {
        CAST_PTR(dev, i4_window_got_drop_class, ev);
        if (dev->drag_info.drag_object_type==i4_drag_info_struct::FILENAMES)
          drop_files(dev->drag_info.t_filenames, dev->drag_info.filenames);
      }
    } break;

    case i4_event::MOUSE_BUTTON_DOWN:
    {
      CAST_PTR(bev, i4_mouse_button_down_event_class, ev);

      if (bev->but == i4_mouse_button_down_event_class::LEFT)
        grab |= LEFT_BUTTON;
      if (bev->but == i4_mouse_button_down_event_class::RIGHT)
        grab |= RIGHT_BUTTON;
      if (bev->but == i4_mouse_button_down_event_class::CENTER)
        grab |= MIDDLE_BUTTON;

      i4_window_request_key_grab_class kgrab(this);
      i4_kernel.send_event(parent, &kgrab);

      state->mouse_down();
    } break;
      
    case i4_event::MOUSE_BUTTON_UP:
    {
      CAST_PTR(bev, i4_mouse_button_up_event_class, ev);

      if (bev->but == i4_mouse_button_down_event_class::LEFT)
        grab &= ~LEFT_BUTTON;
      if (bev->but == i4_mouse_button_down_event_class::RIGHT)
        grab &= ~RIGHT_BUTTON;
      if (bev->but == i4_mouse_button_down_event_class::CENTER)
        grab &= ~MIDDLE_BUTTON;

      state->mouse_up();
    } break;
      
    case i4_event::MOUSE_MOVE:
    {
      i4_window_class::receive_event(ev);

      int do_redraw = 0;

      if (!grab)
      {
        do_redraw = find_hit(mouse_x,mouse_y);
        if (do_redraw)
          m1_st_edit->change_current_verts();
      }
      else
        do_redraw = state->mouse_drag();

      if (do_redraw)
      {
        recalc_view();
        request_redraw(i4_F);
        wm->root_draw();
      }

      last_x = mouse_x;
      last_y = mouse_y;

    } break;
      
    case i4_event::KEY_PRESS:
    {
      CAST_PTR(kev, i4_key_press_event_class, ev);

      switch (kev->key) 
      {
        case I4_LEFT  : pan(-0.05,0); break;
        case I4_RIGHT : pan(0.05,0); break;
        case I4_UP    : pan(0,-0.05); break;
        case I4_DOWN  : pan(0,0.05); break;

        case 'z':
        case 'Z': grab |= LEFT_KEY;   state->mouse_down(); break;
        case 'x':
        case 'X': grab |= MIDDLE_KEY; state->mouse_down(); break;
        case 'c':
        case 'C': grab |= RIGHT_KEY;  state->mouse_down(); break;
      }
    } break;
      
    case i4_event::KEY_RELEASE:
    {
      CAST_PTR(kev, i4_key_release_event_class, ev);

      switch (kev->key) 
      {
        case 'z':
        case 'Z': grab &= ~LEFT_KEY;   state->mouse_up(); break;
        case 'x':
        case 'X': grab &= ~MIDDLE_KEY; state->mouse_up(); break;
        case 'c':
        case 'C': grab &= ~RIGHT_KEY;  state->mouse_up(); break;
      }
    } break;
  }

  if (!pre_grab && grab)
  {
    i4_window_request_mouse_grab_class grab_ev(this);
    i4_kernel.send_event(parent,&grab_ev);
  }
  if (pre_grab && !grab)
  {
    i4_window_request_mouse_ungrab_class grab_ev(this);
    i4_kernel.send_event(parent,&grab_ev);
  }

  i4_parent_window_class::receive_event(ev);
}
//}}}

i4_bool m1_utility_window_class::project_point(const i4_3d_point_class &p, r1_vert &v)
//{{{
{
  i4_3d_vector &temp_v = (i4_3d_vector &)v.v;
  transform.transform(p,temp_v);
  v.v.x *= scale_x;
  v.v.y *= scale_y;

  if (v.v.z>0.001)
  {
    float ooz = r1_ooz(v.v.z);
    
    v.px = v.v.x * ooz * center_x + center_x;
    v.py = v.v.y * ooz * center_y + center_y;

    v.w = ooz;
    return i4_T;

  }
  return i4_F;
}
//}}}

void m1_utility_window_class::draw_3d_text(i4_3d_point_class p,
                                           const i4_const_str &str,
                                           w32 color,
                                           i4_draw_context_class &context)
{

  r1_vert v;
  if (project_point(p, v))
  {
    int x=(int)(v.px-m1_info.r_font->width(str)/2);  

    m1_info.r_api->set_constant_color(0);
    m1_info.r_font->set_color(0);
    m1_info.r_font->put_string(local_image, x+1, v.py+1,  str, context);

    m1_info.r_api->set_constant_color(0xffffff);
    m1_info.r_font->set_color(color);
    m1_info.r_font->put_string(local_image, x, v.py,  str, context);
  }

}

void m1_utility_window_class::draw_3d_line(i4_3d_point_class p1,
                                           i4_3d_point_class p2,
                                           i4_color color,
                                           i4_bool on_top)
//{{{
{
  r1_vert v[2];

  project_point(p1, v[0]);
  project_point(p2, v[1]);

  api->set_shading_mode(R1_CONSTANT_SHADING);
  api->set_constant_color(color);
  api->set_alpha_mode(R1_ALPHA_DISABLED);
  api->disable_texture();

  r1_clip_render_lines(1, v, center_x, center_y, api);
  api->set_constant_color(0xffffff);
}
//}}}

void m1_utility_window_class::draw_3d_point(i4_3d_point_class p, i4_color color, i4_bool on_top)
//{{{
{
  r1_vert v;

  project_point(p, v);
  int ix = int(v.px),iy = int(v.py);
  if (ix>0 && iy>0 && ix<center_x*2-1 && iy<center_y*2-1)
  {
    api->set_alpha_mode(R1_ALPHA_DISABLED);
    api->disable_texture();
    api->r1_render_api_class::clear_area(ix-1,iy-1,ix+1,iy+1,color,
                                         on_top ? v.v.z : m1_near_clip_z());
  }
}
//}}}

void m1_utility_window_class::draw_plane(const i4_3d_vector &u, const i4_3d_vector &v, w8 color)
//{{{
{
  if (color>0x10)
  {
    w32 col = color*0x010101;
    i4_3d_point_class zero(0,0,0), t1, t2, s1, s2;

    t1 = u;
    t1 += v;
    t1 *= -10;
    t2 = u;
    t2 -= v;
    t2 *= -10;

    s1 = t1;
    s2 = t2;
    s2.reverse();

    for (int x=-10; x<=10; x++)
    {
      draw_3d_line(t1, x?t2:zero, col);
      draw_3d_line(s1, x?s2:zero, col);
      s1 += v;
      s2 += v;
      t1 += u;
      t2 += u;
    }
  }
}
//}}}

static r1_vert t_vertices[3000], clip_buf_1[3000], clip_buf_2[3000];
static int src_quad[4], dst1[3000], dst2[3000];

void m1_utility_window_class::update_object(i4_float time) 
//{{{
{
  get_obj()->update(m1_info.time);
}
//}}}

void m1_utility_window_class::render_object(i4_draw_context_class &context)
//{{{
{
  g1_quad_object_class *obj = get_obj();
  
  int i,j,k,num_vertices;

  num_vertices            = obj->num_vertex;
  r1_vert *v              = t_vertices;
  g1_vert_class *src_vert = obj->get_verts(m1_info.current_animation, 
                                           m1_info.current_frame);
    
  w8 ANDCODE = 0xFF;
  w8 ORCODE  = 0;

  //api->disable_texture();
  //api->set_shading_mode(R1_CONSTANT_SHADING);
  //api->set_alpha_mode(R1_ALPHA_CONSTANT);
  //api->set_constant_color(0x7F000000);
  //api->set_write_mode(R1_COMPARE_W | R1_WRITE_COLOR);
  api->set_shading_mode(R1_COLORED_SHADING);
  if (!m1_info.get_flags(M1_WIREFRAME))
  {
    i4_3d_vector cam_in_object_space;
    transform.inverse_transform(i4_3d_vector(0,0,0),cam_in_object_space);

    i4_3d_vector light=i4_3d_vector(0,0.2, 0.9), light_dir;
    light.normalize();
    
    transform.transform_3x3(light, light_dir);

    for (i=0; i<num_vertices; i++, v++)
    {       
      i4_3d_vector &temp_v = (i4_3d_vector &)v->v;
      transform.transform(src_vert[i].v, temp_v);

      v->v.x *= scale_x;
      v->v.y *= scale_y;
    
      
      if (m1_info.get_flags(M1_SHADING))
      {     
        i4_float intensity = 0.5 * src_vert[i].normal.dot(light_dir) + 0.5;
        if (intensity<0) intensity=0;
        if (intensity>1) intensity=1;

        v->r = intensity;
        v->g = intensity;
        v->b = intensity;
        v->a = intensity;
      }
      else
      {
        v->r = 1;
        v->g = 1;
        v->b = 1;    
        v->a = 1;
      }

      w8 code = r1_calc_outcode(v);

      ANDCODE &= code;
      ORCODE  |= code;
    }

    if (ANDCODE) return;

    api->set_constant_color(0xffffff);
    api->set_shading_mode(R1_WHITE_SHADING);

    g1_quad_class *q = obj->quad;
    for (i=0; i<obj->num_quad; i++, q++)
    {    
      sw32 num_poly_verts = q->num_verts();

      i4_3d_vector cam_to_pt = src_vert[q->vertex_ref[0]].v;
      cam_to_pt -= cam_in_object_space;

      float dot = cam_to_pt.dot(q->normal);

      if (dot<0)
      {
        // copy in the texture coordinates      
        for (j=0; j<num_poly_verts; j++)      
        {
          int ref=q->vertex_ref[j];
          src_quad[j]=ref;

          v = &t_vertices[ref];
          v->s = q->u[j];
          v->t = q->v[j];
        }

      
        if (q->get_flags(g1_quad_class::TINT) && m1_info.current_team!=100)
          api->set_color_tint(g1_player_tint_handles[m1_info.current_team]);
        else
          api->set_color_tint(0);

        if (ORCODE==0)
        {
          float nearest_w = 0;
        
          for (j=0; j<num_poly_verts; j++)
          {        
            r1_vert *temp_vert = &t_vertices[q->vertex_ref[j]];
            
            float ooz = r1_ooz(temp_vert->v.z);
            
            if (ooz > nearest_w)
              nearest_w=ooz;
              
            temp_vert->px = temp_vert->v.x * ooz * center_x + center_x;
            temp_vert->py = temp_vert->v.y * ooz * center_y + center_y;
            temp_vert->w  = ooz;                                                  
          }
        
          i4_float twidth = nearest_w * q->texture_scale * center_x * 2;

          api->use_texture(q->material_ref, i4_f_to_i(twidth), m1_info.current_frame);

          api->render_poly(num_poly_verts,t_vertices,q->vertex_ref);
        }
        else
        {
          r1_vert temp_buf_1[64];
          r1_vert temp_buf_2[64];
          r1_vert *clipped_poly = api->clip_poly(&num_poly_verts,
                                                   t_vertices,
                                                   q->vertex_ref,
                                                   temp_buf_1,
                                                   temp_buf_2,
                                                   R1_CLIP_NO_CALC_OUTCODE);
          
          if (clipped_poly && num_poly_verts>=3)      
          {
            float nearest_w = 0;

            for (j=0; j<num_poly_verts; j++)
            {        
              float ooz = r1_ooz(clipped_poly[j].v.z);
              if (ooz > nearest_w)
                nearest_w=ooz;
              
              clipped_poly[j].px = clipped_poly[j].v.x * ooz * center_x + center_x;
              clipped_poly[j].py = clipped_poly[j].v.y * ooz * center_y + center_y;
              clipped_poly[j].w  = ooz;                  
            }

            i4_float twidth = nearest_w * q->texture_scale * center_x * 2;

            api->use_texture(q->material_ref, i4_f_to_i(twidth), m1_info.current_frame);

            api->render_poly(num_poly_verts,clipped_poly);
          }
        } 
      }
    }
  }

  // Axis display
  if (m1_info.get_flags(M1_SHOW_AXIS))
  {
    draw_3d_line (i4_3d_point_class(0,0,0), i4_3d_point_class(100,0,0), 0xd0d000);
    draw_3d_line (i4_3d_point_class(0,0,0), i4_3d_point_class(0,100,0), 0xd0d0d0);
    draw_3d_line (i4_3d_point_class(0,0,0), i4_3d_point_class(0,0,100), 0xd0d0d0);

    int x,y;
    const i4_float axis_range=0.5;
    w32 col;

    if (dist>4)
    {
      col = w32(transform.z.z*transform.z.z*0x80);
      draw_plane(i4_3d_vector(10,0,0), i4_3d_vector(0,10,0), col);
      col = w32(transform.y.z*transform.y.z*0x60);
      draw_plane(i4_3d_vector(10,0,0), i4_3d_vector(0,0,10), col);
      col = w32(transform.x.z*transform.x.z*0x60);
      draw_plane(i4_3d_vector(0,10,0), i4_3d_vector(0,0,10), col);
    }
    if (dist<15)
    {
      i4_float fade = (15 - dist)*0.2;
      fade = fade>1.0? 1.0 : fade;
      col = w32(fade*transform.z.z*transform.z.z*0x80);
      draw_plane(i4_3d_vector(1,0,0), i4_3d_vector(0,1,0), col);
      col = w32(fade*transform.y.z*transform.y.z*0x60);
      draw_plane(i4_3d_vector(1,0,0), i4_3d_vector(0,0,1), col);
      col = w32(fade*transform.x.z*transform.x.z*0x60);
      draw_plane(i4_3d_vector(0,1,0), i4_3d_vector(0,0,1), col);
    }
  }

  api->set_write_mode(R1_WRITE_COLOR);
  // draw wire stuff
  w32 color=0xffffff;

  int q;
  for (q=0; q<obj->num_quad; q++)
  {      
    if (m1_info.get_flags(M1_WIREFRAME) ||
        obj->quad[q].get_flags(g1_quad_class::SELECTED | 
                               m1_poly_object_class::INVALID_QUAD))
    {
      if (obj->quad[q].get_flags(g1_quad_class::SELECTED))
        color=0xffff00;
      else if (obj->quad[q].get_flags(m1_poly_object_class::INVALID_QUAD))
        color=0xff0000;
      else
        color=0x404040;
      
      i4_3d_point_class p1,p2;

      p2=src_vert[obj->quad[q].vertex_ref[0]].v;
      for (i=obj->quad[q].num_verts()-1; i>=0; i--)
      {
        p1=src_vert[obj->quad[q].vertex_ref[i]].v;
        draw_3d_line(p1, p2, color, i4_T);
        p2 = p1;
      }
    }

  }

  
  // preselections

  api->set_color_tint(0);

  if (m1_info.preselect_poly>=0)
  {
    i4_3d_point_class p1,p2;
    g1_quad_class *q = &obj->quad[m1_info.preselect_poly];
    p2=src_vert[q->vertex_ref[0]].v;
    color=0x0000ff;
    for (i=q->num_verts()-1; i>=0; i--)
    {
      p1=src_vert[q->vertex_ref[i]].v;
      draw_3d_line(p1, p2, color, i4_T);
      p2 = p1;
    }
  }

  if (m1_info.preselect_point>=0)
    draw_3d_point(src_vert[m1_info.preselect_point].v, 0xff00ff, i4_T);

  // mount points

  for (i=0; i<obj->num_mounts; i++)
    draw_3d_point(obj->mount[i], 0x006060, i4_T);

  for (i=0; i<obj->num_mounts; i++)
    draw_3d_point(obj->mount[i], 0x00ffff);


  if (m1_info.flags & (M1_SHOW_FACE_NUMBERS | M1_SHOW_FACE_NAMES))
  {
    for (q=0; q<obj->num_quad; q++)
    {      
      if (obj->quad[q].get_flags(g1_quad_class::SELECTED))
      {
        
        int t_verts=obj->quad[q].num_verts();
        i4_3d_vector sum=i4_3d_vector(0,0,0);

        for (i=0; i<t_verts; i++)
          sum+=src_vert[obj->quad[q].vertex_ref[i]].v;

        sum/=(float)t_verts;


        char buf[150];
        buf[0]=0;

        if (m1_info.flags & M1_SHOW_FACE_NUMBERS)
          sprintf(buf, "%d ", q);
        if ((m1_info.flags & M1_SHOW_FACE_NAMES) && m1_info.obj->texture_names[q])
          i4_os_string(*m1_info.obj->texture_names[q], buf+strlen(buf), 100);

        
        draw_3d_text(sum, buf, 0xffffff, context);

      }
    }
  }

  if (m1_info.flags & (M1_SHOW_VERT_NUMBERS))
  {
    for (int v=0; v<obj->num_vertex; v++)
    {
      int used=0;
      for (q=0; q<obj->num_quad; q++)
        if (obj->quad[q].get_flags(g1_quad_class::SELECTED))
        {        
          int t_verts=obj->quad[q].num_verts();
          for (int qv=0; qv<t_verts; qv++)
            if (obj->quad[q].vertex_ref[qv]==v)
              used=1;
        }

      if (used)
      {
        char buf[100];
        sprintf(buf,"%d", v);
        draw_3d_text(src_vert[v].v, buf, 0x00ffff, context);
      }

    }


  }

}
//}}}

void m1_utility_window_class::parent_draw(i4_draw_context_class &context)
//{{{
{

  if (m1_info.recalcing_textures)
  {
    local_image->clear(0, context);
    return;
  }

  if (background_bitmap)
  {
    for (w32 y=0; y<height(); y+=background_bitmap->height())
      for (w32 x=0; x<width(); x+=background_bitmap->width())
        background_bitmap->put_image(local_image, x,y, context);
  }
  //  else
  //    local_image->clear(15,context);
  //local_image->add_dirty(0,0,width()-1,height()-1, context);

  //  recalc_view();


  ((r1_render_window_class *)parent)->begin_render();

 
  m1_poly_object_class *obj=get_obj();

  if (obj)
  {
    obj->calc_vert_normals();
    api->default_state();

    api->set_z_range(0.01, 1000);
    //r1_far_clip_z = 1000;

    api->clear_area(0,0, width()-1, height()-1, m1_info.bg_color, 1000-1);    

    api->set_filter_mode(R1_BILINEAR_FILTERING);

    if (animating)
    {
      i4_time_class now;
      
      m1_info.time += i4_float(now.milli_diff(last_time)) / 100.0;
      last_time = now;
      update_object(m1_info.time);
      request_redraw();
    }

    render_object(context);
  } 
  else 
    local_image->clear(0,context);

  i4_window_class::draw(context);

  ((r1_render_window_class *)parent)->end_render();

  //    i4_time_dev.request_event(this,new i4_user_message_event_class(0),100);
  //  request_redraw(i4_F);
}
//}}}

i4_bool m1_utility_window_class::find_hit(int mx, int my)
//{{{
{
  i4_bool ret=i4_F;
  
  g1_quad_object_class *obj=get_obj();
  
  // select point
  if (obj)
  {
    g1_vert_class *src_vert = obj->get_verts(m1_info.current_animation, m1_info.current_frame);
    r1_vert tmp_v;
    i4_float left = mx-3,right = mx+3, top=my-3, bottom=my+3;
    int p=-1, np=m1_info.preselect_point;

    for (int j=0; j<obj->num_quad; j++)
    {
      if (obj->quad[j].get_flags(g1_quad_class::SELECTED))
        for (int i=0; i<obj->quad[j].num_verts(); i++)
        {
          int v = obj->quad[j].vertex_ref[i];
          project_point(src_vert[v].v, tmp_v);
        
          if (tmp_v.px>=left && tmp_v.px<=right && tmp_v.py>top && tmp_v.py<bottom)
            p = v;
        }
    }

    m1_info.preselect_point = p;
    if (p!=np)
      ret = i4_T;
  }
  
  int p, np=m1_info.preselect_poly;
  if (m1_info.preselect_point<0)
  {
    // find ray in camera space
    i4_float px = (i4_float(mx)-center_x)/(center_x*scale_x);
    i4_float py = (i4_float(my)-center_y)/(center_y*scale_y);
    
    i4_3d_vector point,ray;
    
    transform.inverse_transform(i4_3d_vector(0,0,0), point);
    transform.inverse_transform(i4_3d_vector(px,py,1.0), ray);
    ray -= point;
    ray *= 10000;
    
    // select poly
    if (!obj || !obj->intersect(point, ray, 
                                m1_info.current_animation, m1_info.current_frame,
                                0, &p))
      p=-1;
  }
  else
    p = -1;

  m1_info.preselect_poly=p;
  if (np!=m1_info.preselect_poly)
    ret = i4_T;

  return ret;
}
//}}}

extern m1_utility_state_class *m1_default_state;
void m1_utility_window_class::restore_state()
//{{{
{
  state = m1_default_state;
}
//}}}

//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}
