/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "device/processor.hh"
#include <string.h>

void i4_win32_get_cpu_info(i4_cpu_info_struct *s);

void i4_get_cpu_info(i4_cpu_info_struct *s)
{
  if (!s)
    return;

  s->cpu_type  = i4_cpu_info_struct::NON_X86;
  s->cpu_flags = 0;

#if (__linux)
  s->cpu_type  = i4_cpu_info_struct::UNKNOWN_X86;
  s->cpu_flags = 0;
#endif

#if (_WINDOWS)
  i4_win32_get_cpu_info(s);
#endif
}
