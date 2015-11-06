/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef __X11_INPUT_HH
#define __X11_INPUT_HH

#include "device/device.hh"
#include "time/time.hh"
#include <X11/Xlib.h> 
#include <X11/Xutil.h>

class i4_display_class;
class x11_shm_extension_class;
class i4_draw_context_class;

class x11_input_class : public i4_device_class 
{
public:
  Atom                    wm_delete_window, wm_protocols;
  XVisualInfo             *my_visual;
  Display                 *display;
  i4_bool                  mouse_locked;
  Window                   mainwin;
  GC                       gc;
  i4_display_class        *i4_display;
  Colormap                 xcolor_map;
  int                      screen_num;

  i4_draw_context_class   *context;
  w16                      modifier_state;            // keyboard shift state
  sw32                     mouse_x, mouse_y;
  i4_time_class            last_down[3], last_up[3];
  
  void get_x_time(w32 xtick, i4_time_class &t);

  i4_time_class i4_start_time;
  i4_bool need_first_time;
  sw32 first_time;
  i4_bool repeat_on;

  XVisualInfo *find_visual_with_depth(int depth);
  char *name() { return "X11 input"; } 

  i4_bool open_display();
  void close_display();

  i4_bool create_window(sw32 x, sw32 y, w32 w, w32 h, 
                        i4_display_class *i4_display,
                        i4_bool takeup_fullscreen,
                        XVisualInfo *visual);
  void destroy_window();

  virtual i4_bool process_events();

  virtual void resize(int w, int h) { ; }
  virtual void note_event(XEvent &xev) { ; }

  i4_bool lock_mouse_in_place(i4_bool yes_no)
  {
    mouse_locked=yes_no;
    return i4_T;
  }

  x11_input_class();
};




#endif
