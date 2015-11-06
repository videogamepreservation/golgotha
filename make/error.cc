/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "error.hh"


int line_on=0;
char *file_on=0;
int mk_debug_on=0;
char *file_contents=0;

void mk_error(char *format,...)
{
  if (file_on)
    fprintf(stderr, "%s:%d: ", file_on, line_on);


  va_list args;

  va_start(args,format);
  vfprintf(stderr, format, args);
  va_end(args);

  exit(1);
}

void mk_debug(char *format,...)
{
  if (mk_debug_on)
  {
    va_list args;

    va_start(args,format);
    vfprintf(stderr, format, args);
    va_end(args);

  }
}
