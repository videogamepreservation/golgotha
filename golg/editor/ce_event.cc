/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "editor/contedit.hh"
#include "device/event.hh"
#include "window/win_evt.hh"
#include "device/kernel.hh"
#include "editor/e_state.hh"
#include "tile.hh"
#include "device/keys.hh"
#include "editor/mode/e_mode.hh"
#include "editor/editor.hh"


void g1_controller_edit_class::receive_event(i4_event *ev)
{
  g1_object_controller_class::receive_event(ev);

  if (!g1_map_is_loaded()) return ;
  if (!active())
  {
    if (ev->type()==i4_event::WINDOW_MESSAGE)
    {
      CAST_PTR(wev, i4_window_message_class, ev);
      if (wev->sub_type==i4_window_message_class::LOST_KEYBOARD_FOCUS)
        unfocused();
      return;
    }              
    else if (ev->type()==i4_event::MOUSE_BUTTON_DOWN)      
    {
      focused();
      g1_current_controller=this;

      request_redraw();
      setup_context();
      if (!mode->pass_through_focus_click())
        return;
    }
    else
    {
      if (ev->type()==i4_event::MOUSE_MOVE)
      {
        CAST_PTR(mev, i4_mouse_move_event_class, ev);
        last_mouse_x=mev->x;
        last_mouse_y=mev->y;

        if (abs((sw32)last_mouse_x-(sw32)g1_edit_state.context_help.mx)>15 ||
            abs((sw32)last_mouse_y-(sw32)g1_edit_state.context_help.my)>15)
          g1_edit_state.context_help.hide();

      }

      update_cursor();
      return;
    }
  }


  switch (ev->type())
  {
    case i4_event::IDLE_MESSAGE :
    {
      if (active())
        mode->idle();
    } break;


    case i4_event::MOUSE_MOVE :
    {
      CAST_PTR(mev, i4_mouse_move_event_class, ev);
      setup_context();
      mode->mouse_move(mev->x, mev->y);
      last_mouse_x=mev->x;
      last_mouse_y=mev->y;

      if (abs((sw32)last_mouse_x-(sw32)g1_edit_state.context_help.mx)>15 ||
          abs((sw32)last_mouse_y-(sw32)g1_edit_state.context_help.my)>15)
        g1_edit_state.context_help.hide();

    } break;
    
    case i4_event::MOUSE_BUTTON_DOWN :
    {
      CAST_PTR(bev, i4_mouse_button_down_event_class, ev);
      if (bev->left())
      {
        setup_context();
        mode->mouse_down();
      }
    } break;

    case i4_event::MOUSE_BUTTON_UP :
    {
      CAST_PTR(bev, i4_mouse_button_up_event_class, ev);
      if (bev->left())
      {
        setup_context();
        mode->mouse_up();
      }
    } break;

    case i4_event::KEY_PRESS :
    {
      CAST_PTR(kev, i4_key_press_event_class, ev);
      setup_context();
      mode->key_press(kev);
    } break;

    case i4_event::WINDOW_MESSAGE :
    {
      CAST_PTR(wev, i4_window_message_class, ev);
      switch (wev->sub_type)
      {
        case i4_window_message_class::LOST_KEYBOARD_FOCUS :
          unfocused();
          break;

        case i4_window_message_class::LOST_MOUSE_FOCUS :
          mode->hide_focus();
          break;

        case i4_window_message_class::GOT_MOUSE_FOCUS :
          mode->show_focus();
          break;

      }
    } break;

 
  }
}



