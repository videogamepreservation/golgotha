/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_SELECTION_HH
#define G1_SELECTION_HH

#include "arch.hh"

class g1_object_class;

class g1_selection_list
{
  g1_object_class **list;
  w32 t, list_size;

public:
  g1_selection_list();
  ~g1_selection_list();

  w32 total() { return t; }

  void add(g1_object_class *o);
  void remove(g1_object_class *o);

  void clear();  // removes all objects from list
};

#endif
