/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "gui/scroll_bar.hh"
#include "window/colorwin.hh"
#include "image/image.hh"
#include "gui/button.hh"
#include "gui/image_win.hh"
#include "window/win_evt.hh"



inline void draw_out_deco(i4_image_class *screen,
                          i4_draw_context_class &context,
                          i4_coord x1, i4_coord y1, i4_coord x2, i4_coord y2,
                          i4_color bright, i4_color medium, i4_color dark, i4_color black)
{
  screen->add_dirty(x1,y1,x2,y2,context);

  screen->bar(x1,y1,x2-1,y1, bright, context);
  screen->bar(x1,y1,x1,y2-1, bright, context);
  screen->bar(x1,y2-1,x2-1,y2-1, dark, context);
  screen->bar(x2-1,y1+1,x2-1,y2-1, dark, context);

  screen->bar(x1,y2,x2,y2, black, context);
  screen->bar(x2,y1,x2,y2, black, context);
  screen->bar(x1+1, y1+1, x2-2, y2-2, medium, context);
  
}


class i4_scroll_button : public i4_window_class
{
  i4_bool active,dragging;
  i4_scroll_bar *buddy;

  void reparent(i4_image_class *draw_area, i4_parent_window_class *parent)
  {
    i4_window_class::reparent(draw_area, parent);
    if (parent)
      fit_parent();
  }

  i4_bool vertical() { return buddy->vertical; }
  i4_graphical_style_class *style() { return buddy->style; }



public:
  void fit_parent()
  {   
    if (vertical())
      resize(parent->width(), 
             parent->height() * buddy->total_visible_objects / buddy->total_scroll_objects);
    else
      resize(parent->width() * buddy->total_visible_objects / buddy->total_scroll_objects,
             parent->height());
  }

  char *name() { return "scroll_button"; }


  i4_scroll_button(i4_scroll_bar *buddy)
    : i4_window_class(0,0), 
      buddy(buddy)
  {
    active=i4_F;
    dragging=i4_F;
  }

  void draw(i4_draw_context_class &context)
  {
    i4_color_hint_class::bevel *color;
    if (active)
      color=&style()->color_hint->window.active;
    else 
      color=&style()->color_hint->window.passive;

    draw_out_deco(local_image,context,
                  0,0,width()-1,height()-1,
                  color->bright,
                  color->medium,
                  color->dark,
                  style()->color_hint->black);
  }

  void receive_event(i4_event *ev)
  {
    switch (ev->type())
    {
      case i4_event::WINDOW_MESSAGE :
      {
        CAST_PTR(wev,i4_window_message_class,ev);
        if (wev->sub_type==i4_window_message_class::GOT_MOUSE_FOCUS)
        {
          active=i4_T;
          request_redraw();
        }
        else if (wev->sub_type==i4_window_message_class::LOST_MOUSE_FOCUS)
        {
          active=i4_F;
          request_redraw();
        }
      } break;
      case i4_event::MOUSE_MOVE :
      {
        CAST_PTR(mev,i4_mouse_move_event_class,ev);
        if (dragging)
        {
          if (vertical())
          {
            i4_coord new_y=mev->y+y()-parent->y()-height()/2;  // convert to parent coords
            if (new_y<0)
              new_y=0;
            else if (new_y+height()>=parent->height())
              new_y=parent->height()-height();

            new_y=new_y+parent->y()-y();  // convert back to our coord system
            if (new_y!=y())
            {
              move(0,new_y);
              buddy->calc_pos();
              buddy->send_position();
            }
          }
          else
          {
            i4_coord new_x=mev->x+x()-parent->x()-width()/2;  // convert to parent coords
            if (new_x<0)
              new_x=0;
            else if (new_x+width()>=parent->width())
              new_x=parent->width()-width();

            new_x=new_x+parent->x()-x();  // convert back to our coord system
            if (new_x!=x())
            {
              move(new_x, 0); 
              buddy->calc_pos();
              buddy->send_position();
            }
          }
        }
      } break;

      case i4_event::MOUSE_BUTTON_DOWN :
      {
        CAST_PTR(bev,i4_mouse_button_down_event_class,ev);
        if (bev->but==i4_mouse_button_down_event_class::LEFT)
        {
          dragging=i4_T;
          i4_window_request_mouse_grab_class grab(this);
          i4_kernel.send_event(parent,&grab);
        }
      } break;

      case i4_event::MOUSE_BUTTON_UP :
      {
        CAST_PTR(bev,i4_mouse_button_up_event_class,ev);
        if (bev->but==i4_mouse_button_up_event_class::LEFT)
        {
          dragging=i4_F;
          i4_window_request_mouse_ungrab_class ungrab(this);
          i4_kernel.send_event(parent,&ungrab);
        }
      } break;


    }
  }

} ;


