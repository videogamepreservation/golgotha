/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef I4_WIN_MAIN_HH
#define I4_WIN_MAIN_HH


#include <windows.h>

// globals that other windows routines may need access to

extern HINSTANCE i4_win32_instance;  // instance of the application passed in through main
extern int       i4_win32_nCmdShow;  // how the window was initially opened (minimized?)
extern HWND      i4_win32_window_handle;    // main window, for some reason direct sound needs this

struct i4_win32_startup_options_struct
{
  char fullscreen;
  i4_win32_startup_options_struct()
  {
    fullscreen=1;
  }
  void check_option(char *opt);
};

extern i4_win32_startup_options_struct i4_win32_startup_options;

#endif
