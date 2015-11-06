/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "st_edit.hh"
#include "m1_info.hh"
#include "window/window.hh"
#include "gtext_load.hh"
#include "image/image.hh"
#include "max_object.hh"
#include "window/win_evt.hh"
#include "device/kernel.hh"
#include "render.hh"
#include "app/app.hh"
#include "window/wmanager.hh"
#include "gui/text_input.hh"
#include "tmanage.hh"

#define HANDLE_SIZE   3
#define SNAP_DISTANCE 5

i4_float m1_st_edit_window_class::twidth() const
{
  return (texture)? texture->width()-0.0001 : 255.9999;
}

i4_float m1_st_edit_window_class::theight() const
{
  return (texture)? texture->height()-0.0001 : 255.9999;
}

void m1_st_edit_window_class::get_point(int poly, int num, int &x, int &y)
{
  if (m1_info.obj)
  {
    g1_quad_class *q=m1_info.obj->quad+poly;
    float tw=twidth(), th=theight();

    x=(int)(q->u[num] * tw);
    y=(int)(q->v[num] * th);
  }
}

void m1_st_edit_window_class::draw(i4_draw_context_class &context)
{
  local_image->clear(0, context);

  if (texture)
    texture->put_image(local_image, 0,0, context);
  
  if (m1_info.obj)
  {
    w32 c=0x0000ff;   //(254<<16)|(2<<8)|166;  
    int x1,y1,x2,y2;

    for (int j=0; j<m1_info.obj->num_quad; j++)
    {
      if (m1_info.obj->quad[j].get_flags(g1_quad_class::SELECTED))
      {
        int i;
        
        g1_quad_class *q=&m1_info.obj->quad[j];

        get_point(j,0,x2,y2);        
        for (i=q->num_verts()-1; i>=0; i--)
        {
          get_point(j,i,x1,y1);
          local_image->line(x1,y1,x2,y2,  c, context);
          x2 = x1; y2 = y1;
        }

        for (i=0; i<q->num_verts(); i++)
        {
          int x,y;   
          get_point(j,i,x,y);
          if (m1_info.obj->get_poly_vert_flag(j,1<<i))
            local_image->bar(x-2,y-2,x+2,y+2, 0xffff00, context);
          else
            local_image->bar(x-2,y-2,x+2,y+2, 0x808080, context);
        }
      }
    }
    if (preselect_x>=0)
      local_image->bar(preselect_x-2,preselect_y-2,preselect_x+2,preselect_y+2, 0xff00ff, context);
  }
}

void m1_st_edit_window_class::drag_points(int xc, int yc)
{
  m1_poly_object_class *obj=m1_info.obj;
  if (!obj) return;
  
  float tw=twidth(), th=theight();
  float u_change=xc/tw, v_change=yc/th;
  i4_bool change=i4_F;
        
  for (int i=0; i<m1_info.obj->num_quad; i++)
  {
    if (obj->quad[i].get_flags(g1_quad_class::SELECTED))
    {    
      for (int j=0; j<obj->quad[i].num_verts(); j++)
      {
        if (obj->get_poly_vert_flag(i, 1<<j))
        {
          obj->quad[i].u[j]+=u_change;
          if (obj->quad[i].u[j]<0) obj->quad[i].u[j]=0;
          if (obj->quad[i].u[j]>1) obj->quad[i].u[j]=1;

          obj->quad[i].v[j]+=v_change;
          if (obj->quad[i].v[j]<0) obj->quad[i].v[j]=0;
          if (obj->quad[i].v[j]>1) obj->quad[i].v[j]=1;
          change=i4_T;
        }
      }
    }
  }

  if (change)
  {
    obj->calc_texture_scales();
    request_redraw(i4_F);
    if (m1_render_window.get())
      m1_render_window->request_redraw(i4_F);
  }
}

i4_bool m1_st_edit_window_class::verts_are_selected()
{
  if (!m1_info.obj)
    return 0;

  for (int j=0; j<m1_info.obj->num_quad; j++)
    if (m1_info.obj->quad[j].get_flags(g1_quad_class::SELECTED))
    {
      for (int i=0; i<m1_info.obj->quad[j].num_verts(); i++)
        if (m1_info.obj->get_poly_vert_flag(j,1<<i))
          return i4_T;
    }

  return i4_F;
}

