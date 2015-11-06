/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "error/alert.hh"
#include <stdio.h>


extern FILE *i4_error_mirror_file;

int i4_default_alert(const i4_const_str &ret)
//{{{
{
#ifdef __MAC__
  printf("Alert : ");
  i4_const_str::iterator s=ret.begin();
  while (s!=ret.end())
  {
    printf("%c",s.get().value());
    ++s;
  }
  printf("\n");
#else
  fprintf(stderr,"Alert : ");
  i4_const_str::iterator s=ret.begin();
  while (s!=ret.end())
  {
    fprintf(stderr,"%c",s.get().value());
    ++s;
  }
  fprintf(stderr,"\n");
#endif

  return 0;
}
//}}}

i4_alert_function_type i4_alert_function=i4_default_alert;

void i4_alert(const i4_const_str &format, w32 max_length, ...)
{
  va_list ap;
  va_start(ap, max_length);

  i4_str *ret=format.vsprintf(500,ap);  
  (*i4_alert_function)(*ret);

#if defined(WIN32)
  if (!i4_error_mirror_file)
    i4_error_mirror_file = fopen("error.out","wt");
  if (i4_error_mirror_file)
  {
    fprintf(i4_error_mirror_file,"Alert : ");
    i4_const_str::iterator s=ret->begin();
    while (s!=ret->end())
    {
      fprintf(i4_error_mirror_file,"%c",s.get().value());
      ++s;
    }
    fprintf(i4_error_mirror_file,"\n");
  }
#endif

  delete ret;
  va_end(ap);
}


void i4_set_alert_function(i4_alert_function_type fun)
{
  i4_alert_function=fun;
}


