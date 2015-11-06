/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "status/status.hh"

i4_status_create_function_type i4_stat_fun=0;

void i4_set_status_create_function(i4_status_create_function_type fun)
{
  i4_stat_fun=fun;
}

i4_idle_class *i4_idle_class::first=0;


i4_idle_class::i4_idle_class()
{
  next=first;
  first=this;
}

i4_idle_class::~i4_idle_class()
{
  if (first==this)
    first=first->next;
  else 
  {
    i4_idle_class *p=first;
    for (;p->next && p->next!=this; p=p->next);
    p->next=next;
  }
}



#ifdef _WINDOWS
#include <windows.h>
#include <commctrl.h>
#include "main/win_main.hh"
#include "string/string.hh"
#include "time/time.hh"

enum {MAX_UPDATES=50};

static int stat_y=40;

class win32_status_class : public i4_status_class
{
public:
  HWND hwndPB;    // handle of progress bar 
  i4_time_class start_time;
  int last_p;
  i4_bool win_created;
  i4_str *description;

  virtual i4_bool update(float percent)
  {
    i4_time_class now;
    if (now.milli_diff(start_time)>500)
    {
      start_time.get();

      if (!win_created)
      {      
        InitCommonControls(); 

        char buf[100];
        i4_os_string(*description, buf, 100);

        hwndPB = CreateWindowEx(0, PROGRESS_CLASS, buf, 
                                WS_BORDER | WS_VISIBLE, 50, stat_y,
                                500, 40,
                                0, (HMENU) 0, i4_win32_instance, NULL); 
 
 
        // Set the range and increment of the progress bar. 
        SendMessage(hwndPB, PBM_SETRANGE, 0, MAKELPARAM(0, MAX_UPDATES)); 
        SendMessage(hwndPB, PBM_SETSTEP, (WPARAM) 1, 0); 
        win_created=i4_T;
      }   
    }

    if (win_created)
    {
      int p=percent*MAX_UPDATES;
      while (last_p!=p)
      {
        last_p++;
        SendMessage(hwndPB, PBM_STEPIT, 0, 0); 
      }
    }

    return i4_T;
  }

  win32_status_class(const i4_const_str &d)
  {
    description=new i4_str(d);
    win_created=i4_F;
    last_p=0;
    stat_y+=60;
  }

  virtual ~win32_status_class()
  {
    delete description;

    if (win_created)
    {
      while (last_p!=MAX_UPDATES)
      {
        last_p++;
        SendMessage(hwndPB, PBM_STEPIT, 0, 0); 
      }

      DestroyWindow(hwndPB);     
    }

    stat_y-=60;

  }
};



// this is operating system dependant
i4_status_class *i4_create_status(const i4_const_str &description, int flags)
{
  if (i4_stat_fun)
    return i4_stat_fun(description, flags);
  else
  {
    return new win32_status_class(description);
  }
} 


#else


class i4_null_status_class : public i4_status_class
{
public:
  i4_bool update(float percent) 
  { 
    for (i4_idle_class *p=i4_idle_class::first; p;p=p->next)
      p->idle();
    return i4_T; 
  }
};


i4_status_class *i4_create_status(const i4_const_str &description, int flags)
{
  if (i4_stat_fun)
    return i4_stat_fun(description, flags);
  else
    return new i4_null_status_class;
}



#endif