void m1_st_edit_window_class::select_point(int point)
{
  if (!m1_info.obj)
    return;

  if (point<0)
    return;

  m1_poly_object_class *obj=m1_info.obj;
  
  if (!i4_current_app->get_window_manager()->shift_pressed())
  {
    for (int j=0; j<obj->num_quad; j++)
      for (int i=0; i<4; i++)
        obj->set_poly_vert_flag(j, 1<<i, 0);   
  }

  for (int j=0; j<obj->num_quad; j++)
  {
    g1_quad_class *q=obj->quad+j;
    if (q->get_flags(g1_quad_class::SELECTED))
      for (int i=0; i<q->num_verts(); i++)
        if (q->vertex_ref[i] == point)
          obj->set_poly_vert_flag(j, 1<<i, 1);
  }

  request_redraw();
}

void m1_st_edit_window_class::change_current_verts()
{
  if (!m1_info.obj)
    return;

  int old_px = preselect_x;
  int old_py = preselect_y;

  preselect_x = -1;

  if (m1_info.preselect_point>=0)
    for (int j=0; j<m1_info.obj->num_quad; j++)
      if (m1_info.obj->quad[j].get_flags(g1_quad_class::SELECTED))
      {
        for (int i=0; i<m1_info.obj->quad[j].num_verts(); i++)
          if (m1_info.obj->quad[j].vertex_ref[i] == m1_info.preselect_point)
            get_point(j,i,preselect_x,preselect_y);
      }
  
  if (preselect_x!=old_px || preselect_y!=old_py)
    request_redraw();
}

void m1_st_edit_window_class::change_current_texture(i4_const_str new_name)
{
  if (m1_info.obj)
  {
    i4_file_class *fp=i4_open(new_name);
    if (!fp) return;
    delete fp;
    int i;

    for (i=0; i<m1_info.obj->num_quad; i++)
      if (m1_info.obj->quad[i].get_flags(g1_quad_class::SELECTED))
      {
        delete m1_info.obj->texture_names[i];
        m1_info.obj->texture_names[i]=new i4_str(new_name);
      }

    m1_info.texture_list_changed();
  }
}


