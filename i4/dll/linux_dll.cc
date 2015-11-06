/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "dll/dll.hh"
#include <dlfcn.h>

class i4_linux_dll_file : public i4_dll_file
{
  void *handle;
public:
  i4_linux_dll_file(void *handle) : handle(handle) {}

  virtual ~i4_linux_dll_file() 
  {
    dlclose(handle);
  }

  virtual void *find_function(const char *name)
  {
    return dlsym(handle, name);
  }  
};

i4_dll_file *i4_open_dll(const i4_const_str &filename)
{
  char buf[1024];
  i4_os_string(filename, buf, 1024);
  
  void *handle=dlopen(buf,  RTLD_NOW);
  if (handle)
    return new i4_linux_dll_file(handle);    
  else
  {
    i4_warning("DLL load %s failed : %s", buf, dlerror());
    return 0;
  }
  
}

