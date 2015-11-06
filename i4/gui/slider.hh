/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef I4_SLIDER_HH
#define I4_SLIDER_HH

#include "time/timedev.hh"

class i4_graphical_style_class;
class i4_event_handler_class;

#include "device/event.hh"
#include "window/window.hh"

class i4_slider_event : public i4_object_message_event_class
{
public:
  w32 x, divisor;
  i4_slider_event(void *object, w32 x, w32 divisor)
    : i4_object_message_event_class(object), x(x), divisor(divisor)  {}
  i4_event *copy() { return new i4_slider_event(object,x,divisor); }
  char *name() { return "slide event"; }
};

class i4_slider_class : public i4_window_class
{
  i4_graphical_style_class *style;
  sw32 off, lx,ly;
  i4_bool grab, active, need_cancel;
  sw32 bw, milli_delay;                  // button width

  
  i4_time_device_class::id t_event;
  i4_event_handler_class *notify;

  void set_off_from_mouse();
  void send_change();

public:
  void set_notify(i4_event_handler_class *n) { notify=n; }

  i4_slider_class(sw32 width, 
                  sw32 initial_start,
                  i4_event_handler_class *notify,
                  w32 milli_delay,
                  i4_graphical_style_class *style);
  ~i4_slider_class();

  void draw(i4_draw_context_class &context);
  void receive_event(i4_event *ev);
  char *name() { return "slider"; }
};


#endif
