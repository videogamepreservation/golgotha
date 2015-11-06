/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "tex_heap.hh"
#include "tex_no_heap.hh"

i4_profile_class pf_tex_heap_alloc("tex_heap::alloc()");
i4_profile_class pf_tex_heap_free("tex_heap::free()");
i4_profile_class pf_tex_heap_cleanup("tex_heap::cleanup()");

i4_profile_class pf_tex_no_heap_alloc("tex_heap::alloc()");
i4_profile_class pf_tex_no_heap_free("tex_heap::free()");
i4_profile_class pf_tex_no_heap_cleanup("tex_heap::cleanup()");


r1_texture_heap_class::~r1_texture_heap_class()
{
  //kill the free list
  r1_tex_heap_free_node *next_free,*last_free;

  next_free = first_free;
  while (next_free)
  {
    last_free = next_free;
    next_free = next_free->next;
      
    if (last_free->mip)
    {
      last_free->mip->vram_handle = 0;
    }      
    free_node_alloc->free(last_free);
  }
  first_free = 0;

    //kill the used list
  r1_tex_heap_used_node *next_used,*last_used;
  
  next_used = first_used;
  while (next_used)
  {
    last_used = next_used;
    next_used = next_used->next;
      
    if (last_used->node->mip->vram_handle)
    {
      last_used->node->mip->vram_handle = 0;
    }
    free_node_alloc->free(last_used->node);
    used_node_alloc->free(last_used);
  }
  first_used = 0;

  oldest_used = 0;
  //both lists are cleared. kill the allocaters

  delete free_node_alloc;
  free_node_alloc=0;

  delete used_node_alloc;
  used_node_alloc=0;
}

r1_texture_heap_class::r1_texture_heap_class(w32 heap_size, w32 heap_start, w32 free_node_size, w32 used_node_size, sw32 *frame_count)
{
  num_ram_misses  = 0;
  needs_cleanup   = i4_F;

  free_node_alloc = 0;
  used_node_alloc = 0;

  frame_count_ptr = frame_count;
   
  if (!free_node_alloc)
  {
    free_node_alloc   = new i4_linear_allocator(free_node_size, 2048, 1024, "texture heap free nodes");
    first_free        = (r1_tex_heap_free_node *)free_node_alloc->alloc();
    first_free->size  = heap_size;
    first_free->start = heap_start;
    first_free->next  = 0;
    first_free->mip   = 0;
  }

  if (!used_node_alloc)
  {
    used_node_alloc  = new i4_linear_allocator(used_node_size, 2048, 1024, "texture heap used nodes");
    first_used       = 0;
    oldest_used      = 0;
  }
}


void r1_texture_heap_class::free_really_old()
{
  pf_tex_heap_cleanup.start();    

  w32 total_freed = 0;    
    
  r1_tex_heap_used_node *u = oldest_used;
  r1_tex_heap_used_node *last;
    
  while (u && total_freed < max_fail_size)
  {      
    sw32 lfu = u->node->mip->last_frame_used;
    sw32 age = *frame_count_ptr - lfu;

    last = u->last;

    if ((lfu != -1) && (age > 10))
    {        
      total_freed += u->node->size;        

      free(u);
    }
      
    u = last;
  }        

//   if (total_freed < max_fail_size)      
//     i4_warning("Texture cleanup: freed %d bytes (not enough)",total_freed);
//   else
//     i4_warning("Texture cleanup: freed %d bytes (successful)",total_freed);
    
  max_fail_size = 0;
  needs_cleanup = i4_F;

  pf_tex_heap_cleanup.stop();
}


  
r1_tex_heap_used_node *r1_texture_heap_class::alloc(w32 need_size, w8 flags)
{
  pf_tex_heap_alloc.start();

  //ok. we know how much memory we need, now try to find a free area of memory
  r1_tex_heap_free_node *f    = first_free;
  r1_tex_heap_free_node *last = 0;
  
  while (f && f->size<need_size)
  {
    last=f;
    f=f->next;
  }

  r1_tex_heap_used_node *new_used = 0;

  if (!f)
  {
    //no memory large enough. dig through and see if there is a used chunk that we can free
    r1_tex_heap_used_node *u = oldest_used;
      
    while (u)
    {
      if (u->node->size >= need_size)
      {          
        r1_miplevel_t *m = u->node->mip;
        if (m->last_frame_used!=-1 && m->last_frame_used < (*frame_count_ptr - 2))
        {
          if (m->flags & R1_MIPLEVEL_IS_LOADING)
          {
            i4_warning("loading miplevel is in used list. bad.");              
          }
          break;
        }
      }      
      u = u->last;
    }
    
    //couldnt find a used one that we could free
    if (!u)
    {
      num_ram_misses++;
        
      if (need_size > max_fail_size)
        max_fail_size = need_size;

      if ((num_ram_misses & 15) == 0)
      {
        //cleanup old textures every 16 misses
        needs_cleanup = i4_T;
      }

      pf_tex_heap_alloc.stop();
      return 0;
    }
      
    new_used = u;
      
    f = new_used->node;

    //kill the old mip's reference to it
    f->mip->vram_handle = 0;

    if (f->size > need_size)
    {
      //need to merge unused space back into free list        
       
      merge_into_free_list(f->start + need_size, f->size - need_size);
        
      f->size = need_size;
    }

    if (flags & R1_TEX_HEAP_DONT_LIST)
    {
      if (new_used==first_used)
        first_used = new_used->next;        
      if (new_used==oldest_used)
        oldest_used = new_used->last;

      //this flag is set for asynchronously loaded textures
      //we dont even want this node to be listed. it will be added
      //when the async load is finished
      if (new_used->next) new_used->next->last = new_used->last;
      if (new_used->last) new_used->last->next = new_used->next;                

      new_used->next=0;
      new_used->last=0;
    }
  }
  else
  {
    // remove the this node from the free list      
    if (!last)
      first_free = f->next;
    else
      last->next = f->next;

    //if we dont match the size exactly, need to create a new free node to fill in the space
    if (f->size != need_size)
    {
      r1_tex_heap_free_node *new_free = (r1_tex_heap_free_node *)free_node_alloc->alloc();
      
      if (!new_free)
      {
        i4_warning("alloc of tex_heap free_node failed");
        pf_tex_heap_alloc.stop();
        return 0;
      }

      new_free->size  = f->size  - need_size;
      new_free->start = f->start + need_size;
      new_free->mip=0;
   
      if (last)
        last->next = new_free;
      else
        first_free = new_free;
  

      new_free->next = f->next;
      f->size = need_size;
    }
      
    new_used = (r1_tex_heap_used_node *)used_node_alloc->alloc();
    
    //f is the node we'll use
    //add into the used_list    
    
    if (!new_used)
    {
      i4_warning("alloc of tex_heap used_node failed");
      pf_tex_heap_alloc.stop();
      return 0;        
    }

    new_used->node = f;
      
    if (flags & R1_TEX_HEAP_DONT_LIST)
    {
      new_used->next=0;
      new_used->last=0;
    }
    else
    {
      new_used->next = first_used;  
      new_used->last = 0;
    
      if (first_used)
        first_used->last = new_used;          

      first_used = new_used;
    }
  }
    
  pf_tex_heap_alloc.stop();
  return new_used;
}

