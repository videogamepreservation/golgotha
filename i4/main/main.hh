/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef __MAIN_HPP_
#define __MAIN_HPP_

#include "arch.hh"
class i4_const_str;
extern void *i4_stack_base;   // bottom of stack when program started

extern w32           i4_global_argc;
extern i4_const_str *i4_global_argv;


extern void i4_main(w32 argc, i4_const_str *argv);


#endif
