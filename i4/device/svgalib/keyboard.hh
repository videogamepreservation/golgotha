/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef __SVGALIB_KEYBOARD_HPP_
#define __SVGALIB_KEYBOARD_HPP_

#include "device/event.hh"
#include "device/device.hh"
#include "device/keys.hh"
#include <vgakeyboard.h>
#include <stdlib.h>

void key_handler(int scancode, int press);

class svgalib_keyboard_class : public i4_device_class
{
  int inited;
  public :
  svgalib_keyboard_class() { inited=0; }
  char *name() { return "SVGALIB keyboard"; }
  void send_key(int key, int press)
  {
    if (press)
    {
      i4_key_press_event_class ev(key);
      send_event_to_agents(&ev,FLAG_KEY_PRESS);    
    }
    else 
    {
      i4_key_release_event_class ev(key);
      send_event_to_agents(&ev,FLAG_KEY_RELEASE);    
    }
  }

  // Implement these functions for each device
  virtual i4_bool process_events()      // returns true if an event was dispatched
  {
    keyboard_update();
  }

  virtual w32 request_device(i4_event_handler_class *for_who, w32 event_types)
  {
    if (!inited)
      if (keyboard_init()==0)
      {
	keyboard_seteventhandler(key_handler);
        inited=1;
      }

    if (inited)
    {
      if (event_types&i4_event::KEY_PRESS)
        add_agent(for_who,i4_event::KEY_PRESS);
      if (event_types&i4_event::KEY_RELEASE)
        add_agent(for_who,i4_event::KEY_RELEASE);

      return (i4_event::KEY_PRESS|i4_event::KEY_RELEASE)&event_types;
    }
  }


  virtual void release_device(i4_event_handler_class *for_who, w32 event_types)
  {
    if (event_types&i4_event::KEY_PRESS)
      remove_agent(for_who,i4_event::KEY_PRESS);
    if (event_types&i4_event::KEY_RELEASE)
      remove_agent(for_who,i4_event::KEY_RELEASE);

    if (!agent_list && inited)
    {
      keyboard_setdefaulteventhandler();
      keyboard_close();
      inited=0;
    }
  }
} ;

extern svgalib_keyboard_class svgalib_keyboard_instance;

#endif
