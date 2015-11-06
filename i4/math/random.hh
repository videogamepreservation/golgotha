/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef __I4_RANDOM_HPP_
#define __I4_RANDOM_HPP_

#include "arch.hh"
#include <stdlib.h>
w32 i4_rand();
float i4_float_rand();   // returns between 0..1
void i4_rand_reset(w32 seed);


#endif
