/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef CARCASS_HH
#define CARCASS_HH

class g1_object_class;

g1_object_class *g1_create_carcass(g1_object_class *from,
                                  g1_quad_object_class *model,
                                  int ticks=30,
                                  int ticks_to_smoke=10,
                                  g1_quad_object_class *lod_model=0);

#endif
