/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "network/login.hh"
#include "string/string.hh"

#ifdef _WINDOWS
#include <windows.h>
#else
#include <unistd.h>
#endif


i4_str *i4_get_user_name()
{
  char buf[256];

#ifdef _WINDOWS
  DWORD s=sizeof(buf);
  GetUserName(buf, &s);
#else
  char *gl=getlogin();
  if (gl)
    strcpy(buf,gl);
  else
    strcpy(buf,"unknown");
#endif
  
  return i4_from_ascii(buf);
}



