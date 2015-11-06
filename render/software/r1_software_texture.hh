/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef __R1_SOFTWARE_TEXTURE_HH__
#define __R1_SOFTWARE_TEXTURE_HH__

#include "memory/lalloc.hh"
#include "tmanage.hh"
#include "software/r1_software.hh"
#include "tex_heap.hh"

class r1_mip_load_info;

class r1_software_texture_class : public r1_texture_manager_class
{
  friend class r1_software_class;
  friend void software_async_callback(w32 count, void *context);

  w8 *texture_heap;
  r1_texture_heap_class *tex_heap_man;

  class free_node
  {
  public:
    R1_TEX_HEAP_FREE_NODE_DATA
        
    i4_file_class *async_fp;
  };

  class used_node
  {
  public:
    R1_TEX_HEAP_USED_NODE_DATA
    //dont need any new members    
  };
  
public:

  void next_frame();

  void select_texture(r1_local_texture_handle_type handle, float &smul, float &tmul);

  r1_software_texture_class(const i4_pal *pal);

  ~r1_software_texture_class();

  void init();

  void uninit();

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
