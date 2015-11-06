/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef _TEX_HEAP_HH_
#define _TEX_HEAP_HH_

#include "time/profile.hh"

extern i4_profile_class pf_tex_heap_alloc;
extern i4_profile_class pf_tex_heap_free;
extern i4_profile_class pf_tex_heap_cleanup;

#define R1_TEX_HEAP_FREE_NODE_DATA w32 start,size; r1_tex_heap_free_node *next; r1_miplevel_t *mip;

#define R1_TEX_HEAP_USED_NODE_DATA r1_tex_heap_free_node *node; r1_tex_heap_used_node *next; r1_tex_heap_used_node *last;

#define R1_TEX_HEAP_DONT_LIST 1

class r1_tex_heap_free_node
{
public:
  R1_TEX_HEAP_FREE_NODE_DATA   
};

class r1_tex_heap_used_node
{
public:
  R1_TEX_HEAP_USED_NODE_DATA    
};


class r1_texture_heap_class
{
public:
  
  r1_tex_heap_free_node *first_free;  
  r1_tex_heap_used_node *first_used;
  r1_tex_heap_used_node *oldest_used;

  i4_linear_allocator *free_node_alloc;
  i4_linear_allocator *used_node_alloc;

  r1_texture_heap_class(w32 heap_size, w32 heap_start, w32 free_node_size, w32 used_node_size, sw32 *frame_count);
  ~r1_texture_heap_class();

  sw32 *frame_count_ptr;
  sw32 num_ram_misses;
  sw32 max_fail_size;
  
  i4_bool needs_cleanup;

  void free_really_old();
  void update_usage(r1_tex_heap_used_node *u);
  
  r1_tex_heap_used_node *alloc(w32 need_size, w8 flags=0);

  void merge_into_free_list(w32 _start, w32 _size);
  void free(r1_tex_heap_used_node *u);
};

#endif
