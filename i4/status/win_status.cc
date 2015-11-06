/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "status/status.hh"


enum {MAX_UPDATES=100};

class win32_status_class
{
public:
  HWND hwndPB;    // handle of progress bar 

  int last_p;

  virtual i4_bool update(float percent)
  {
    int p=percent*MAX_UPDATES;
    while (last_p!=p)
    {
      last_p++;
      SendMessage(hwndPB, PBM_STEPIT, 0, 0); 
    }
  }

  win32_status_class(HWND hwndPB) : hwndPB(hwndPB)
  {
    last_p=0;
  }

  virtual ~i4_status_class()
  {
    DestroyWindow(hwndPB);     
  }
};

// this is operating system dependant
i4_status_class *i4_create_status(const i4_const_str &description)
{
  RECT rcClient;  // client area of parent window 
  int cyVScroll;  // height of scroll bar arrow 
  HWND hwndPB;    // handle of progress bar 
  HANDLE hFile;   // handle of file 
  DWORD cb;       // size of file and count of bytes read 
  LPCH pch;       // address of data read from file 
  LPCH pchTmp;    // temporary pointer 
 
    // Ensure that the common control DLL is loaded and create a 
    // progress bar along the bottom of the client area of the 
    // parent window. Base the height of the progress bar on 
    // the height of a scroll bar arrow. 
  InitCommonControls(); 


  hwndPB = CreateWindowEx(0, PROGRESS_CLASS, (LPSTR) NULL, 
                          WS_BORDER | WS_VISIBLE, CW_USEDEFAULT,  CW_USEDEFAULT,
                          320, 20,
                          0, (HMENU) 0, i4_win32_instance, NULL); 
 
 
  // Set the range and increment of the progress bar. 
  SendMessage(hwndPB, PBM_SETRANGE, 0, MAKELPARAM(0, MAX_UPDATES)); 
  SendMessage(hwndPB, PBM_SETSTEP, (WPARAM) 1, 0); 
 
  return new win32_status_class(hwndPB);
} 



