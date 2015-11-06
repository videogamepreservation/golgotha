/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef STRUCTURE_DEATH_HH
#define STRUCTURE_DEATH_HH

class g1_object_class;

class g1_structure_death_class
{
protected:
  g1_object_class *me;
public:
  g1_structure_death_class(g1_object_class *me) : me(me) {}
  i4_bool think();
  void damage(g1_object_class *obj, int hp, i4_3d_vector damage_dir);
};

#endif
