/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef MAX_LOAD_HH
#define MAX_LOAD_HH

#include "max_object.hh"
#include "tmanage.hh"

extern m1_poly_object_class *m1_load_model(const i4_const_str &name,
                                           r1_texture_manager_class *tman);
                                    
#endif
