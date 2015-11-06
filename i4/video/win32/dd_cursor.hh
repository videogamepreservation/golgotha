/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "video/win32/dx_cursor.hh"
#include "video/win32/dx5_util.hh"
#include <ddraw.h>

// base class that you should derive from
// this class will call at the appropriate times :
//    remove(), save(), and display()
// these should use the 'cursor' i4_image type to draw with

class ddraw_thread_cursor_class : public dx_threaded_mouse_class
{  
  public:  
  
  i4_dx5_image_class *mouse_save;

  ddraw_thread_cursor_class(const i4_pal *screen_pal,
                            HWND window_handle,
                            sw32 clip_x1, sw32 clip_y1,
                            sw32 clip_x2, sw32 clip_y2);

  void set_cursor(i4_cursor_class *cursor);

  i4_bool use_back_buffer;

  void use_backbuffer(i4_bool use);

  void remove();
  void save();
  void display();  
};
