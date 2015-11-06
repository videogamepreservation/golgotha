/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "main/main.hh"
#include "main/win_main.hh"
#include "error/error.hh"
#include "error/alert.hh"
#include "file/file.hh"

// this global will be used by video/directx/directx.c when opening a window
#include <windows.h>
#include <windowsx.h>
#include <stdio.h>

w32           i4_global_argc;
i4_const_str *i4_global_argv;


HINSTANCE i4_win32_instance;
int       i4_win32_nCmdShow;
HWND      i4_win32_window_handle=0;
i4_win32_startup_options_struct i4_win32_startup_options;
void *i4_stack_base=0;

int i4_win32_alert(const i4_const_str &ret)
{
  char tmp[1024], *st;
  strcpy(tmp, "Alert : ");
  st=tmp+8;

  i4_const_str::iterator s=ret.begin();
  while (s!=ret.end())
  {
    *st=s.get().value();
    ++s;
    st++;
  }
  *st=0;
  strcat(tmp,"\n");
  OutputDebugString(tmp);
  setbuf(stdout,0);
  printf(tmp);
  return 1;
}


void i4_win32_startup_options_struct::check_option(char *opt)
{
  if (!strcmp(opt,"-no_full"))
    fullscreen=i4_F;
}


int i4_win32_error(const char *st)
{

  static int died = 0;

  if (died)
  {
    i4_debug->printf("recursive error : %s", st);
    return 1;  
  }

  died = 1;

  MessageBox(0, st, "Error!", MB_OK | MB_APPLMODAL);
  
  exit(0);

  return 1;
}



static FILE *debug_file=0;


int i4_windows_warning_function(const char *st)
{
  OutputDebugString(st);
  OutputDebugString("\n");

  return 1;
}

class i4_win32_debug_stream_class : public i4_file_class
{
public:
  virtual w32 read (void *buffer, w32 size) { return 0; }

  virtual w32 write(const void *buffer, w32 size) 
  {
    if (debug_file)
    {
      fwrite(buffer, 1, size, debug_file);
      fflush(debug_file);
    }

    char b[1024];
    if (size>1023)
    {
      b[1]=0;
      for (int i=0; i<size; i++)
      {
        b[0]=*(((w8 *)buffer)+i);
        OutputDebugString(b);
      }
    }
    else
    {
      memcpy(b,buffer, size);
      b[size]=0;
      OutputDebugString(b);
    }
    return size;
  }

  virtual w32 seek (w32 offset) { return 0; }
  virtual w32 size () { return 0; }
  virtual w32 tell () { return 0; }
} win32_debug_stream;



void debug_init()
{
  i4_set_error_function(i4_win32_error);
  i4_set_warning_function(i4_windows_warning_function);
  i4_set_alert_function(i4_win32_alert);

  for (int i=1; i<i4_global_argc; i++)
  {
    if (i4_global_argv[i]=="-eout")
    {
      i++;
      char fn[100];
      i4_os_string(i4_global_argv[i], fn, 100);
      debug_file=fopen(fn, "wb");
    }
  }

  i4_debug=&win32_debug_stream;
}



class tmp_main_str : public i4_const_str
{
public:
  tmp_main_str();
  void set_ptr(char *_ptr)
  {
    ptr=_ptr;
    len=strlen(ptr); 
  }

};



int main(int argc, char **argv)
{ 
  long t;
  i4_stack_base=(void *)(&t);

  tmp_main_str *tmp=(tmp_main_str *)malloc(sizeof(tmp_main_str)*(argc+1));
  w32 i;
  for (i=0; i<argc; i++)
    tmp[i].set_ptr(argv[i]);

  i4_global_argc=argc;
  i4_global_argv=tmp;
  debug_init();
  i4_main(argc,tmp);

  free(tmp);
  
  return 1;
}

int PASCAL WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow)


{
  long t;
  i4_stack_base=(void *)(&t);


  /*
  char hacked_args[512];
  strcpy(hacked_args,"x:\\jc\\gdata\\obj\\test_text.gmod -no_full");
  */
  w32 count, white;
  char *s,*p,*st = strdup(lpCmdLine);

  s = st;
  count = 1;
  white = 1; 
  while (*s)
  {
    if (white)
    {
      if (*s != ' ')
      {
        count++;
        white = 0;
      }
      s++;
    }
    else
    {
      if (*s == ' ')
        white = 1;
      s++;
    }
  }

  tmp_main_str *tmp=(tmp_main_str *)malloc(sizeof(tmp_main_str)*(count+1));

  s = st;
  tmp[0].set_ptr("name");
  count = 1;
  white = 1; 
  while (*s)
  {
    if (white)
    {
      if (*s != ' ')
      {
        white = 0;
        p = s;
      }
      s++;
    }
    else
    {
      if (*s == ' ')
      {
        white = 1;
        *s = 0;

        i4_win32_startup_options.check_option(p);
          
        tmp[count].set_ptr(p);
        count++;
      }
      s++;
    }
  }
  if (!white)
  {
    i4_win32_startup_options.check_option(p);
    tmp[count].set_ptr(p);
    count++;
  }

  i4_win32_nCmdShow=nCmdShow;
  i4_win32_instance=hInstance;

  i4_global_argc=count;
  i4_global_argv=tmp;
  debug_init();
  i4_main(count, tmp);  
  free(tmp);

  PostQuitMessage( 0 );

  return 0;
}
