/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "math/random.hh"
#include "tick_count.hh"

w32 g1_rand(int seed)
{
  i4_rand_reset(g1_tick_counter + seed);
  return i4_rand();
}


float g1_float_rand(int seed)
{
  i4_rand_reset(g1_tick_counter + seed);
  return i4_float_rand();
}
