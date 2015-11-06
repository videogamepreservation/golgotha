/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "main/main.hh"
#include "file/file.hh"
#include "memory/malloc.hh"
#include "string/str_checksum.hh"
#include "threads/threads.hh"
#include <stdio.h>

int done=0;

void my_thread(void *arg)
{
  i4_directory_struct dir;

  i4_get_directory(i4_const_str("/tmp/ctext"), dir, i4_T);
  
  done=1;
}

void i4_main(w32 argc, i4_const_str *argv)
{
  i4_init(); 

  i4_add_thread(my_thread, 1024*1024, 0);
  int count=0;
  while (!done)
    count++;

  fprintf(stderr, "%d\n", count);


  i4_uninit();
}
