/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "device/kernel.hh"
#include "memory/malloc.hh"
#include "error/alert.hh"
#include "device/device.hh"
#include "device/event.hh"
#include "time/time.hh"
#include "threads/threads.hh"
#include "isllist.hh"

enum { I4_SHOW_NONE,
       I4_SHOW_ALL,
       I4_SHOW_NON_TRIVAL,        // excludes mouse move & window messages     
};

int i4_show_events=I4_SHOW_NONE;

i4_kernel_device_class i4_kernel;


struct defered_event
{
  i4_event_handler_reference_class<i4_event_handler_class> send_to;
  defered_event *next;
  i4_event *ev_copy;
  defered_event(i4_event_handler_class *_send_to, 
                i4_event *ev) : ev_copy(ev->copy())
  {
    send_to.reference(_send_to);
  }

  ~defered_event()
  {
    delete ev_copy;
  }

};

  
struct event_handler_delete_node
{
  i4_event_handler_class *who;
  event_handler_delete_node *next;
  event_handler_delete_node(i4_event_handler_class *who)
    : who(who) {}
};


static defered_event *defered_list;



static i4_critical_section_class list_lock;
static i4_isl_list<defered_event> list;
typedef i4_isl_list<defered_event>::iterator def_iter;

static i4_isl_list<event_handler_delete_node> eh_delete_list;
static i4_time_class last_user_input;

// r includes event and who to send to
void i4_kernel_device_class::send(i4_event_reaction_class *r)
{
  if (r && r->handler_reference.get())
    send_event(r->handler_reference.get(), r->event);
}



#ifdef DEBUG
void i4_kernel_device_class::show_pending()
{
  list_lock.lock();

  i4_isl_list<defered_event>::iterator i=list.begin();

  for (;i!=list.end();++i)
    i4_warning("'%s' for '%s'",i->ev_copy->name(), i->send_to.get()->name());    
  
  list_lock.unlock();
}
#endif

void i4_kernel_device_class::deque_events(i4_event_handler_class *for_who)
{
  list_lock.lock();

  i4_isl_list<defered_event>::iterator i=list.begin(), last=list.end(), q;
  for (; i!=list.end(); )
  {
    if (i->send_to.get()==for_who)
    {
      if (last==list.end())
        list.erase();
      else
        list.erase_after(last);
      

      q=i;
      ++i;

      list_lock.unlock();      // in case deleted events send events
      delete &*q;
      list_lock.lock();
    }
    else
    {
      last=i;
      ++i;
    }
  }
  
  list_lock.unlock();
}


void i4_kernel_device_class::send_event(i4_event_handler_class *send_to, i4_event *ev)
{


  if (ev->when()!=i4_event::NOW)
  {   
#ifndef I4_RETAIL
    if (i4_show_events==I4_SHOW_ALL || 
        (i4_show_events==I4_SHOW_NON_TRIVAL && 
         !(ev->type()==i4_event::MOUSE_MOVE || ev->type()==i4_event::WINDOW_MESSAGE)))
      i4_warning("queing : '%s' to '%s'",ev->name(), send_to->name());
#endif
    defered_event *dv=new defered_event(send_to,ev);

    list_lock.lock(); 
    list.insert_end(*dv);   
    list_lock.unlock();
  } else 
  {
#ifndef I4_RETAIL
    if (i4_show_events==I4_SHOW_ALL || 
        (i4_show_events==I4_SHOW_NON_TRIVAL && 
         !(ev->type()==i4_event::MOUSE_MOVE || ev->type()==i4_event::WINDOW_MESSAGE)))
      i4_warning("sending : '%s' to '%s'",ev->name(), send_to->name());
#endif

    send_to->call_stack_counter++;
    events_sent++;
    send_to->receive_event(ev);
    send_to->call_stack_counter--;
  }  
}

i4_bool i4_kernel_device_class::process_events()       // returns true if an event was dispatched  
{
  i4_bool ret=i4_F;
  for (i4_device_class *d=device_list; d; d=d->next)
    if (d->process_events())
      ret=i4_T;

  ret=(i4_bool)(flush_events() | ret); 

  check_for_idle();

  return ret;
}


