/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "gui/divider.hh"
#include "window/style.hh"
#include "window/win_evt.hh"

void i4_divider_class::reparent(i4_image_class *draw_area, i4_parent_window_class *parent)
{
  if (parent && (w_attached && width()!=parent->width() ||
                 h_attached && height()!=parent->height()))
  {
    int nw=w_attached ? parent->width() : width();
    int nh=h_attached ? parent->height() : height();

    resize(nw,nw);
  }
                 
  i4_parent_window_class::reparent(draw_area, parent);
}

void i4_divider_class::resize(w16 new_width, w16 new_height)
{
  if (w_attached && parent)
    new_width=parent->width();

  if (h_attached && parent)
    new_height=parent->height();

  i4_parent_window_class::resize(new_width, new_height);

  int x1,y1,x2,y2;
  get_drag_area(x1,y1,x2,y2);

  if (split_up_down)
  {   
    if (w1.get())
    {
      remove_child(w1.get());
      if (y1<0) y1=0;
      w1->resize(new_width, y1);
      add_child(0,0,w1.get());
    }
    
    if (w2.get())
    {
      remove_child(w2.get());
      int nh=new_height-y2;
      if (nh<0) nh=0;

      w2->resize(w, nh);
      add_child(0,y2+1, w2.get());
    }
  }
  else
  {
    if (w1.get())
    {
      remove_child(w1.get());
      if (x1<0) x1=0;
      w1->resize(x1, new_height);
      add_child(0,0,w1.get());
    }
    
    if (w2.get())
    {
      remove_child(w2.get());
      int nw=new_width-x2;
      if (nw<0) nw=0;
      w2->resize(nw, new_height);
      add_child(x2+1,0, w2.get());
    }
  }  
}

void i4_divider_class::get_drag_area(int &x1, int &y1, int &x2, int &y2)
{
  w32 l,r,t,b;
  style->get_out_deco_size(l,t,r,b);

  if (split_up_down)
  {
    int vbh=t+b;
    y1=split_value-vbh/2;
    y2=y1+vbh-1; 
    x1=0; x2=width()-1;
  }
  else
  {
    int vbw=l+r;
    x1=split_value-vbw/2;
    x2=x1+vbw-1;
    y1=0;
    y2=height()-1;
  }
}

i4_divider_class::i4_divider_class(int w, int h,
                                   i4_bool split_up_down,
                                   int split_x_or_y,
                                   i4_window_class *window1,
                                   i4_window_class *window2,
                                   i4_graphical_style_class *style,
                                   int window1_min_size,
                                   int window2_min_size)
  : split_up_down(split_up_down),
    split_value(split_x_or_y),
    style(style),
    i4_parent_window_class(w,h),
    min1(window1_min_size),
    min2(window2_min_size)    
{
  w1=window1;
  w2=window2;

  if (w==-1)
  {
    w_attached=i4_T;
    w=0;
  }
  else
    w_attached=i4_F;

  if (h==-1)
  {
    h=0;
    h_attached=i4_T;
  }
  else
    h_attached=i4_F;


  dragging=i4_F;
  if (split_x_or_y==-1)
  {
    if (split_up_down)
      split_value=h/2;
    else
      split_value=w/2;
  }
  
  w1=window1;
  w2=window2;

  add_child(0,0, window1);
  add_child(0,0, window2);
  resize(w,h);
}


void i4_divider_class::parent_draw(i4_draw_context_class &context)
{
  int x1,y1,x2,y2;
  get_drag_area(x1,y1,x2,y2);
  local_image->add_dirty(x1,y1,x2,y2, context);
  style->draw_out_deco(local_image, x1,y1, x2,y2, i4_F, context);
}


void i4_divider_class::receive_event(i4_event *ev)
{
  switch (ev->type())
  {
    case i4_event::MOUSE_BUTTON_DOWN :
    {
      CAST_PTR(bev, i4_mouse_button_down_event_class, ev);
      if (bev->left())
      {
        int x1,y1,x2,y2;
        get_drag_area(x1,y1,x2,y2);
        if (bev->x>=x1 && bev->x<=x2 &&
            bev->y>=y1 && bev->y<=y2)
        {
          dragging=i4_T;
          i4_window_request_mouse_grab_class grab(this);
          i4_kernel.send_event(parent, &grab);
        }
        else i4_parent_window_class::receive_event(ev);
      }
      else i4_parent_window_class::receive_event(ev);
    } break;


    case i4_event::MOUSE_BUTTON_UP :
    {        
      CAST_PTR(bev, i4_mouse_button_up_event_class, ev);
      if (dragging && bev->left())
      {
        dragging=i4_F;
        i4_window_request_mouse_ungrab_class ungrab(this);
        i4_kernel.send_event(parent, &ungrab);        
      }
      else i4_parent_window_class::receive_event(ev);
    } break;

    case i4_event::MOUSE_MOVE :
    {
      CAST_PTR(mev, i4_mouse_move_event_class, ev);
      if (dragging)
      {
        if (split_up_down)
        {
          if (mev->y!=split_value && mev->y>min1 && height()-mev->y>min2)
          {
            split_value=mev->y;
            resize(width(), height());
          }
        }
        else if (mev->y!=split_value && mev->x>min1 && width()-mev->x>min2)
        {
          split_value=mev->x;
          resize(width(), height());
        }
      }
      else i4_parent_window_class::receive_event(ev);
    } break;

    default:
      i4_parent_window_class::receive_event(ev);
  }
}





