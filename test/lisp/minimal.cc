/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "main/main.hh"
#include "init/init.hh"
#include "lisp/lisp.hh"

void i4_main(w32 argc, i4_const_str *argv)
{
  i4_init();
  
  li_load("minimal.scm");  // loads & evaluates from current directory

  i4_uninit();
}

