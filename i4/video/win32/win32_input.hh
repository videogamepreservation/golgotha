/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef I4_WIN32_HH
#define I4_WIN32_HH


#include <windows.h>

#include "video/win32/dx_cursor.hh"
#include "device/device.hh"
#include "palette/pal.hh"
#include "time/time.hh"
#include "window/win_evt.hh"

class i4_display_class;


// this classes process key &mouse input for a win32 winow

class win32_input_class : public i4_device_class 
{
  HWND whandle;

  char *window_name() { return "Golgotha Milestone 5"; }

  char *class_name() { return "Crack dot Com"; }

//  BOOL CreateKeyboard(GUID &guid, LPDIRECTINPUTDEVICE& lpdiKey, DWORD dwAccess);
//  BOOL InitDInput(void);
  w16 translate_windows_key(w16 wkey, w32 &modifer);
  i4_bool have_window;
  w16 modifier_state;
  i4_bool active;
  i4_display_class *display;
  static i4_bool class_registered;
  i4_bool mouse_locked;    

  char  window_message[128];
  i4_time_class  last_down[3], last_up[3];

protected:
  virtual void redraw() { ; }
  virtual void resize(sw32 w, sw32 h) { ; }
  virtual void move(sw32 x, sw32 y) { ; }
  virtual void set_active(i4_bool yes_no) { active=yes_no; }
  virtual void close_event() { ; }
public:
  
  enum mouse_button_type  {LEFT,RIGHT,MIDDLE};
  enum mouse_button_state {UP,DOWN};

  void update_mouse_movement(sw32 new_mouse_x, sw32 new_mouse_y);

  void update_mouse_buttons(i4_mouse_button_event_class::btype type, mouse_button_state state);
  
  HWND get_window_handle() { return whandle; }

  //dx_threaded_mouse_class *async_mouse;
  void *async_mouse;

  sw32 mouse_x, mouse_y;
  struct confine_struct 
  {
    i4_bool on;
    sw32 x1,y1,x2,y2;
    confine_struct() { on=i4_F; }
  } confine;

  sw32 process(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, DWORD time);

  win32_input_class(char *extra_message);

  char *name() { return "Win32 device manager"; } 

  void get_window_area(int &x, int &y, int &w, int &h);

  i4_bool create_window(sw32 x, sw32 y, w32 w, w32 h, 
                        i4_display_class *display_responsible_for_close,
                        i4_bool takeup_fullscreen=i4_F);

  void destroy_window();

  i4_bool get_active() const { return active; }

  virtual i4_bool process_events();

  void set_async_mouse(dx_threaded_mouse_class *m);

  i4_bool lock_mouse_in_place(i4_bool yes_no);  
};





#endif

