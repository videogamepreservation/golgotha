/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef DX5_DISPLAY_HH
#define DX5_DISPLAY_HH

#include "video/display.hh"
#include "video/win32/win32_input.hh"
#include "video/win32/dx5_mouse.hh"
#include "video/win32/dd_cursor.hh"

#include "error/error.hh"
#include <ddraw.h>

class i4_cursor_class;


class i4_dx5_display_class : public i4_display_class
{
  i4_draw_context_class *context;
  
  sw32 mouse_x, mouse_y, last_mouse_x, last_mouse_y;

  i4_dx5_image_class *fake_screen;

  i4_rect_list_class next_frame_copy;
  i4_rect_list_class next_frame_two;


  class i4_dx5_mode : public i4_display_class::mode
  {    
  public:
    dx5_mode *dx5;
    int driver_id;
  } amode, cur_mode, last_mode;

  dx5_mode *mode_list;
  
  dx5_mouse_class           *mouse;
  ddraw_thread_cursor_class *thread_mouse;

  i4_bool use_page_flip, use_exclusive_mode;

  char name_buffer[1000];
public:
  int using_accelerated_driver() { return (i4_bool)((cur_mode.driver_id & 0x8000)!=0); }

  i4_dx5_display_class();

  struct error_handler_type
  {

    i4_error_function_type old_error_handler;
    i4_bool need_restore_old;
    error_handler_type() { need_restore_old=i4_F; }

  } error_handler;

  win32_input_class input;
  
  i4_draw_context_class *get_context() { return context; }  
  w16 width() const { return cur_mode.xres; }
  w16 height() const { return cur_mode.yres; }
  
  mode *current_mode() { return &cur_mode; }
  mode *get_first_mode(int driver_id);
  mode *get_next_mode(); 
  i4_bool initialize_mode();

  void flush();
  

  
  i4_image_class *get_screen();   
  
  void init();

  // should be called before a program quits
  i4_bool close();

  virtual i4_image_class *lock_frame_buffer(i4_frame_buffer_type type,
                                            i4_frame_access_type access);
  virtual void unlock_frame_buffer(i4_frame_buffer_type type);

  
  i4_bool set_mouse_shape(i4_cursor_class *cursor);
  
  i4_bool lock_mouse_in_place(i4_bool yes_no)
  {
    return input.lock_mouse_in_place(yes_no);
  }

  void begin_render_mode();
  void end_render_mode();

  virtual i4_bool display_busy() const 
  {
    return (i4_bool)(!input.get_active());
  }

  virtual i4_refresh_type update_model();

  ~i4_dx5_display_class();
};


extern i4_dx5_display_class *i4_dx5_display;

#endif
