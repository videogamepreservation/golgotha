/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef __X11_DISPLAY
#define __X11_DISPLAY

#include <X11/Xlib.h> 
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include "image/image.hh"
#include "video/display.hh"
#include "video/x11/mitshm.hh"
#include "time/time.hh"
#include "video/x11/x11_input.hh"


class x11_display_input_class : public x11_input_class
{
public:
  virtual void resize(int w, int h);
  virtual void note_event(XEvent &xev);

};

class x11_display_class : public i4_display_class
{  
protected:
  virtual XVisualInfo *get_visual(Display *display);

public:
  i4_display_list_struct me;
  x11_shm_extension_class *shm_extension;

  x11_display_input_class input;


  i4_bool open_display();
  void close_display();


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

  i4_image_class *mouse_save, *screen;

  i4_draw_context_class *context;

  i4_coord                 mouse_hot_x,mouse_hot_y;
  const i4_pal            *mouse_pal;
  i4_color                 mouse_trans;
  x11_shm_image_class     *shm_image;
  XImage                  *ximage;
 
  virtual i4_bool set_mouse_shape(i4_cursor_class *cursor);

  virtual i4_image_class *get_screen() { return screen; }

  virtual w16 width() const { return screen->width(); }
  virtual w16 height() const { return screen->height(); }

  // makes the physical display consistant with previous gfx calls
  // either through page flipping or copying dirty rects from an
  // off-screen buffer
  virtual void flush();

  // returns some descriptive name for this display
  virtual char *name() { return "X11 display"; }

  virtual i4_draw_context_class *get_context() { return context; }
  x11_display_class();

  virtual mode *current_mode() { return &cur_mode; }
  virtual mode *get_first_mode(int driver_id);
  virtual mode *get_next_mode() { return 0; }


  // initialize_mode need not call close() to switch to another mode
  virtual i4_bool initialize_mode();

  // should be called before a program quits
  virtual i4_bool close();

  virtual i4_bool lock_mouse_in_place(i4_bool yes_no)
  {
    return input.lock_mouse_in_place(yes_no);
  }

  // if you want to read/draw directly to/from the frame buffer you have to lock it
  // first, be sure to release it as soon as possible.  Normally locking the frame buffer is
  // bad in a game because the 3d pipeline most be flushed before this can happen.
  virtual i4_image_class *lock_frame_buffer(i4_frame_buffer_type type,
                                            i4_frame_access_type access)
  {
    return screen;
  }

  virtual void unlock_frame_buffer(i4_frame_buffer_type type) { ; }
  
  // tells you weither the current update model is BLT, or PAGE_FLIP
  virtual i4_refresh_type update_model() { return I4_BLT_REFRESH; }



  virtual i4_bool display_busy() const
  {
    //    if (shm_extension && shm_extension->need_sync_event)
    //      return i4_T;
    //    else 
    return i4_F;
  }

  void init();
};

extern x11_display_class x11_display_instance;

#endif



