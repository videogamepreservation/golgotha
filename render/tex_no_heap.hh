/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef _TEX_NO_HEAP_HH_
#define _TEX_NO_HEAP_HH_

#include "time/profile.hh"

extern i4_profile_class pf_tex_no_heap_alloc;
extern i4_profile_class pf_tex_no_heap_free;
extern i4_profile_class pf_tex_no_heap_cleanup;

#define R1_TEX_NO_HEAP_USED_NODE_DATA r1_miplevel_t *mip; r1_tex_no_heap_used_node *next; r1_tex_no_heap_used_node *last;

#define R1_TEX_NO_HEAP_DONT_LIST 1

class r1_tex_no_heap_used_node
{
public:
  R1_TEX_NO_HEAP_USED_NODE_DATA    
};

class r1_texture_no_heap_class
{
public:  
  r1_tex_no_heap_used_node *first_used;
  r1_tex_no_heap_used_node *oldest_used;
  
  i4_linear_allocator *used_node_alloc;

  r1_texture_no_heap_class(r1_texture_manager_class *tman, w32 used_node_size, w32 *frame_count)
  {
    tmanager = tman;

    num_ram_misses  = 0;
    needs_cleanup   = i4_F;
    
    used_node_alloc = 0;

    frame_count_ptr = frame_count;
       
    if (!used_node_alloc)
    {
      used_node_alloc  = new i4_linear_allocator(used_node_size, 2048, 1024, "texture non-heap used nodes");
      first_used       = 0;
      oldest_used      = 0;
    }
  }

  ~r1_texture_no_heap_class()
  {
    //kill the used list
    r1_tex_no_heap_used_node *u,*next_used;
  
    u = first_used;
    while (u)
    {
      next_used = u->next;      
      
      tmanager->free_mip(u);
      
      u = next_used;      
    }
    
    first_used  = 0;
    oldest_used = 0;

    delete used_node_alloc;
    used_node_alloc=0;
  }

  r1_texture_manager_class *tmanager;
  w32 *frame_count_ptr;
  w32  num_ram_misses;
  sw32 max_fail_size;
  
  i4_bool needs_cleanup;

  void free_really_old()
  {
    pf_tex_no_heap_cleanup.start();    

    w32 total_freed = 0;    
    
    r1_tex_no_heap_used_node *u = oldest_used;
    r1_tex_no_heap_used_node *last;
    
    while (u && total_freed < max_fail_size)
    {      
      sw32 lfu = u->mip->last_frame_used;
      sw32 age = *frame_count_ptr - lfu;

      last = u->last;

      if ((lfu != -1) && (age > 10))
      {        
        total_freed += (u->mip->width*u->mip->height*2);

        tmanager->free_mip(u);
      }
      
      u = last;
    }        

#ifdef DEBUG
    if (total_freed < max_fail_size)      
      i4_warning("Texture cleanup: freed %d bytes (not enough)",total_freed);
    else
      i4_warning("Texture cleanup: freed %d bytes (successful)",total_freed);
#endif
    
    max_fail_size = 0;
    needs_cleanup = i4_F;

    pf_tex_no_heap_cleanup.stop();
  }
  
  r1_tex_no_heap_used_node *alloc(w8 flags=0)
  {
    r1_tex_no_heap_used_node *u = (r1_tex_no_heap_used_node *)used_node_alloc->alloc();
    
    if (flags & R1_TEX_NO_HEAP_DONT_LIST)
    {
      u->next = 0;
      u->last = 0;
    }
    else
    {
      u->next = first_used;  
      u->last = 0;
    
      if (first_used)
        first_used->last = u;          

      first_used = u;
    }
    
    return u;
  }

  r1_tex_no_heap_used_node *alloc_from_used(w32 need_size, w8 flags=0)
  {
    pf_tex_no_heap_alloc.start();

    //we know how much memory we need, now try to find an old texture of the same or greater size

    //no memory large enough. dig through and see if there is a used chunk that we can free
    r1_tex_no_heap_used_node *u = oldest_used;
      
    while (u)
    {
      if (u->mip->width*u->mip->height*2 >= need_size)
      {          
        r1_miplevel_t *m = u->mip;
        if (m->last_frame_used < (*frame_count_ptr - 2))
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

      pf_tex_no_heap_alloc.stop();
      return 0;
    }
    
    //kill the old mip's reference to it
    u->mip->vram_handle = 0;

    //free it
    tmanager->free_mip(u);

    u = alloc(flags);
    
    pf_tex_no_heap_alloc.stop();
    return u;
  }  

  void missed_one(sw32 miss_size)
  {
    if (miss_size > max_fail_size)
      max_fail_size = miss_size;
  }

  void update_usage(r1_tex_no_heap_used_node *u)
  {
    if ((u->mip->last_frame_used) != -1)
    {
      u->mip->last_frame_used = *frame_count_ptr;
      
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
    }
    
    //update the oldest used pointer
    if (!oldest_used)
      oldest_used = u;
    else
    {
      if (oldest_used->mip->last_frame_used==-1)
        oldest_used = u;
      else
      {
        if ((u->mip->last_frame_used != -1) &&
            (u->mip->last_frame_used < oldest_used->mip->last_frame_used))
        {
          oldest_used = u;    
        }
      }
    }
  }

  void free(r1_tex_no_heap_used_node *u)
  {
    if (!u)
    {
      i4_warning("tex_no_heap_class free : handle is 0");
      return;
    }
    
    if (u->mip->flags & R1_MIPLEVEL_IS_LOADING)
    {
      i4_warning("free called on a mip that was still loading");
    }

    pf_tex_no_heap_free.start();
    
    //give its r1_miplevel_t an invalid vram handle
    u->mip->vram_handle = 0;
  
    //process these cases 1st
    if (u==first_used)
      first_used = u->next;
    if (u==oldest_used)
      oldest_used = u->last;

    //pull it from the used_list    
    if (u->next) u->next->last = u->last;
    if (u->last) u->last->next = u->next;          

    used_node_alloc->free(u);
    
    pf_tex_no_heap_free.stop();
  }

};

#endif