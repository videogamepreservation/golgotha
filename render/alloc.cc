/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "tmanage.hh"
#include "threads/threads.hh"

/*
static i4_critical_section_class r1_texture_alloc_lock;

// Mmeory allocation might be called from a seperate thread so you should be aware 
// of what the main program sees in memory at each stage of allocation.
// Since Garbage collection & free's will not occur while the texture loading thread
// (async_read) is active, this should not present any problems.

r1_texture_node_struct *r1_texture_manager_class::free_node::alloc_some(sw32 alloc_size, 
                                                                        free_node *&last_ptr)
{
  alloc_size=(alloc_size+3)&(~3);
  r1_texture_node_struct *a=(r1_texture_node_struct *)this;
  
  // is there enough space to split the free node?
  if (alloc_size+sizeof(r1_texture_node_struct)+4>size)
  {
    a->set_size((sw32)size+(sw32)sizeof(free_node)-(sw32)sizeof(r1_texture_node_struct));
    last_ptr=next;

  }
  else
  {
    // first setup the new free node
    free_node *n=(free_node *)((w8 *)this+sizeof(r1_texture_node_struct)+alloc_size);
    n->next=next;
    n->size=(sw32)size-(sw32)sizeof(r1_texture_node_struct)-(sw32)alloc_size;

    a->set_size(alloc_size);

    // set the last pointer to the new me
    last_ptr=n;
  }

  return a;
}


r1_texture_node_struct *r1_texture_manager_class::alloc(sw32 size)
{  
  free_node *last=0, *next;
  r1_texture_node_struct *ret;
  
  r1_texture_alloc_lock.lock();
  
  if (!first_free)
  {
    r1_texture_alloc_lock.unlock();
    return 0;
  }
  
  for (free_node *f=first_free; f!=end; f=f->next)
  {
    if (f->size>=size+(sw32)sizeof(r1_texture_node_struct)-(sw32)sizeof(free_node))
    {
      if (last)
        ret=f->alloc_some(size, last->next);
      else  
        ret=f->alloc_some(size, first_free);

      ret->vram_handle=0;

      r1_texture_alloc_lock.unlock();
      return ret;
    }
    last=f;
  }
 
  r1_texture_alloc_lock.unlock();
  return 0;      
}

i4_bool r1_texture_manager_class::can_alloc(sw32 size)
{
  if (!first_free)
    return i4_F;

  for (free_node *f=first_free; f!=end; f=f->next)
        if (f->size>=size+(sw32)sizeof(r1_texture_node_struct)-(sw32)sizeof(free_node))
      return i4_T;
  return i4_F;
}

*/
