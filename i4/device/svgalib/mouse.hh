/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef __SVGALIB_MOUSE_HPP_
#define __SVGALIB_MOUSE_HPP_

#include "device/device.hh"
#include "device/event.hh"
#include <vgamouse.h>
#include <vga.h>

enum { SVGA_L=4,SVGA_R=1,SVGA_C=2 } ;

class svgalib_mouse_class : public i4_device_class
{
  int cur_mb,cur_mx,cur_my,initialized;

  public :
  virtual i4_bool process_events() 
  {
    if (initialized)
    {
      mouse_update();
      int x=mouse_getx(),y=mouse_gety(),b=cur_mb;
      cur_mb=mouse_getbutton();

      if (b!=cur_mb)
      {
	if ((b&SVGA_L) && !(cur_mb&SVGA_L))   // left button up
	{
	  i4_mouse_button_up_event_class b(i4_mouse_button_up_event_class::LEFT);
	  send_event_to_agents(&b,FLAG_MOUSE_BUTTON_UP);
	} else if ((cur_mb&SVGA_L) && !(b&SVGA_L))
	{
	  i4_mouse_button_down_event_class b(i4_mouse_button_down_event_class::LEFT);
	  send_event_to_agents(&b,FLAG_MOUSE_BUTTON_DOWN);
	}

	if ((b&SVGA_R) && !(cur_mb&SVGA_R))   // right button up
	{
	  i4_mouse_button_up_event_class b(i4_mouse_button_up_event_class::RIGHT);
	  send_event_to_agents(&b,FLAG_MOUSE_BUTTON_UP);
	} else if ((cur_mb&SVGA_R) && !(b&SVGA_R))
	{
	  i4_mouse_button_down_event_class b(i4_mouse_button_down_event_class::RIGHT);
	  send_event_to_agents(&b,FLAG_MOUSE_BUTTON_DOWN);
	}

	if ((b&SVGA_C) && !(cur_mb&SVGA_C))   // center button up
	{
	  i4_mouse_button_up_event_class b(i4_mouse_button_up_event_class::CENTER);
	  send_event_to_agents(&b,FLAG_MOUSE_BUTTON_UP);
	} else if ((cur_mb&SVGA_C) && !(b&SVGA_C))
	{
	  i4_mouse_button_down_event_class b(i4_mouse_button_down_event_class::CENTER);
	  send_event_to_agents(&b,FLAG_MOUSE_BUTTON_DOWN);
	}

      } 
      else if (cur_mx!=x || cur_my!=y)
      {
	cur_mx=x; cur_my=y;
	i4_mouse_move_event_class ev(cur_mx,cur_my);
	send_event_to_agents(&ev,FLAG_MOUSE_MOVE);
      }
    }
  }

  svgalib_mouse_class()
  {
    initialized=0;
  }


  char *name() { return "SVGALIB mouse"; }

  w32 request_device(i4_event_handler_class *for_who, w32 event_types)
  {
    if ((event_types&i4_device_class::FLAG_MOUSE_BUTTON_UP) || 
	(event_types&i4_device_class::FLAG_MOUSE_BUTTON_DOWN) ||
	(event_types&i4_device_class::FLAG_MOUSE_MOVE))
    {
      if (!initialized)
	if (mouse_init("/dev/mouse",vga_getmousetype(),MOUSE_DEFAULTSAMPLERATE )==0)
	  initialized=1;

      if (initialized)
      {
	if (event_types&i4_device_class::FLAG_MOUSE_BUTTON_UP)
	  add_agent(for_who,i4_device_class::FLAG_MOUSE_BUTTON_UP);

	if (event_types&i4_device_class::FLAG_MOUSE_BUTTON_DOWN)
	  add_agent(for_who,i4_device_class::FLAG_MOUSE_BUTTON_DOWN);
	  
	if (event_types&i4_device_class::FLAG_MOUSE_MOVE)
	  add_agent(for_who,i4_device_class::FLAG_MOUSE_MOVE);

	return (i4_device_class::FLAG_MOUSE_BUTTON_UP|
		i4_device_class::FLAG_MOUSE_BUTTON_DOWN|
		i4_device_class::FLAG_MOUSE_MOVE)&event_types;
      }
    }
    return 0;
  }


  void release_device(i4_event_handler_class *for_who, w32 event_types)
  {
    if (event_types&i4_device_class::FLAG_MOUSE_BUTTON_UP)
      remove_agent(for_who,i4_device_class::FLAG_MOUSE_BUTTON_UP);

    if (event_types&i4_device_class::FLAG_MOUSE_BUTTON_DOWN)
      remove_agent(for_who,i4_device_class::FLAG_MOUSE_BUTTON_DOWN);

    if (event_types&i4_device_class::FLAG_MOUSE_MOVE)
      remove_agent(for_who,i4_device_class::FLAG_MOUSE_MOVE);
    
    if (!agent_list && initialized)  // no one using the mouse any more, uninit
    {
      mouse_close();
      initialized=0;
    }
  }

} ;

extern svgalib_mouse_class svgalib_mouse_instance;

#endif
