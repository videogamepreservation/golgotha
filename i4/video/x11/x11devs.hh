/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef __X11_DEVICES_HPP_
#define __X11_DEVICES_HPP_

#include <X11/Xlib.h> 
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include "image/depth.hh"
#include "image/image8.hh"
#include "image/image16.hh"
#include "image/image32.hh"
#include "video/display.hh"
#include "video/x11/mitshm.hh"
#include "time/time.hh"

class x11_display_class : public i4_display_class
{  
private:
  i4_bool open_display();
  void close_display();

  // assumes that the display has already been grabbed
  i4_bool open_X_window(w32 width, w32 height, i4_display_class::mode *mode);
  void    create_X_image(w32 width, w32 height);
  void    destroy_X_image();
  void    close_X_window();

  // returns false if an error occured
  i4_bool copy_part_to_vram(i4_coord x, i4_coord y, 
                            i4_coord x1, i4_coord y1, 
                            i4_coord x2, i4_coord y2);

  class x11_mode : public mode
  {
  public:
    i4_display_class *assoc;     // pointer to use, so we can confirm we created this mode
  };
    x11_mode amode, cur_mode;

  // generic image which will take the type of the screen when we copy the save portion
  i4_image_class *mouse_pict;    

  I4_SCREEN_TYPE *mouse_save,
                 *screen;

  i4_coord mouse_x,mouse_y;


  
protected:
  class x11_device_manager : public i4_device_class     // maintains the mouse & keyboard
    {
  private:
      void get_x_time(w32 xtick, i4_time_class &t);
      i4_time_class i4_start_time;
      i4_bool need_first_time;
      sw32 x11_start_time, x11_start_time_best_diff, first_time;
      x11_display_class *parent;
      w16 modifier_state;            // keyboard shift state
  public:
      char *name() { return "x11 device manager"; } 
      virtual i4_bool process_events();       // returns true if an event was dispatched
      x11_device_manager();
      
      virtual device_flags reports_events()
    {
      return FLAG_MOUSE_MOVE|
        FLAG_MOUSE_BUTTON_DOWN|
        FLAG_MOUSE_BUTTON_UP|
        FLAG_KEY_PRESS|
        FLAG_KEY_RELEASE|
        FLAG_DISPLAY_CHANGE;
    }

  };
    x11_device_manager devs;

  i4_draw_context_class *context;

public:
  i4_bool mouse_locked;
  i4_coord                 mouse_hot_x,mouse_hot_y;
  i4_pal_handle_class      mouse_pal;
  i4_color                 mouse_trans;
  x11_shm_extension_class *shm_extension;
  x11_shm_image_class     *shm_image;
  XImage                  *ximage;
  Colormap                 xcolor_map;
  XVisualInfo             *my_visual;
  int                      screen_num;
  Display                 *display;
  Window                   mainwin;
  GC                       gc;
 
  virtual i4_bool set_mouse_shape(i4_cursor_class *cursor);

  virtual i4_image_class *get_screen();

  virtual w16 width() const { return screen->width(); }
  virtual w16 height() const { return screen->height(); }

  // makes the physical display consistant with previous gfx calls
  // either through page flipping or copying dirty rects from an
  // off-screen buffer
  virtual void flush();

  // returns some descriptive name for this display
  virtual char *name();

  virtual i4_draw_context_class *get_context() { return context; }
  x11_display_class();

  virtual mode *current_mode() { return &cur_mode; }
  virtual mode *get_first_mode();
  virtual mode *get_next_mode(mode *last_mode);


  // initialize_mode need not call close() to switch to another mode
  virtual i4_bool initialize_mode(mode *which_one);

  // should be called before a program quits
  virtual i4_bool close();
  virtual i4_bool available();

  // loads palette into hardware registers, return i4_F if pal_id is wrong color depth for display
  virtual i4_bool realize_palette(i4_pal_handle_class pal_id);

  virtual i4_bool lock_mouse_in_place(i4_bool yes_no)
  {
    mouse_locked=yes_no;
    return i4_T;
  }

  virtual i4_bool display_busy() const
    {
    //    if (shm_extension && shm_extension->need_sync_event)
    //      return i4_T;
    //    else 
    return i4_F;
  }
  };

extern x11_display_class x11_display_instance;

#endif



