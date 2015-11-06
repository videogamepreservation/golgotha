/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/


/* 
   Grow heap will only grow, it never shrinks.
   It should not be used for allocating large blocks.  And
   the largest allocation request should be less than grow_increment.

 */

#ifndef __GROWHEAP_HPP_
#define __GROWHEAP_HPP_

#include "memory/malloc.hh"
#include "isllist.hh"

class i4_grow_heap_class
{
public:
  class heap
  {
  public:
    heap *next;
    void *data;
    heap(w32 size, char *name) { data=i4_malloc(size,name); name=0; }
    ~heap() { i4_free(data); }
  } ;

  typedef i4_isl_list<heap>           heap_list;
  typedef i4_isl_list<heap>::iterator heap_iter;

  heap_list list;
  w32 current_offset,increment,current_size;

  i4_grow_heap_class(w32 initial_size, w32 grow_increment);
  void *malloc(w32 size, char *description);           // description not used right now
  ~i4_grow_heap_class() { list.destroy_all(); }
  void clear();
} ;


#endif
