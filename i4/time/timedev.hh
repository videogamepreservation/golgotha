/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef __TIMEDEV_HPP_
#define __TIMEDEV_HPP_




#include "device/device.hh"
#include "time/time.hh"
class i4_event_handler_class;
class i4_event;


class i4_time_device_class : public i4_device_class
{


public:
#ifdef _WINDOWS
  void callback(w32 id);
#endif

  
  class timed_event
  {
  public:
    i4_event_handler_class *send_to;
    i4_event *event;
    i4_time_class start_time;
    w32 milli_wait;
    w32 win32_timer_id;
    timed_event *next;
    timed_event(i4_event_handler_class *send_to, i4_event *event, w32 milli_wait);
    ~timed_event();
  } ;

 
  class id
  {
  public:
    timed_event *reference;
    id(timed_event *reference) : reference(reference) {}
    id() { reference=0; }
  } ;


  id request_event(i4_event_handler_class *send_to,   // who to send the event to
      i4_event *event,                                // what event to send
      w32 milli_wait);                   // how much time to wait until sending (in milli-seconds)

  i4_bool cancel_event(const id &handle);
   
  virtual i4_bool process_events();       // returns true if an event was dispatched
  virtual char *name() { return "Time Device"; }
  virtual device_flags reports_events() { return 0; }  // doesn't not report common events
} ;

extern i4_time_device_class i4_time_dev;


#endif
