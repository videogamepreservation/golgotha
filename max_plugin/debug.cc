/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include <string.h>
#include <stdarg.h>
#include "debug.hh"

m1_debugfile dbg;

m1_debugfile::m1_debugfile(char *_name)
{
  if (!_name)
    _name = "c:\\tmp\\debug.out";
  strcpy(name,_name); 
  f = fopen(name,"wt");
}

m1_debugfile::~m1_debugfile()
{
  if (f)
    fclose(f);
}

void m1_debugfile::restart()
{
  if (f)
    fclose(f);
  f = fopen(name,"wt");
}

void m1_debugfile::printf(char *format, ...)
{
  va_list arg;
  char str[40960];
  
  va_start(arg, format);
  vsprintf(str, format, arg);
  fputs(str, f);
  fflush(f);
  va_end(arg);
}

void m1_debugfile::operator()(char *format,...)
{
  va_list arg;
  char str[40960];
  
  va_start(arg, format);
  vsprintf(str, format, arg);
  fputs(str, f);
  fflush(f);
  va_end(arg);
}

