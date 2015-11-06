/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_PLAYER_TYPE_HH
#define G1_PLAYER_TYPE_HH

// Currently the limit for # players in golgotha is 255.
// If this is changed g1_object load and save need reflect the new type
#include "arch.hh"

typedef w8 g1_player_type;
enum { g1_default_player = 0 };

#endif

