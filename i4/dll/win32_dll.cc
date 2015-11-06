/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "dll/dll.hh"
#include <windows.h>

class i4_win32_dll_file : public i4_dll_file
{

  HINSTANCE handle;
public:
  i4_win32_dll_file(HINSTANCE handle) : handle(handle) {}

  virtual ~i4_win32_dll_file() 
  {
    FreeLibrary(handle);
  }

  virtual void *find_function(const char *name)
  {
    return GetProcAddress(handle, name);
  }  
};

i4_dll_file *i4_open_dll(const i4_const_str &name)
{
  char buf[1024];
  HINSTANCE handle=LoadLibrary(i4_os_string(name, buf, 1024));
  if (handle)
    return new i4_win32_dll_file(handle);    
  else return 0;
}


