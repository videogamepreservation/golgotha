/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef __WIN32_HPP_
#define __WIN32_HPP_

#include <windows.h>
#include "threads/threads.hh"

#include "image/image.hh"
#include "video/display.hh"
#include "isllist.hh"
#include "video/win32/win32_input.hh"

class win32_display_class : public i4_display_class
{  
private:
  i4_bool create_direct_draw();
  void destroy_direct_draw();

  void copy_part_to_vram(i4_coord x, i4_coord y, 
                         i4_coord x1, i4_coord y1, 
                         i4_coord x2, i4_coord y2);



  mode amode, cur_mode;

  I4_SCREEN_TYPE          *back_buffer;
  BITMAPINFOHEADER         back_buffer_bmp;
  BITMAPINFO               dc_bmpinfo;
  i4_bool                  win_is_open;
  HBITMAP                  bitmap;
  i4_draw_context_class   *context;  
  w32                      open_window_flags;  // passed to CreateWindowEx

public:
  win32_input_class input;

  void make_window_stay_top(i4_bool yes);


  i4_bool minimized;

  char *class_name() { return "game"; }
  char *window_name() { return "window"; }

  virtual mode *current_mode() { return &cur_mode; }
                          
  i4_coord            current_window_x,current_window_y;
  friend class win32_device_manager;

  virtual w16 width() const { return back_buffer->width(); }
  virtual w16 height() const { return back_buffer->height(); }


  // called when windows gives us a WM_MOVE message, this tells everyone else interested
  void move_screen(i4_coord x, i4_coord y);         
  // called when windows gives us a WM_RESIZE message, this tells everyone else interested
  void resize_screen(i4_coord w, i4_coord h);         

 
  virtual i4_bool set_mouse_shape(i4_cursor_class *cursor)
  { 
    return i4_F;
  }

  // devices local to this display (Keys & mice)
  virtual i4_device_class *local_devices() { return &input; }

  // makes the physical display consistant with previous gfx calls
  // either through page flipping or copying dirty rects from an
  // off-screen buffer
  virtual void flush();
  virtual i4_image_class *get_screen() { return back_buffer; }

  // returns some descriptive name for this display
  virtual char *name() { return "Windowed"; }

  virtual i4_draw_context_class *get_context() { return context; }
  win32_display_class();

  virtual mode *get_first_mode();
  virtual mode *get_next_mode(mode *last_mode) { return 0; }


  // initialize_mode need not call close() to switch to another mode
  virtual i4_bool initialize_mode(mode *which_one);

  // should be called before a program quits
  virtual i4_bool close();
  virtual i4_bool available() { return i4_T; }

  virtual i4_image_class *grab_framebuffer() { return back_buffer->copy(); }

  // loads palette into hardware registers, return i4_F if pal_id is wrong color depth for display
  virtual i4_bool realize_palette(i4_pal_handle_class pal_id) 
  {
    return i4_F;
  }

  virtual i4_bool display_busy() const
  {
    if (!input.get_active())
      return i4_T;
    else return i4_F;
  }

  virtual void set_mouse_raw_mode(i4_bool yes_no) 
  {
    input.set_mouse_raw_mode(yes_no);
  }
};

extern win32_display_class win32_display_instance;

#endif



