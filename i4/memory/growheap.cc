/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "memory/growheap.hh"
#include "error/error.hh"

i4_grow_heap_class::i4_grow_heap_class(w32 initial_size, w32 grow_increment)
//{{{
{
  increment=grow_increment;

  if (initial_size)
  {
    heap *h=new heap(initial_size,"initial grow heap");
    list.insert(*h);
    current_size=initial_size;
    current_offset=0;
  } 
  else 
  {
    current_size=0;
    current_offset=0;
  }
}
//}}}


void *i4_grow_heap_class::malloc(w32 size, char *description)           
//{{{
// description not used right now
{
  if (size==0) 
    return 0;

  if (current_offset+size<current_size)
  {
    void *ret=(void *)(((w8 *)list.begin()->data)+current_offset);
    current_offset+=size;
    return ret;
  } else
  {
    if (size>increment)
      i4_error("grow heap param problem");

    heap *h=new heap(increment,"grow heap");
    list.insert(*h);
    current_size=increment;
    current_offset=0;
    return malloc(size,description);
  }
}
//}}}


void i4_grow_heap_class::clear()
//{{{
{
  heap_iter p = list.begin();
  heap_iter q(p);

  current_offset=0;

  ++p;

  while (p != list.end())
  {
    list.erase_after(q);
    delete &*p;
    p = q;
    ++p;
  }
  

}
//}}}


//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}


