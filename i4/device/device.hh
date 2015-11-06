/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef __DEVICE_HPP_
#define __DEVICE_HPP_

#include "arch.hh"
#include "error/error.hh"

/*
  
  Creating and sending events :
  -------------------
  See event/event.hh

 */

class i4_event_handler_class;
class i4_event;

class i4_event_handler_class;


// this class cannot be used directly, you must derive off of it with 
// the i4_event_handler_reference_class template (used to ensure type convience)
class i4_event_handler_private_reference_class
{
protected:
  friend class i4_event_handler_class;
  i4_event_handler_class *ref;
  i4_event_handler_private_reference_class *next;
  void reference(i4_event_handler_class *who);      // if who is 0, then reference is destroyed  
  void destroy_ref();
public:
  i4_event_handler_private_reference_class() { ref=0; }
  ~i4_event_handler_private_reference_class() { destroy_ref(); }
};

template <class T>
class i4_event_handler_reference_class : public i4_event_handler_private_reference_class
{
public:
  T *get() { return (T *)ref; }
  void reference(T *r) 
  { 
    i4_event_handler_private_reference_class::reference((i4_event_handler_class *)r); 
  }

  T& operator*() const { return *((T *)ref); }
  T* operator->() const { return (T *)ref; }
  i4_event_handler_reference_class<T>& operator=(const i4_event_handler_reference_class<T> &r) 
  { 
    i4_event_handler_private_reference_class::reference((i4_event_handler_class *)r.ref); 
    return *this; 
  }

  i4_event_handler_reference_class<T>& operator=(const T *ref) 
  { 
    i4_event_handler_private_reference_class::reference((i4_event_handler_class *)ref); 
    return *this; 
  }
};

// anything that wishes to receive events, must be derived from this
// event handlers should be destroyed through the kernel (i4_kernel.delete_handler())
// this will ensure that a referenced object is not destroyed.
class i4_event_handler_class  
{
  friend class i4_event_handler_private_reference_class;
  i4_event_handler_private_reference_class *first;
public:
  w16 call_stack_counter;                   // this ensures we are not deleted while running

  i4_bool thinking() { return (i4_bool)(call_stack_counter!=0); }
  i4_event_handler_class() { call_stack_counter=0; first=0; }
  virtual void receive_event(i4_event *ev) { ; }
  virtual ~i4_event_handler_class();

  virtual char *name() = 0;
};


// this class is used to pass around an event handler/event pair
// most gui objects use this to signal changes to other objects
class i4_event_reaction_class
{
public:
  i4_event_handler_reference_class<i4_event_handler_class> handler_reference;
  i4_event                                                *event;

  i4_event_reaction_class() { event=0; }

  i4_event_reaction_class(i4_event_handler_class *handler,
                          i4_event               *event);

  i4_event_reaction_class(i4_event_handler_class *handler, w32 user_message_id);

  i4_event_reaction_class *copy();
  ~i4_event_reaction_class();

};


// anything that generates events should be derived from this
class i4_device_class : public i4_event_handler_class   
{
protected:
  i4_device_class *dev_list;

  public:
  void receive_event(i4_event *ev) {; }
  enum 
  {
    FLAG_MOUSE_MOVE          = 1, 
    FLAG_MOUSE_BUTTON_DOWN   = 2, 
    FLAG_MOUSE_BUTTON_UP     = 4, 
    FLAG_KEY_PRESS           = 8, 
    FLAG_KEY_RELEASE         = 16,
    FLAG_DISPLAY_CHANGE      = 32,   // when display screen changes (size, location, etc).
    FLAG_DISPLAY_CLOSE       = 64,
    FLAG_SYSTEM_SIGNAL       = 128,
    FLAG_IDLE                = 256,  // when no mouse or keyboard input sent after set interval
    FLAG_DO_COMMAND          = 512,
    FLAG_END_COMMAND         = 1024,
    FLAG_DRAG_DROP_EVENTS    = 2048
  };
  typedef w32 device_flags;
  i4_device_class *next;
  
  void send_event_to_agents(i4_event *ev, device_flags receive_types);
  i4_device_class();

  // ************* Implement these functions for each device *******************
  virtual i4_bool process_events() = 0;       // returns true if an event was dispatched
};

#endif
