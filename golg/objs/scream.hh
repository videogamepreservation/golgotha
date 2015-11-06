/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef OBJS_SCREAM_HH
#define OBJS_SCREAM_HH

#include "math/num_type.hh"

enum g1_scream_type
{ G1_SCREAM_QUICK,
  G1_SCREAM_PAINFUL,
  G1_SCREAM_MOCK
};
       

void g1_play_scream(i4_float x, i4_float y, i4_float h, g1_scream_type type);
                    

#endif
