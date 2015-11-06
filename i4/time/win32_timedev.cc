/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include <windows.h>
#include <windowsx.h>


#include "arch.hh"
#include "time/time.hh"
#include "time/timedev.hh"
#include "device/kernel.hh"
#include "init/init.hh"

i4_time_device_class i4_time_dev;

class i4_time_device_adder_class : public i4_init_class
{
  public :
  void init() 
  { 
    i4_kernel.add_device(&i4_time_dev);
  }
} i4_time_device_adder_instance;



i4_bool i4_time_device_class::cancel_event(const id &handle)
{
  i4_isl_list<timed_event>::iterator i=events.begin(),last=events.end();
  for (;i!=events.end();++i)
  {
    if (handle.reference==&*i)
    {
      if (last==events.end())
        events.erase();
      else
        events.erase_after(last);
      timeKillEvent(i->win32_timer_id);
      delete &*i;
      return i4_T;
    }
    last=i;
  }
  return i4_F;
}

i4_bool i4_time_device_class::process_events()       // returns true if an event was dispatched
{
  return i4_F;
}

void CALLBACK win32_timer_callback(UINT id, UINT msg, DWORD user, DWORD dw1, DWORD dw2)
{
  i4_time_dev.callback(user);
}

void i4_time_device_class::callback(w32 user)
{
  timed_event *got_event=(timed_event *)(user);
  got_event->send_to->receive_event(got_event->event);

  if (got_event==&*i4_time_dev.events.begin())
    events.erase();
  else
  {
    i4_isl_list<timed_event>::iterator i=events.begin(),last=events.end();
    for (; i->next!=got_event && i!=last; ++i);

    if (i!=last)
      events.erase_after(i);   
  }
  delete got_event;
}

// who to send the event to
i4_time_device_class::id i4_time_device_class::request_event(i4_event_handler_class *send_to, 
                                       i4_event *event,                 // what event to send
                                       w32 milli_wait) // how much time to wait (in milli-seconds)
{
  // first make sure this is not an event that needs to be sent right now
  // these events usually are two-way events that have return codes inside
  if (event->when()==i4_event::NOW)
    i4_error("Cannot send NOW events throught the time device!");
 
  timed_event *ev=new timed_event(send_to,event,milli_wait);
  events.insert(*ev);
  ev->win32_timer_id=timeSetEvent(milli_wait,0,
                                  win32_timer_callback,
                                  (DWORD)(ev),
                                  TIME_ONESHOT);

  return id(ev);
}
