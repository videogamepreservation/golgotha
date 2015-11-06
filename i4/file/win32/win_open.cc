/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "file/get_filename.hh"
#include "window/style.hh"
#include "window/window.hh"
#include "string/string.hh"
#include "device/device.hh"
#include "device/kernel.hh"
#include <windows.h>
#include "string/string.hh"
#include "main/win_main.hh"
#include <direct.h>

class open_string : public i4_str
{
public:
  open_string(char *fname)
    : i4_str(strlen(fname))
  {
    len=strlen(fname);
    memcpy(ptr, fname, len);
  }
};


long FAR PASCAL WindowProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );



UINT APIENTRY win32_dialog_hook( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
  switch( message )
  {
    case WM_LBUTTONUP :

    case WM_RBUTTONUP :

    case WM_MBUTTONUP :
    case WM_ACTIVATEAPP :


    case WM_SYSKEYUP :
    case WM_KEYUP :
      WindowProc(hWnd, message, wParam, lParam);
      break;
  }

  return FALSE;
}


void i4_create_file_open_dialog(i4_graphical_style_class *style,
                                const i4_const_str &title_name,
                                const i4_const_str &start_dir,
                                const i4_const_str &file_mask,
                                const i4_const_str &mask_name,
                                i4_event_handler_class *tell_who,
                                w32 ok_id, w32 cancel_id)
{
  OPENFILENAME ofn;

  char mname[256], m[256], tname[256], idir[256], fname[256], curdir[256];

  ShowCursor(TRUE);

  _getcwd(curdir,256);

  i4_os_string(mask_name, mname, sizeof(mname));
  i4_os_string(file_mask, m, sizeof(m));
  i4_os_string(title_name, tname, sizeof(tname));
  i4_os_string(start_dir, idir, sizeof(idir));


  char *af="All files\0*.*\0\0";

  int i=strlen(mname)+1;
  int l=strlen(m) + 1;
  memcpy(mname + i, m, l);
  i+=l;
  l=15;
  memcpy(mname + i, m, l);

  fname[0]=0;

  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.hwndOwner = i4_win32_window_handle;
  ofn.hInstance = i4_win32_instance;
  ofn.lpstrCustomFilter = 0;
  ofn.nMaxCustFilter = 0;
  ofn.nFilterIndex = 0;
  ofn.lpstrFile = fname;
  ofn.nMaxFile = 256;

  ofn.nMaxFileTitle = 256;
  ofn.lpstrFileTitle = tname;

  ofn.lpstrInitialDir = idir;
  ofn.nFileOffset = 0;
  ofn.nFileExtension = 0;
  ofn.lCustData = 0L;
  ofn.lpfnHook = NULL;
  ofn.lpTemplateName = NULL;

  ofn.lpfnHook = win32_dialog_hook;


  ofn.lpstrFilter = mname;
  ofn.lpstrDefExt = "level";
  ofn.lpstrTitle = tname;
  ofn.Flags = OFN_HIDEREADONLY | OFN_ENABLEHOOK | OFN_EXPLORER  ;

  WindowProc(i4_win32_window_handle, WM_LBUTTONUP, 0,0);
  WindowProc(i4_win32_window_handle, WM_RBUTTONUP, 0,0);
  WindowProc(i4_win32_window_handle, WM_MBUTTONUP, 0,0);



  if (GetOpenFileName(&ofn)) 
  {	
    i4_file_open_message_class o(ok_id, new open_string(ofn.lpstrFile));
    i4_kernel.send_event(tell_who, &o);
  }
  else
  {
    i4_user_message_event_class o(cancel_id);
    i4_kernel.send_event(tell_who, &o);
  }

  _chdir(curdir);

  ShowCursor(FALSE);
}



void i4_create_file_save_dialog(i4_graphical_style_class *style,
                                const i4_const_str &default_name,
                                const i4_const_str &title_name,
                                const i4_const_str &start_dir,
                                const i4_const_str &file_mask,
                                const i4_const_str &mask_name,
                                i4_event_handler_class *tell_who,
                                w32 ok_id, w32 cancel_id)
{
  OPENFILENAME ofn;

  char mname[256], m[256], tname[256], idir[256], fname[256], curdir[256];

  _getcwd(curdir,256);

  i4_os_string(mask_name, mname, sizeof(mname));
  i4_os_string(file_mask, m, sizeof(m));
  i4_os_string(title_name, tname, sizeof(tname));
  i4_os_string(start_dir, idir, sizeof(idir));


  char *af="All files\0*.*\0\0";

  int i=strlen(mname)+1;
  int l=strlen(m) + 1;
  memcpy(mname + i, m, l);
  i+=l;
  l=15;
  memcpy(mname + i, m, l);

  fname[0]=0;

  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.hwndOwner = i4_win32_window_handle;
  ofn.hInstance = i4_win32_instance;
  ofn.lpstrCustomFilter = 0;
  ofn.nMaxCustFilter = 0;
  ofn.nFilterIndex = 0;
  ofn.lpstrFile = fname;
  ofn.nMaxFile = 256;

  ofn.nMaxFileTitle = 256;
  ofn.lpstrFileTitle = tname;

  ofn.lpstrInitialDir = idir;
  ofn.nFileOffset = 0;
  ofn.nFileExtension = 0;
  ofn.lCustData = 0L;
  ofn.lpfnHook = NULL;
  ofn.lpTemplateName = NULL;

  ofn.lpfnHook = win32_dialog_hook;

  ofn.lpstrFilter = mname;
  ofn.lpstrDefExt = m;
  while (*ofn.lpstrDefExt && *ofn.lpstrDefExt!='.')
    ofn.lpstrDefExt++;

  ofn.lpstrTitle = tname;
  ofn.Flags = OFN_HIDEREADONLY | OFN_ENABLEHOOK | OFN_EXPLORER ;

  WindowProc(i4_win32_window_handle, WM_LBUTTONUP, 0,0);
  WindowProc(i4_win32_window_handle, WM_RBUTTONUP, 0,0);
  WindowProc(i4_win32_window_handle, WM_MBUTTONUP, 0,0);


  if (GetSaveFileName(&ofn)) 
  {	
    i4_file_open_message_class o(ok_id, new open_string(ofn.lpstrFile));
    i4_kernel.send_event(tell_who, &o);
  }
  else
  {
    i4_user_message_event_class o(cancel_id);
    i4_kernel.send_event(tell_who, &o);
  }

  _chdir(curdir);
}
