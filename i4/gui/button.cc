/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "gui/button.hh"
#include "device/kernel.hh"
#include "device/keys.hh"
#include "window/win_evt.hh"

i4_button_class::i4_button_class(const i4_const_str *idle_context_help,  // can be null
                                 i4_window_class *child, 
                                 i4_graphical_style_class *hint,
                                 i4_event_reaction_class *press,
                                 i4_event_reaction_class *depress,
                                 i4_event_reaction_class *activate,
                                 i4_event_reaction_class *deactivate) :
  i4_menu_item_class(idle_context_help,
                     hint,
                     child ? child->width()+4 : 0,
                     child ? child->height()+4 : 0,
                     press,depress,
                     activate,
                     deactivate),
  decore(child)
{
  popup=i4_F;
  state=WAIT_CLICK;
  repeat_down=i4_F;
  repeat_event=0;
  grabbing=i4_F;
  if (child)
    add_child(2,2,child);
}

i4_button_class::~i4_button_class()
{
  if (decore)
  {
    remove_child(decore);
    delete decore;

    if (grabbing)
    {
      I4_ASSERT(parent, "~button/grab/and no parent");
        
      i4_window_request_mouse_ungrab_class ungrab(this);
      i4_kernel.send_event(parent,&ungrab);
    }
  }
}

void i4_button_class::reparent(i4_image_class *draw_area, i4_parent_window_class *par)
{
  if (grabbing && !draw_area)
  {
    grabbing=i4_F;
    i4_window_request_mouse_ungrab_class ungrab(this);
    i4_kernel.send_event(parent,&ungrab);
  }
  i4_menu_item_class::reparent(draw_area, par);
}


void i4_button_class::receive_event(i4_event *ev)
{
  if (!disabled)
  {
    if (ev->type()==i4_event::OBJECT_MESSAGE)
    {
      CAST_PTR(oev,i4_object_message_event_class,ev);
      if (oev->object==this)
      {
        i4_kernel.not_idle();
        
        if (repeat_event)
          send_event(repeat_event, PRESSED);
        else
          send_event(send.press, PRESSED);

        time_id=i4_time_dev.request_event(this, 
                                          new i4_object_message_event_class(this),
                                          hint->time_hint->button_repeat);
        state=WAIT_REPEAT;

      } else i4_menu_item_class::receive_event(ev);
    }
    else if (ev->type()==i4_event::MOUSE_BUTTON_DOWN)
    {
      CAST_PTR(b,i4_mouse_button_down_event_class,ev);
      if (b->but==i4_mouse_button_down_event_class::LEFT)
      {        
        if (pressed)
        {
          do_depress();

          send_event(send.depress, DEPRESSED);
        }
        else 
        {
          do_press();
          send_event(send.press, PRESSED);

          if (!grabbing)
          {
            i4_window_request_mouse_grab_class grab(this);
            i4_kernel.send_event(parent,&grab);
            grabbing=i4_T;
          }
         
          if (repeat_down)
          {         
            i4_kernel.not_idle();
            time_id=i4_time_dev.request_event(this, 
                                              new i4_object_message_event_class(this),
                                              hint->time_hint->button_delay);
            state=WAIT_DELAY;
          }

        }
      }
    } else if (ev->type()==i4_event::MOUSE_BUTTON_UP)
    {
      CAST_PTR(b,i4_mouse_button_down_event_class,ev);
      if (b->but==i4_mouse_button_down_event_class::LEFT)
      {  
        if (grabbing)
        {
          grabbing=i4_F;
          i4_window_request_mouse_ungrab_class ungrab(this);
          i4_kernel.send_event(parent,&ungrab);
        }
      
        if (state==WAIT_DELAY || state==WAIT_REPEAT)
        {
          i4_time_dev.cancel_event(time_id);
          state=WAIT_CLICK;
        }

        if (pressed && popup)
        {
          do_depress();
          send_event(send.depress, DEPRESSED);
        }

      }
    }
    else if (ev->type()==i4_event::KEY_PRESS)
    {
      CAST_PTR(k,i4_key_press_event_class,ev);
      if (k->key==I4_ENTER)
      {
        if (pressed)
        {
          do_depress();
          send_event(send.depress, DEPRESSED);

        }
        else 
        {
          do_press();
          send_event(send.press, PRESSED);

        }
      } else if (k->key==I4_TAB)
      {
        i4_window_message_class t(i4_window_message_class::REQUEST_NEXT_KEY_FOCUS,this);
        i4_kernel.send_event(parent, &t);
      }
      else if (k->key==I4_LEFT)
      {
        i4_window_message_class l(i4_window_message_class::REQUEST_LEFT_KEY_FOCUS,this);
        i4_kernel.send_event(parent, &l);
      }
      else if (k->key==I4_RIGHT)
      {
        i4_window_message_class r(i4_window_message_class::REQUEST_RIGHT_KEY_FOCUS,this);
        i4_kernel.send_event(parent, &r);
      }
      else if (k->key==I4_UP)
      {
        i4_window_message_class u(i4_window_message_class::REQUEST_UP_KEY_FOCUS,this);
        i4_kernel.send_event(parent, &u);
      }
      else if (k->key==I4_DOWN)
      {
        i4_window_message_class d(i4_window_message_class::REQUEST_DOWN_KEY_FOCUS,this);
        i4_kernel.send_event(parent, &d);
      }
    } else 
      i4_menu_item_class::receive_event(ev);
  } else i4_menu_item_class::receive_event(ev);
}


void i4_button_class::parent_draw(i4_draw_context_class &context)
{
  local_image->add_dirty(0,0,width()-1,height()-1,context);
    
  i4_color_hint_class::bevel *color;
  if (active)
    color=&hint->color_hint->button.active;
  else 
    color=&hint->color_hint->button.passive;
    
  i4_color b,d,m=color->medium;
  if (!pressed)
  {
    b=color->bright;
    d=color->dark;
  } else 
  {
    b=color->dark;
    d=color->bright;
  }

  local_image->bar(0,0,width()-1,0,b,context);
  local_image->bar(0,0,0,height()-1,b,context);    
  local_image->bar(1,1,width()-2,1,m,context);
  local_image->bar(1,1,1,height()-2,m,context);

  local_image->bar(2,height()-2,width()-2,height()-2,d,context);
  local_image->bar(width()-2,2,width()-2,height()-2,d,context);
  local_image->bar(0,height()-1,width()-1,height()-1,hint->color_hint->black,context);
  local_image->bar(width()-1,0,width()-1,height()-1,hint->color_hint->black,context);


  local_image->bar(2,2,width()-3,height()-3,color->medium,context);
}


void i4_button_class::set_popup(i4_bool value)
{
  popup=value;
}

void i4_button_class::set_repeat_down(i4_bool value, i4_event_reaction_class *_repeat_event)
{
  if (!value && (state==WAIT_DELAY || state==WAIT_REPEAT))
    i4_time_dev.cancel_event(time_id);
  state=WAIT_CLICK;
  repeat_down=value;

  if (repeat_event) delete repeat_event;
  repeat_event=_repeat_event;
}
