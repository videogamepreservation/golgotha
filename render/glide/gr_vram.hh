/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef __GLIDE_VRAM_HH
#define __GLIDE_VRAM_HH

#include "memory/lalloc.hh"
#include "memory/array.hh"
#include "tmanage.hh"
#include "tex_heap.hh"

// this class keeps track of what's where in 3dfx texture memory


class r1_glide_vram_class : public r1_texture_manager_class
{
  friend void glide_async_callback(w32 count, void *context);

  class free_node
  {
  public:
    R1_TEX_HEAP_FREE_NODE_DATA    
    i4_float smul, tmul;    
    
    //used when loading asynchronously from disk
    i4_file_class *async_fp;
    w8 *data; 

    w8 lod, aspect, tmu;
  };

  class used_node
  {
  public:
    R1_TEX_HEAP_USED_NODE_DATA
    //no extra members for this class
  };

  enum {scratch_texture_size = 256*256*2};

  w8 scratch_texture[scratch_texture_size]; // used (shared) to load in textures off disk  

  r1_texture_heap_class *tex_heap_man;
  r1_texture_heap_class *tex_heap_man_2;
  int heap1_address;
  int heap2_address;
  
public:

  void next_frame();

  void select_texture(r1_vram_handle_type handle, 
                      float &smul, float &tmul, i4_bool holy, i4_bool alpha);


  r1_glide_vram_class(const i4_pal *pal);

  void init();
  void uninit();

  ~r1_glide_vram_class()
  {
    uninit();
  }

  i4_bool immediate_mip_load(r1_mip_load_info *load_info);
  i4_bool async_mip_load(r1_mip_load_info *load_info);

  void free_mip(r1_vram_handle_type vram_handle);

  r1_miplevel_t *get_texture(r1_texture_handle handle,
                             w32 frame_counter,
                             sw32 desired_width,
                             sw32 &w, sw32 &h);

  i4_critical_section_class array_lock;  
  
  i4_array<used_node *> finished_array;
  
  void async_load_finished(used_node *u);

  sw32 bytes_loaded;
  sw32 textures_loaded;
};


#endif