void m1_st_edit_window_class::receive_event(i4_event *ev)
{
  int pre_grab = grab;

  m1_poly_object_class *obj=m1_info.obj;
  if (!obj)
    return ;

  switch (ev->type())
  {
    case i4_event::WINDOW_MESSAGE:
    {
      CAST_PTR(wev, i4_window_message_class, ev);
      if (wev->sub_type==i4_window_message_class::GOT_DROP)
        i4_kernel.send_event(tname_edit, ev);
    } break;

    case i4_event::OBJECT_MESSAGE :
    {
      CAST_PTR(tc, i4_text_change_notify_event, ev);
      if (tc->object==tname_edit && tc->new_text && m1_info.obj)
      {
        i4_file_class *fp=i4_open(*tc->new_text);
        if (!fp) return;
        delete fp;
        int i;

        for (i=0; i<m1_info.obj->num_quad; i++)
          if (m1_info.obj->quad[i].get_flags(g1_quad_class::SELECTED))
            m1_info.obj->texture_names[i]=new i4_str(*tc->new_text);

        m1_info.texture_list_changed();
      }


    } break;

    case i4_event::MOUSE_BUTTON_DOWN :
    {      
      CAST_PTR(bev, i4_mouse_button_down_event_class, ev);

      if (bev->left())   grab |= LEFT;
      if (bev->right())  grab |= RIGHT;
      if (bev->center()) grab |= MIDDLE;

      i4_window_request_key_grab_class kgrab(this);
      i4_kernel.send_event(parent, &kgrab);

      i4_bool clear_old=i4_T;
      int sel_poly=-1, sel_vert=-1;

      if (bev->left() && obj)
      {
        for (int j=0; j<obj->num_quad; j++)
        {
          g1_quad_class *q=obj->quad+j;
          if (q->get_flags(g1_quad_class::SELECTED))
          {
            for (int i=0; i<q->num_verts(); i++)
            {
              int x,y;
              get_point(j,i,x,y);
              if (abs(bev->x-x)<HANDLE_SIZE && abs(bev->y-y)<HANDLE_SIZE)
              {
                if (obj->get_poly_vert_flag(j, 1<<i))
                  clear_old=i4_F;
                else
                  obj->set_poly_vert_flag(j, 1<<i, 1);

                sel_poly=j;
                sel_vert=i;
              }
            }
          }
        }
      }

      if (clear_old && !i4_current_app->get_window_manager()->shift_pressed())
      {
        for (int j=0; j<obj->num_quad; j++)
          for (int i=0; i<4; i++)
            obj->set_poly_vert_flag(j, 1<<i, 0);   

        if (sel_poly!=-1)
          obj->set_poly_vert_flag(sel_poly, 1<<sel_vert, 1);
      }

      request_redraw(i4_T);
    } break;

    case i4_event::MOUSE_BUTTON_UP :
    {
      CAST_PTR(bev, i4_mouse_button_up_event_class, ev);

      if (bev->left())   grab &= ~LEFT;
      if (bev->right())  grab &= ~RIGHT;
      if (bev->center()) grab &= ~MIDDLE;
    } break;
    
    case i4_event::MOUSE_MOVE :
    {
      CAST_PTR(mev, i4_mouse_move_event_class, ev);

      int old_px = preselect_x, old_py = preselect_y;

      preselect_x = -1;

      if (m1_info.obj)
      {
        if (grab && verts_are_selected())
        {
          int snap_x = mev->x, snap_y = mev->y;

          for (int j=0; j<m1_info.obj->num_quad; j++)
          {
            g1_quad_class *q=m1_info.obj->quad+j;
            if (q->get_flags(g1_quad_class::SELECTED))
            {
              for (int i=0; i<q->num_verts(); i++)
              {
                if (!m1_info.obj->get_poly_vert_flag(j,1<<i))
                {
                  int x,y;
                  get_point(j,i,x,y);
                  if (abs(mev->x-x)<SNAP_DISTANCE && abs(mev->y-y)<SNAP_DISTANCE)
                  { 
                    snap_x = x; 
                    snap_y = y; 
                  }
                }
              }
            }
          }

          drag_points(snap_x+snap_off_x-mev->lx, snap_y+snap_off_y-mev->ly);

          snap_off_x = mev->x - snap_x;
          snap_off_y = mev->y - snap_y;
        }
        else
        {
          for (int j=0; j<m1_info.obj->num_quad; j++)
          {
            g1_quad_class *q=m1_info.obj->quad+j;
            if (q->get_flags(g1_quad_class::SELECTED))
            {
              for (int i=0; i<q->num_verts(); i++)
              {
                int x,y;
                get_point(j,i,x,y);
                if (abs(mev->x-x)<HANDLE_SIZE && abs(mev->y-y)<HANDLE_SIZE)
                { 
                  preselect_x = x; 
                  preselect_y = y; 
                  snap_off_x = mev->x - preselect_x;
                  snap_off_y = mev->y - preselect_y;
                }
              }
            }
          }
        }

        if (old_px!=preselect_x || old_py!=preselect_y)
          request_redraw();
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
}



void m1_st_edit_window_class::edit_poly_changed()
{
  if (texture)    
    delete texture;
  texture=0;

  if (m1_info.obj)
  {
    m1_poly_object_class *obj=m1_info.obj;

    int sel_poly=-1;
    for (int j=0; j<obj->num_quad; j++)
      if (obj->quad[j].get_flags(g1_quad_class::SELECTED))
      {
        if (sel_poly==-1)
          sel_poly=j;
        else if (sel_poly>=0)
          if (!(*obj->texture_names[sel_poly] == *obj->texture_names[j]))
            sel_poly=-2;
      }

    if (sel_poly==-1)
      tname_edit->change_text("<None Selected>");
    else if (sel_poly==-2)
      tname_edit->change_text("<Multiple Selected>");
    else
    {
      i4_image_class *im[10];
      int t=r1_load_gtext(r1_get_texture_id(*obj->texture_names[sel_poly]), im);
      
      for (int i=1; i<t; i++)
        delete im[i];
      
      if (t)
        texture=im[0];

      if (texture)
        tname_edit->change_text(*obj->texture_names[sel_poly]);
      else
        tname_edit->change_text("<Untextured>");
    }
  }

  request_redraw(i4_F);
}


m1_st_edit_window_class::m1_st_edit_window_class(w16 w, w16 h,
                                                 i4_text_input_class *tname_edit)
  : i4_window_class(w,h),
    tname_edit(tname_edit)
{ 
  texture=0; 
  dragging=i4_F;
  preselect_x = -1;
  grab=0;
}

i4_event_handler_reference_class<m1_st_edit_window_class> m1_st_edit;



