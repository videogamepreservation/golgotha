/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef R1_TEXTURE_MANAGER_HH
#define R1_TEXTURE_MANAGER_HH


#include "arch.hh"
#include "tex_id.hh"
#include "math/num_type.hh"

#include "file/file.hh"
#include "palette/pal.hh"
#include "memory/array.hh"
#include "mip.hh"

class i4_image_class;
extern w32 R1_CHROMA_COLOR;

struct r1_texture_entry_struct
{
public:  
  w32 id; // checksum of filename 
  w8  flags;

  w32 average_color;

  r1_miplevel_t *mipmaps[R1_MAX_MIP_LEVELS+1]; //null terminated
  w32 file_offsets[R1_MAX_MIP_LEVELS];

  i4_bool is_transparent()
  {
    if (flags & R1_MIP_IS_TRANSPARENT)
      return i4_T;
    else
      return i4_F;
  }
  
  i4_bool is_alphatexture()
  {
    if (flags & R1_MIP_IS_ALPHATEXTURE)
      return i4_T;
    else
      return i4_F;
  }  
};


struct r1_texture_matchup_struct
{
  w32 id;
  w16 handle;
  sw16 left, right;
  char name[128];
};

struct r1_texture_animation_entry_struct
{
  w32 id;
  w16 total_frames;
  r1_texture_handle *frames;
};

class r1_texture_manager_class
{  
public:
  i4_bool textures_loaded;

  r1_texture_manager_class(const i4_pal *pal);

  virtual void init();
  virtual void uninit();  

  w32 average_texture_color(r1_texture_handle handle, w32 frame_num);

  void matchup_textures();

  r1_texture_handle find_texture(w32 id);

  i4_bool texture_resolution_change()
  {
    i4_bool ret = texture_resolution_changed;
    texture_resolution_changed  = i4_F;
    return ret;
  }

  void keep_resident(const i4_const_str &tname, sw32 desired_width);  

  char *get_texture_name(r1_texture_handle handle);
  r1_texture_handle register_texture(const i4_const_str &tname,
                                     const i4_const_str &error_string,
                                     i4_bool *has_been_loaded=0);

  r1_texture_handle register_image(i4_image_class *image);
  
  // loads textures previously registered with register_texture
  i4_bool load_textures();  
  
  w32 get_animation_length(r1_texture_handle handle)
  {
    if (handle>=0) return 1;
    else return tanims[-handle-1].total_frames;
  }

  virtual void next_frame();

  virtual void reset();

  virtual void toggle_texture_loading();
  
  virtual r1_miplevel_t *get_texture(r1_texture_handle handle,
                                     w32 frame_counter,
                                     sw32 desired_width,
                                     sw32 &w, sw32 &h);

  virtual i4_bool valid_handle(r1_texture_handle handle)
  {
    if (handle<registered_tnames.size() && registered_tnames[handle].handle != 0)
      return i4_T;
    else
      return i4_F;
  }

protected:

  void keep_cache_current(i4_array<w32> *file_ids);

  i4_bool build_cache_file(i4_array<w32> &texture_file_ids,
                           const i4_const_str &network_dir,
                           const i4_const_str &local_dir);

  i4_bool update_cache_file(i4_array<w32> &update_ids,
                            const i4_const_str &network_dir,
                            const i4_const_str &local_dir);

  i4_array<r1_texture_matchup_struct> registered_tnames;
  i4_array<r1_texture_entry_struct>   *entries;
  r1_texture_animation_entry_struct   *tanims;

  sw32 total_textures;
  sw32 total_tanims;
  
  //time keeper
  sw32 frame_count;

  i4_bool texture_load_toggle;

  i4_bool texture_resolution_changed;

  const i4_pal *pal;
  
  //////////////////////////// derive / specify these for each texture manager derivative
  i4_pixel_format regular_format;
  i4_pixel_format chroma_format;
  i4_pixel_format alpha_format;  
  
  sw32    min_texture_dimention;
  sw32    max_texture_dimention;
  i4_bool square_textures;
  
public:
  virtual i4_bool immediate_mip_load(r1_mip_load_info *load_info) = 0;

  virtual i4_bool async_mip_load(r1_mip_load_info *load_info) = 0;

  virtual void free_mip(r1_vram_handle_type vram_handle) = 0;          
};


#endif
