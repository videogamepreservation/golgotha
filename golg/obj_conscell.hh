/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef OBJ_CONSCELL_HH
#define OBJ_CONSCELL_HH

#include "arch.hh"
#include "g1_limits.hh"
#include "memory/lalloc.hh"



class g1_object_class;
class g1_obj_conscell_class;

class g1_obj_conscell_class
{
public:
  g1_object_class *data;
  g1_obj_conscell_class *next;

};


class g1_obj_conscell_manager_class : public i4_linear_allocator
{
  enum { MAXIMUM_OCCUPANCY=G1_MAX_OBJECTS*4 * 2/3 };

public:
  g1_obj_conscell_manager_class()
    : i4_linear_allocator(sizeof(g1_obj_conscell_class),
                          MAXIMUM_OCCUPANCY,0,"g1_conscells") {}
};

extern g1_obj_conscell_manager_class *g1_obj_conscell_manager;

#endif
