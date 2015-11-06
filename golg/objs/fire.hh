/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_FIRE_HH
#define G1_FIRE_HH


#include "math/vector.hh"

class g1_object_class;      // golg/g1_object.hh
class li_symbol;            // lisp/li_types.hh

g1_object_class *g1_fire(li_symbol *fire_type,
                         g1_object_class *who_is_firing,
                         g1_object_class *who_firing_at,
                         const i4_3d_vector &starting_position,
                         const i4_3d_vector &initial_direction_of_projectile,
                         g1_object_class *last_object_fired=0);  // for continous stream type

#endif
