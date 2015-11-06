/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "input.hh"
#include "device/kernel.hh"
#include "resources.hh"
#include "human.hh"
#include "g1_speed.hh"
#include "error/alert.hh"
#include "controller.hh"
#include "device/key_man.hh"
#include "statistics.hh"
#include "app/app.hh"
#include "window/win_evt.hh"
#include "app/app.hh"
#include "lisp/lisp.hh"


enum
{ 
  G1_MAIN_MENU=g1_input_class::NUM_STANK_CONTROLS,
  G1_TOGGLE_SHADOWS,
  G1_TOGGLE_STATS,
  G1_TOGGLE_TEXTURE_LOADING,
  G1_SHOW_HELP,
  G1_NUM_COMMANDS
};

g1_input_class g1_input;

i4_bool g1_disable_all_drawing = i4_F;

//(OLI)
void (*g1_reload_dll)(void) = NULL;

void g1_input_class::acknowledge()
{
  for (w32 i=0; i<NUM_STANK_CONTROLS; i++)
    if (in[i]==PRESSED_AND_RELEASED)
      in[i]=NOT_PRESSED;
}

void g1_input_class::init()
{
  // default to invalid keys (negative)
  memset(keys, 0xff, sizeof(keys));
  mouse_scroll_flags=0;

  i4_key_man.load(i4gets("keys_res_file"));

  matchup=new i4_key_matchup_class; 

  char *cmds[]={"Move Left",            // LEFT =0,
                "Move Right",           // RIGHT, 
                "Move Forward",         // UP,
                "Move Backward",        // DOWN,
                "Main Gun",             // BUTTON_1,
                "Missiles",             // BUTTON_2,
                "Chain Gun",            // BUTTON_3,
                "Strafe Left",          // STRAFE_LEFT,
                "Strafe Right",         // STRAFE_RIGHT,
                "Strafe Mode",          // STRAFE_MODIFIER,
                "Zoom In",
                "Zoom Out",
                "Zoom Up",
                "Zoom Down",
                "Zoom Left",
                "Zoom Right",
                "Main Menu",
                "Toggle Shadows",
                "Toggle Stats",
                "Toggle Texture Loading",
                "Help",

                0};

                
  for (int i=0; cmds[i]; i++)
    matchup->add(cmds[i], i);


  memset(in,0,sizeof(in));
  memset(time_qued, 0, sizeof(time_qued));
  memset(cheat_code_buffer,0,CHEATCODE_BUFFERSIZE);

  esc=0;
  cont=i4_F;
}

void g1_input_class::uninit()
{
  i4_key_man.uninit();
  delete matchup;
  matchup=0;
}


void g1_input_class::que_keys(i4_time_class cur_time)
{
  for (int i=0; i<NUM_STANK_CONTROLS; i++)
  {
    if (in[i]==PRESSED)
    {
      sw32 add=cur_time.milli_diff(time_down[i]);
      if (add<0)
        // haven't reached the keypress yet
        add=0;
      if (add>1000/G1_HZ)
        add=1000/G1_HZ;    

      time_qued[i]+=add;
      time_down[i].add_milli(add);
    }
  }
}

w32 g1_input_class::deque_time(in_type t)
{
  w32 ret = time_qued[t];
   if (ret>1000/G1_HZ)
    ret=1000/G1_HZ;    
  time_qued[t]-=ret;
  return ret;
}

void g1_input_class::receive_event(i4_event *ev)
{
  if (ev->type()==i4_event::DO_COMMAND)
  {
    CAST_PTR(kev, i4_do_command_event_class, ev);

    int com=matchup->remap(kev->command_id);
    
    if (com>=0 && com<NUM_STANK_CONTROLS)
    {
      if (in[com]!= PRESSED)
      {
        time_down[com]=kev->time;    
        in[com]=PRESSED;
      }
    }
    else if (memcmp(kev->command, "Build ", 6)==0)
      g1_human->build_unit(g1_get_object_type(kev->command+6));         // build an object
    else 
    {
      switch (com)
      {
        case G1_MAIN_MENU : esc=i4_T; break;
        case G1_TOGGLE_STATS :  g1_stat_counter.show();   break;
        case G1_SHOW_HELP : i4_kernel.send_event(i4_current_app, ev); break;
      }
    }  
  }
  else if (ev->type()==i4_event::END_COMMAND)
  {
    CAST_PTR(kev,i4_end_command_event_class,ev);

    int com=matchup->remap(kev->command_id);

    if (com>=0 && com<NUM_STANK_CONTROLS)
    {
      sw32 add=kev->time.milli_diff(time_down[com]);
      if (add>0)
      {
        time_qued[com]+=add;
        time_down[com].add_milli(add);
      }
      
      if (in[com]==PRESSED)
        in[com]=PRESSED_AND_RELEASED;
      else
        in[com]=NOT_PRESSED;
    }
  }
  else if (ev->type()==i4_event::KEY_PRESS)
  {
    key_pressed=i4_T;   // any key sets continue to true


    CAST_PTR(kev,i4_key_press_event_class,ev);
    memmove(cheat_code_buffer+1,cheat_code_buffer,CHEATCODE_BUFFERSIZE-1);
    cheat_code_buffer[0] = kev->key_code;
  }
  else if (ev->type()==i4_event::MOUSE_MOVE)
  {
    CAST_PTR(mev,i4_mouse_move_event_class,ev);
    
    mouse_scroll_flags=0;

#if 0
    if (mev->x<g1_resources.mouse_scroll.x1)
      mouse_scroll_flags|=MOUSE_SCROLL_LEFT;
    if (mev->x>i4_current_app->get_root_window()->width()-g1_resources.mouse_scroll.x2)
      mouse_scroll_flags|=MOUSE_SCROLL_RIGHT;

    if (mev->y>i4_current_app->get_root_window()->height()-g1_resources.mouse_scroll.y2)
      mouse_scroll_flags|=MOUSE_SCROLL_DOWN;

    if (mev->y<g1_resources.mouse_scroll.y1)
      mouse_scroll_flags|=MOUSE_SCROLL_UP;
#endif
  } else if (ev->type()==i4_event::WINDOW_MESSAGE)
  {
    CAST_PTR(wev, i4_window_message_class, ev);
    if (wev->sub_type==i4_window_message_class::LOST_MOUSE_FOCUS)
      mouse_scroll_flags=0;
  }

}