void r1_texture_heap_class::merge_into_free_list(w32 _start, w32 _size)
{    
  r1_tex_heap_free_node *next, *last;

  // find proper spot to add into free list
  next = first_free; last=0;
  for (; next && next->start < _start;)
  {
    last = next;
    next = next->next;
  }

  // can we combine with the last node?
  if (last && last->start+last->size == _start)
  {
    last->size += _size;
      
    // does this also combine with the next node?
    if (next && (last->start + last->size == next->start) )
    {
      last->size += next->size;
      last->next =  next->next;
  
      free_node_alloc->free(next);
    }        
  }
  else
    if (next && (_start + _size == next->start) )  // does it combine with the next block?
    {
      next->start = _start;
      next->size += _size;                  
    }
    else
      if (last) // put after the last proper node
      {
        r1_tex_heap_free_node *f = (r1_tex_heap_free_node *)free_node_alloc->alloc();

        f->size  = _size;
        f->start = _start;
        f->next  = last->next; //(last->next should be NULL)
        f->mip   = 0;

        last->next = f;
      }
      else // put it at the begining of the list
      {
        r1_tex_heap_free_node *f = (r1_tex_heap_free_node *)free_node_alloc->alloc();
      
        f->size  = _size;
        f->start = _start;
        f->next  = first_free;
        f->mip   = 0;
        first_free = f;      
      }    
}

void r1_texture_heap_class::update_usage(r1_tex_heap_used_node *u)
{
  if (u->node->mip->last_frame_used==-1)
    return ;
    

  u->node->mip->last_frame_used = *frame_count_ptr;
      
  if (u != first_used)
  {
    //put this node at the front of the used list
    //has the effect of sorting all used nodes according to
    //their age (most recently used are at the
    //front of the list)

    if (u==oldest_used)
      oldest_used = u->last;

    if (u->next) u->next->last = u->last;    
    if (u->last) u->last->next = u->next;
  
    u->last = 0;
    u->next = first_used;
      
    if (first_used)
      first_used->last = u;

    first_used = u;
  }
    
  //update the oldest used pointer
  if (!oldest_used)
    oldest_used = u;
  else
  {
    if ((u->node->mip->last_frame_used != -1) &&
        (u->node->mip->last_frame_used < oldest_used->node->mip->last_frame_used))
    {
      oldest_used = u;    
    }
  }
}

void r1_texture_heap_class::free(r1_tex_heap_used_node *u)
{    
  if (!u || !u->node)
  {
    i4_warning("tex_heap_class free : handle is 0");
    return;
  }
      
  if (u->node->mip->flags & R1_MIPLEVEL_IS_LOADING)
  {
    i4_warning("free called on a mip that was still loading");
  }

  pf_tex_heap_free.start();

  r1_tex_heap_free_node *f = u->node, *next, *last;
    
  //give its r1_miplevel_t an invalid vram handle
  f->mip->vram_handle = 0;
  
  //process these cases 1st
  if (u==first_used)    
    first_used = u->next;    
    
  if (u==oldest_used)    
    oldest_used = u->last;

  //pull it from the used_list    
  if (u->next) u->next->last = u->last;
  if (u->last) u->last->next = u->next;          

  used_node_alloc->free(u);
  //end of pulling from the used list
  
  // find proper spot to add into free list
  next = first_free; last=0;
  for (; next && next->start<f->start;)
  {
    last=next;
    next=next->next;
  }

  // can we combine with the last node?
  if (last && last->start+last->size==f->start)
  {
    last->size += f->size;
      
    // does this also combine with the next node?
    if (next && (last->start + last->size == next->start) )
    {
      last->size += next->size;
      last->next =  next->next;
  
      free_node_alloc->free(next);
    }
  
    free_node_alloc->free(f);
  }
  else
    if (next && (f->start + f->size == next->start) )  // does it combine with the next block?
    {
      next->start = f->start;
      next->size += f->size;   
      
      free_node_alloc->free(f);      
    }
    else
      if (last)              // put after the last proper node
      {
        f->next = last->next;
        last->next = f;
      }
      else                       // put it at the begining of the list
      {
        f->next = first_free;
        first_free = f;      
      }
    
  pf_tex_heap_free.stop();
}
