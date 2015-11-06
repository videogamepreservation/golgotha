/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef R1_TNODE_HH
#define R1_TNODE_HH

// this sturcture is preceeds the system (8bit pal) texture in memory


struct r1_texture_entry_struct;

typedef void *r1_local_texture_handle_type;

//#define R1_CHECK_TEXTURE_MEMORY

struct r1_texture_node_struct
{
  r1_texture_node_struct *lower;   // 0 if texture is lowest level mip, otherwise next lowest mip
  sw32 signed_size;
  
  sw32 size() { return -signed_size; }
  void set_size(sw32 size) { signed_size=-size; }

  r1_texture_entry_struct *entry;  // pointer to entry in entry table 
  r1_texture_node_struct *higher;  // higher mip level, or 0 if this is the highest loaded


#ifdef R1_CHECK_TEXTURE_MEMORY
  w32 check_sum;
#endif

public:

  r1_local_texture_handle_type vram_handle;  // handle vram installed texture

  i4_bool is_loaded_in_vram() { return vram_handle!=0; }

  sw32 last_used;                           // last time this was used by the game

  //  i4_float u_correct, v_correct;

  // this stuff needs to be at the end of the structure becuase 
  // it's read in by the async read (don't forget to convert byte-order after read)
  w16 w,h;
  w32 next_size;


  w16 width() { return w; }
  w16 height() { return h; }
};


#endif
