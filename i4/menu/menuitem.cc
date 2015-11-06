/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "menu/menuitem.hh"
#include "window/win_evt.hh"
#include "window/style.hh"

i4_menu_item_class::i4_menu_item_class(const i4_const_str *idle_context_help,
                                       i4_graphical_style_class *hint,
                                       w16 w, w16 h,
                                       i4_event_reaction_class *press,   
                                       i4_event_reaction_class *depress,
                                       i4_event_reaction_class *activate,
                                       i4_event_reaction_class *deactivate
                                       ) :
  i4_parent_window_class(w,h),
  hint(hint)
{
  if (idle_context_help)
    context_help=new i4_str(*idle_context_help);
  else
    context_help=0;
  
  send.press=press;
  send.depress=depress;
  send.activate=activate;
  send.deactivate=deactivate; 
  menu_parent=0;

  active=i4_F;
  pressed=i4_F;
  disabled=i4_F;
}

void i4_menu_item_class::send_event(i4_event_reaction_class *ev, reaction_type type)
{
  if (menu_parent)
    menu_parent->note_reaction_sent(this, ev, type);

  if (ev && ev->event && ev->handler_reference.get())
    i4_kernel.send_event(ev->handler_reference.get(), ev->event);
}

void i4_menu_item_class::receive_event(i4_event *ev)
{
  i4_parent_window_class::receive_event(ev);

  if (!disabled)
  {
    if (ev->type()==i4_event::MOUSE_BUTTON_UP || ev->type()==i4_event::MOUSE_BUTTON_DOWN)
      if (context_help_window.get())
        i4_kernel.delete_handler(context_help_window.get());


    if (ev->type()==i4_event::WINDOW_MESSAGE)
    {
      CAST_PTR(w,i4_window_message_class,ev);
      switch (w->sub_type)
      {
        case i4_window_message_class::GOT_KEYBOARD_FOCUS :
        case i4_window_message_class::GOT_MOUSE_FOCUS :
        {          
          do_activate();
          send_event(send.activate, ACTIVATED);
        } break;
        case i4_window_message_class::LOST_KEYBOARD_FOCUS :
        case i4_window_message_class::LOST_MOUSE_FOCUS :
        {
          if (context_help_window.get())
          {
            i4_window_class *w=context_help_window.get();
            context_help_window=0;
            i4_kernel.delete_handler(w);
          }

          do_deactivate();
          send_event(send.deactivate, DEACTIVATED);
        } break;

      }
    }
    else if (ev->type()==i4_event::IDLE_MESSAGE) 
      do_idle();

  }
}


void i4_menu_item_class::do_idle()
{
  if (!context_help_window.get() && context_help)
    context_help_window=hint->create_quick_context_help(x(), y()+height()+5, *context_help);
}

void i4_menu_item_class::do_activate()
{
  if (!active)
  {
    active=i4_T;
    request_redraw();
  }
}

void i4_menu_item_class::do_deactivate()
{
  if (active)
  {
    active=i4_F;
    request_redraw();
  }
}

void i4_menu_item_class::do_disable()
{
  if (!disabled)
  {
    disabled=i4_T;
    request_redraw();
  }
}

void i4_menu_item_class::do_undisable()
{
  if (disabled)
  {
    disabled=i4_F;
    request_redraw();
  }
}

void i4_menu_item_class::do_press()
{
  if (!pressed)
  {
    pressed=i4_T;
    request_redraw();
  }
}

void i4_menu_item_class::do_depress()
{
  if (pressed)
  {
    pressed=i4_F;
    request_redraw();
  }
}


i4_menu_item_class::~i4_menu_item_class()
{
  if (context_help_window.get())
    i4_kernel.delete_handler(context_help_window.get());

  if (context_help)
    delete context_help;

  if (send.press)
    delete send.press;   

  if (send.depress)
    delete send.depress;

  if (send.activate)
    delete send.activate;

  if (send.deactivate)
    delete send.deactivate;
}


