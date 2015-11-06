/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef __DRAGWIN_HPP_
#define __DRAGWIN_HPP_

#include "window/window.hh"

class i4_drag_frame_class;

class i4_draggable_window_class : public i4_parent_window_class
{
  i4_coord last_mouse_x,
           last_mouse_y,
           current_mouse_x,
           current_mouse_y;

  i4_drag_frame_class *drag_frame;
  public :
  virtual void receive_event(i4_event *ev);
  i4_draggable_window_class(w16 w, w16 h);
  char *name() { return "drag_frame"; }
} ;



#endif
