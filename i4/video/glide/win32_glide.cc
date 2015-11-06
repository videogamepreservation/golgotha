/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "threads/threads.hh"
#include "video/glide/glide_display.hh"
#include "video/win32/win32_input.hh"
#include "main/win_main.hh"

class i4_win32_glide_display_class : public i4_glide_display_class
{
public:
  virtual char *name() { return "3dfx / Glide"; }

  win32_input_class input;
  virtual i4_device_class *local_devices() { return &input; }
  virtual void destroy_window() {  input.destroy_window(); }
  virtual i4_bool create_window()
  {
    return input.create_window(0,0,tmp.xres,
                               tmp.yres, this, i4_win32_startup_options.fullscreen);
  }


  virtual void get_mouse_pos(sw32 &mouse_x, sw32 &mouse_y)
  {
    mouse_x=input.mouse_x;
    mouse_y=input.mouse_y;
  }

  virtual i4_bool lock_mouse_in_place(i4_bool yes_no)
  {
    return input.lock_mouse_in_place(yes_no);
  }

  virtual unsigned long window_handle() { return (unsigned long)i4_win32_window_handle; }
  
  i4_win32_glide_display_class(char *message):input(message) {}
  
};

i4_win32_glide_display_class i4_win32_glide_display_instance("Hit Left Control + F9 to return to Golgotha");
