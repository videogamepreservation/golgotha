/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef LI_ALLOC_HH
#define LI_ALLOC_HH

#include "arch.hh"
class li_list;
class li_symbol;
class li_object;

void *li_cell8_alloc();
void li_cell8_free(void *ptr);

int li_gc();
i4_bool li_valid_object(li_object *o);
extern int li_max_cells;               // set this before i4_init to change limits

// if you have li_object's that are not referenced by the stack and by a symbol,
// and you do not what them to be garbage collected, you must derive one of these
// call mark() on all the objects you reference

class li_gc_object_marker_class
{
public:
  li_gc_object_marker_class *next;
  virtual void mark_objects(int set) { ; }
  li_gc_object_marker_class();      // automatically adds to list
  ~li_gc_object_marker_class();
};


#endif
