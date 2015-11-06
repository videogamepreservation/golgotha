/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef MK_ERROR_HH
#define MK_ERROR_HH

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

extern int line_on;
extern char *file_on;
extern char *file_contents;
extern int mk_debug_on;

void mk_error(char *format,...);
void mk_debug(char *format,...);

#define assert(cond,mess) if (!(cond)) mk_error("%s:%d - %s\n",__FILE__,__LINE__,mess)

#endif
