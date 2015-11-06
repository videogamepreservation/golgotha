/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "video/glide/glide_display.hh"
#include "video/x11/x11_input.hh"


class i4_x11_glide_display_class : public i4_glide_display_class
{
public:
  x11_input_class input;
  virtual i4_device_class *local_devices() { return &input; }
  virtual void destroy_window() {  input.destroy_window(); }
  virtual i4_bool create_window()
  {
    return input.create_window(0,0,tmp.xres,
                               tmp.yres, this, i4_F, input.find_visual_with_depth(16));
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

};

i4_x11_glide_display_class i4_x11_glide_display_instance;
