/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "main/main.hh"
#include "string/string.hh"
#include "error/error.hh"
#include "file/file.hh"

#include <stdlib.h>


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

w32           i4_global_argc;
i4_const_str *i4_global_argv;


void *i4_stack_base=0;

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
  i4_main(argc,tmp);
  free(tmp);
  
  return 1;
}