i4_bool i4_kernel_device_class::flush_events()
{


  i4_bool ret=i4_F;

  // send any events that were qued
  while (list.begin()!=list.end())
  {
    list_lock.lock();
    i4_isl_list<defered_event>::iterator old=list.begin();
    list.erase();
    list_lock.unlock();

    // make sure event handler is still around..
    i4_event_handler_class *eh=old->send_to.get();   
    if (eh)
    {
#ifdef DEBUG
      i4_event *ev=old->ev_copy;
      if (i4_show_events==I4_SHOW_ALL || 
          (i4_show_events==I4_SHOW_NON_TRIVAL && 
           !(ev->type()==i4_event::MOUSE_MOVE || ev->type()==i4_event::WINDOW_MESSAGE)))
        i4_warning("sending : '%s' to '%s'",old->ev_copy->name(), eh->name());
#endif

      eh->call_stack_counter++;
      events_sent++;
      eh->receive_event(old->ev_copy);
      eh->call_stack_counter--;
    }

    delete &*old;

    ret=i4_T;
  }


  list_lock.lock();
  // delete any event handlers that are qued for deletion
  i4_isl_list<event_handler_delete_node>::iterator i=eh_delete_list.begin(), 
    last=eh_delete_list.end(), q;


  while (i!=eh_delete_list.end())
  {
    if (!i->who->thinking())
    {
      q=i;
      ++i;
      
      if (last!=eh_delete_list.end())
        eh_delete_list.erase_after(last);
      else
        eh_delete_list.erase();

      list_lock.unlock();     // unlock because deleted object might send events

      delete q->who;
      delete &*q;

      list_lock.lock();
    }
    else ++i;
  }


  list_lock.unlock();

  return ret;
}


void i4_kernel_device_class::request_events(i4_event_handler_class *for_who, w32 event_types)
{
  int type=0;
  while (event_types)
  {
    if (event_types&1)
      response[type]=new response_type(for_who, response[type]);

    event_types>>=1;    
    type++;
  }
}

// unrequest_events tells any devices sending event_types events to you to stop
void i4_kernel_device_class::unrequest_events(i4_event_handler_class *for_who, w32 event_types)
{
  int type=0;
  while (event_types)
  {
    if (event_types&1)
    {
      response_type *last=0, *p;
      for (p=response[type]; p && p->who!=for_who; p=p->next)
        last=p;
        
        
      
      if (!p)
        i4_error("unrequesting events & not installed");
      else
      {
        //        for_who->dereference();
        if (response[type]->who==for_who)
          response[type]=response[type]->next;
        else
          last->next=p->next;

        delete p;
      }
    }
    event_types>>=1;
    type++;
  }
}

void i4_kernel_device_class::add_device(i4_device_class *device)       // returns 16 bits device id
{
  device->next=device_list;
  device_list=device;
}

void i4_kernel_device_class::remove_device(i4_device_class *device)
{
  i4_device_class *last=0, *p;
  for (p=device_list; p && p!=device; p=p->next)
    last=p;

  if (!p)
    i4_error("remove device : device not found");
  if (last)
    last->next=device->next;
  else
    device_list=device->next;
}


void i4_kernel_device_class::not_idle()
{
  last_user_input.get();
}

void i4_kernel_device_class::check_for_idle()
{
  if (can_send_idle)
  {
    i4_time_class now;
    if (now.milli_diff(last_user_input)>milliseconds_before_idle_events_sent)
    {
      i4_user_idle_event_class uiev;
      broadcast_event_type(&uiev, i4_device_class::FLAG_IDLE);
      can_send_idle=i4_F;
    }  
  }
}

void i4_kernel_device_class::set_milliseconds_before_idle_events_sent(w32 milli_seconds)
{
  milliseconds_before_idle_events_sent=milli_seconds;
  check_for_idle();
}

i4_kernel_device_class::i4_kernel_device_class()
{
  events_sent=0;
  can_send_idle=i4_F;
  milliseconds_before_idle_events_sent=1000;

  memset(response,0,sizeof(response));
  device_list=0;
}


void i4_kernel_device_class::broadcast_event_type(i4_event *ev, w32 event_type)
{
  if (event_type & (i4_device_class::FLAG_MOUSE_MOVE | 
                    i4_device_class::FLAG_MOUSE_BUTTON_DOWN | 
                    i4_device_class::FLAG_MOUSE_BUTTON_UP |
                    i4_device_class::FLAG_KEY_PRESS |                    
                    i4_device_class::FLAG_KEY_RELEASE))
  {
    can_send_idle=i4_T;
    last_user_input.get();      
  }

  int type=0;
  while ((event_type&1)==0)
  {
    event_type>>=1;
    type++;
  }

  for (response_type *r=response[type]; r; r=r->next)
    send_event(r->who, ev);

}


void i4_kernel_device_class::delete_handler(i4_event_handler_class *handler)
{
  if (handler->thinking())
    eh_delete_list.insert(*(new event_handler_delete_node(handler)));
  else
  {
    //i4_warning("need to fix this handler deletion thing, trey");
    ///*
    if (!valid_ptr(handler))
    {
      valid_ptr(handler);
      i4_warning("bad handler pointer");
    }
    else
    //*/
      delete handler;
  }
}
