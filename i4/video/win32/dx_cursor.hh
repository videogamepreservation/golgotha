/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef DX_CURSOR_HH
#define DX_CURSOR_HH

#include "threads/threads.hh"
#include "palette/pal.hh"
#include "image/image.hh"


#include "window/cursor.hh"
#include <dinput.h>

class win32_input_class;

class dx_threaded_mouse_class
{
protected:

  LPDIRECTINPUT          direct_input_driver;
  LPDIRECTINPUTDEVICE    direct_input_mouse_device;
  HANDLE                 direct_input_mouse_event;
  HWND                   window_handle;
    
  i4_cursor_class     cursor;
  
  i4_bool             position_locked;
  
  i4_bool             active;

  i4_bool             visible;        // this is false until cursor is first displayed
  
  volatile i4_bool    stop;           // set by delete(), who waits for thread to stop  

  enum { thread_stack_size = 4000*1024 };
  
  void uninit_mouse();
  void init_mouse(HWND hwnd);
    
public:
  i4_critical_section_class draw_lock; 
  i4_critical_section_class acquire_lock;

  sw32 clip_x1, clip_y1, clip_x2, clip_y2;


  win32_input_class *input;

  sw32 current_mouse_x,
       current_mouse_y;  

  sw32 save_mouse_x,
       save_mouse_y;
  
  void lock_position(i4_bool yes_no)
  {
    if (yes_no)
      position_locked = i4_T;
    else
      position_locked = i4_F;
  }

  // private, don't call
  void thread();

  virtual void remove()  {}
  virtual void save()    {}
  virtual void display() {}
  virtual void set_cursor(i4_cursor_class *cursor) {};
  virtual void use_backbuffer(i4_bool use_bbuf) = 0;
  
  virtual void set_visible(i4_bool set)
  {    
    visible = set;    
  }


  dx_threaded_mouse_class(const i4_pal *screen_pal, 
                          HWND window_handle,
                          sw32 clip_x1, sw32 clip_y1,
                          sw32 clip_x2, sw32 clip_y2);

  virtual ~dx_threaded_mouse_class();

  // call when app loses focus
  void set_active(i4_bool act);
};



#endif