// if total items under control changes
void i4_scroll_bar::set_new_total(int total)
{
  total_scroll_objects=total;
  if (total_scroll_objects<=0)
    total_scroll_objects=1;

  scroll_but->fit_parent();
}

i4_button_class *i4_scroll_bar::create_button(i4_button_class *&b, i4_image_class *im)
{
  b=new i4_button_class(0, new i4_image_window_class(im), style);

  b->send.press=new i4_event_reaction_class(this, new i4_object_message_event_class(b));
  b->set_repeat_down(i4_T);
  b->set_popup(i4_T);
  
  return b;
}

i4_scroll_bar::i4_scroll_bar(i4_bool vertical,
                             int max_dimention_size,
                             int total_visible_objects,
                             int total_scroll_objects,    // total number of objects
                             w32 message_id,
                             i4_event_handler_class *send_to,
                             i4_graphical_style_class *style)

  : i4_parent_window_class(0,0),
    vertical(vertical),
    total_scroll_objects(total_scroll_objects),
    total_visible_objects(total_visible_objects),
    id(message_id),
    send_to(send_to),
    style(style)
{
  pos=0;
  if (vertical)
  {
    add_child(0,0, create_button(up_but, style->icon_hint->up_icon));

    create_button(down_but, style->icon_hint->down_icon);
    add_child(0, max_dimention_size-down_but->height(), down_but);

    left_but=right_but=0;

    resize(up_but->width(), max_dimention_size);
  }
  else
  {
    add_child(0,0, create_button(left_but, style->icon_hint->left_icon));

    create_button(right_but, style->icon_hint->right_icon);
    add_child(max_dimention_size-right_but->width(),0, right_but);
    up_but=down_but=0;    

    resize(max_dimention_size,  left_but->height());
  }


  int x=vertical ? 0 : left_but->width(),
    y=vertical ? up_but->height() : 0;
  int sa_w=vertical ? width() : width() - left_but->width() - right_but->height(),
    sa_h=vertical ? height() - up_but->width() - down_but->height() : height();

  // area where scroll grab button resides
  scroll_area=i4_add_color_window(this,
                                  style->color_hint->window.passive.dark,
                                  style, x,y, sa_w, sa_h);


  scroll_but=new i4_scroll_button(this);
  scroll_area->add_child(0,0,scroll_but);

}
  
void i4_scroll_bar::receive_event(i4_event *ev)
{
  if (ev->type()==i4_event::OBJECT_MESSAGE)
  {
    CAST_PTR(oev,i4_object_message_event_class,ev);
    if (oev->object==down_but || oev->object==right_but)
    {    
      if (pos<total_scroll_objects-1)
      {
        pos+=1;
        set_bar_pos(pos);
        send_position();
      }           
    } else if (oev->object==up_but || oev->object==left_but)
    {
      if (pos>0)
      {
        pos-=1;
        set_bar_pos(pos);
        send_position();
      }           
    }

  } else i4_parent_window_class::receive_event(ev);
}

void i4_scroll_bar::set_bar_pos(sw32 pos)
{ 
  sw32 reverse_pos;

  if (vertical)
  {
    if (total_scroll_objects<=1)
      reverse_pos=0;
    else
      reverse_pos=pos*(scroll_area->height()-scroll_but->height())/(total_scroll_objects-1);

    scroll_but->move(0,reverse_pos- (scroll_but->y()-scroll_area->y()));
  }
  else
  {
    if (total_scroll_objects<=1)
      reverse_pos=0;
    else
      reverse_pos=pos*(scroll_area->width()-scroll_but->width())/(total_scroll_objects-1);

    scroll_but->move(reverse_pos- (scroll_but->x()-scroll_area->x()),0);
  }
                   
}

void i4_scroll_bar::calc_pos()
{
  if (total_visible_objects>=total_scroll_objects || total_scroll_objects<=1)
    pos=0;
  else if (vertical)
    pos=(scroll_but->y()-scroll_area->y()) * (total_scroll_objects-1) / 
      (scroll_area->height()-scroll_but->height());
  else
    pos=(scroll_but->x()-scroll_area->x()) * (total_scroll_objects-1) / 
      (scroll_area->width()-scroll_but->width());

}

void i4_scroll_bar::send_position()
{
  if (send_to)
  {
    //    i4_vscroll_button *b=(i4_vscroll_button *)scroll_but;
    i4_scroll_message message(pos, total_scroll_objects, id);
    i4_kernel.send_event(send_to,&message);
  }  
}

