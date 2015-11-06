/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

class g1_object_class;

g1_object_class *g1_create_damager_object(const i4_3d_vector &pos,
                                          int damage_per_tick, int _ticks,
                                          g1_object_class *person_giving_damage,
                                          g1_object_class *person_being_damaged,
                                          li_symbol *smoke_type);
