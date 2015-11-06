/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

//{{{ Display Base Class

//    - no drawing is seen until the function flush() is called.. 
//      This is acomplished through page-flipping or backbuffering.
//      Dirty rectangles are maintained for each graphic activity, so
//      speed is not the optimal.  On a page-flipping system, the
//      dirty rectangles are copied to the previous page after
//      flipping On a back-buffer system, the dirty rectangles are
//      copied from system to video memory.
//
//    - a display maintains a list of devices associated with it
//      (typically keyboard and mouse) these are typically added in
//      externally by another unit.
//
//    - a display maintains a list of clip areas which can be modified
//      by external code (expected to a window manager)
//
//    - a display is capable of opening and closing different video
//      modes which may be different resolutions, color depth and flip
//      methods
//


#ifndef __DISPLAY_HPP_
#define __DISPLAY_HPP_

#include "arch.hh"
#include "init/init.hh"
#include "area/rectlist.hh"
#include "device/device.hh"
#include "image/image.hh"
#include "palette/pal.hh"
#include "window/cursor.hh"

class i4_display_class;
struct i4_display_list_struct;
extern i4_display_list_struct *i4_display_list;

// display's will add themselves to this list (possibly more than once)
// during their init if they determine they are available for use
struct i4_display_list_struct
{
public:
  char             *name;
  int               driver_id;
  i4_display_class *display;
  i4_display_list_struct *next;

  void add_to_list(char *_name, int _driver_id, 
                   i4_display_class *_display,
                   i4_display_list_struct *_next);
};

enum i4_frame_buffer_type { I4_FRONT_FRAME_BUFFER,
                            I4_BACK_FRAME_BUFFER };
                                            

enum i4_frame_access_type { I4_FRAME_BUFFER_READ,
                            I4_FRAME_BUFFER_WRITE };

enum i4_refresh_type { I4_BLT_REFRESH,
                       I4_PAGE_FLIP_REFRESH };

extern i4_display_list_struct *i4_display_list;


class i4_display_class : public i4_init_class
{
protected:
  const i4_pal *pal;

public:
  const i4_pal *get_palette() { return pal; }

  static i4_display_class *first_display;    // list of all displays currently available
  i4_display_class *next_display;
  
  virtual i4_image_class *get_screen() = 0;
  virtual i4_draw_context_class *get_context() = 0;

  // makes the physical display consistant with previous gfx calls
  // either through page flipping or copying dirty rects from an
  // off-screen buffer
  virtual void flush() = 0; 

  // width and height of the current mode
  virtual w16 width() const = 0;
  virtual w16 height() const = 0;

  //************************* Mode detection ***********************
  class mode
  {
  public:
    enum { MAX_MODE_NAME=60 } ;
    char name[MAX_MODE_NAME];

    enum 
    {
      RESOLUTION_DETERMINED_ON_OPEN=1,  // (windowed?) in which case xres & yres are maximum values
      BACK_BUFFER=2,                    // if not then page flipped should be true
      PAGE_FLIPPED=4     
    };

    w16 xres,yres;
    w8  bits_per_pixel;       // 1, 8, 16, 24, or 32

    w32 red_mask;
    w32 green_mask;
    w32 blue_mask;

    w16 flags;
  };

  virtual mode *current_mode() = 0;  // will not be valid until initialize_mode is called
  
  virtual mode *get_first_mode(int driver_id) = 0;
  virtual mode *get_next_mode() = 0;


  // if you want to read/draw directly to/from the frame buffer you have to lock it
  // first, be sure to release it as soon as possible.  Normally locking the frame buffer is
  // bad in a game because the 3d pipeline most be flushed before this can happen.
  virtual i4_image_class *lock_frame_buffer(i4_frame_buffer_type type,
                                            i4_frame_access_type access) = 0;
  virtual void unlock_frame_buffer(i4_frame_buffer_type type) = 0;
  
  // tells you weither the current update model is BLT, or PAGE_FLIP
  virtual i4_refresh_type update_model() = 0;

  // initialize_mode need not call close() to switch to another mode
  // initialize_mode uses the last mode accessed by user (through get_first_mode or get_next_mode)
  virtual i4_bool initialize_mode() = 0;

  // should be called before a program quits
  virtual i4_bool close() = 0;

  // changes shape of the mouse cursor, return false on failure (no mouse or unchangeable mouse)
  // cursor will be copied, so it can be deleted after this call
  virtual i4_bool set_mouse_shape(i4_cursor_class *cursor) = 0;

  virtual i4_bool lock_mouse_in_place(i4_bool yes_no) = 0;

  // returns i4_T if the display is busy, i.e. doing a bitblt, pageflip, or other
  // you usually must call kernel.process_events() before this will change
  virtual i4_bool display_busy() const { return i4_F; }
};

#endif

