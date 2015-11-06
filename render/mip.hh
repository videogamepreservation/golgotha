/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef _MIP_HH_
#define _MIP_HH_

#define R1_MAX_MIP_LEVELS  8

#include "arch.hh"
#include "file/file.hh"
#include <stdio.h>

typedef void *r1_vram_handle_type;

inline int r1_mip_header_disk_size() { return 4 * R1_MAX_MIP_LEVELS + 4 +32+2+2+1+1; }

struct mipheader_t
{
  sw32 offsets[R1_MAX_MIP_LEVELS];
  w32  average_color;
  char tname[32];
  sw16 base_width,base_height;
  w8   flags;
  sw8  num_mip_levels;

  void write(i4_file_class *fp)
  {
    for (int i=0; i<R1_MAX_MIP_LEVELS; i++)
      fp->write_32(offsets[i]);
    fp->write_32(average_color);
    fp->write(tname, 32);
    fp->write_16(base_width);
    fp->write_16(base_height);
    fp->write_8(flags);
    fp->write_8(num_mip_levels);
  }

  void read(i4_file_class *fp)
  {
    for (int i=0; i<R1_MAX_MIP_LEVELS; i++)
      offsets[i]=fp->read_32();
    average_color=fp->read_32();
    fp->read(tname, 32);
    base_width=fp->read_16();
    base_height=fp->read_16();
    flags=fp->read_8();
    num_mip_levels=fp->read_8();
  }
};

#define R1_MIP_IS_TRANSPARENT      1
#define R1_MIP_IS_ALPHATEXTURE     2
#define R1_MIP_IS_JPG_COMPRESSED  64

//use this only temporarily. isnt necessarily maintained
#define R1_MIP_EXTRA_FLAG                32

class r1_miplevel_t;
class r1_texture_entry_struct;

#define R1_MIP_LOAD_NO_ROOM 1
#define R1_MIP_LOAD_BUSY    2

class r1_mip_load_info
{
public:  
  r1_miplevel_t *dest_mip;
  i4_file_class *src_file;
  w8             error;
};

#define R1_MIPLEVEL_IS_LOADING 1

class r1_miplevel_t
{
public:
  r1_miplevel_t::r1_miplevel_t()
  {
    level=0;
    width=height=0;
    vram_handle=0;
    entry=0;    
    last_frame_used=0;
    flags=0;
  }

  //every time get_texture() is called and this miplevel is returned,
  //the current frame # is copied into last_frame_used
  sw32 last_frame_used;  
  
  r1_texture_entry_struct *entry;
  r1_vram_handle_type vram_handle;  //this is just a void * (defined above)

  sw16 width,height; //not technically necessary, can be recalculated via entry and level
  w8   level;
  w8   flags;    
};


struct i4_pixel_format;
void r1_setup_decompression(i4_pixel_format *reg_fmt,
                            i4_pixel_format *chroma_fmt,
                            i4_pixel_format *alpha_fmt,
                            w32 chroma_color,
                            i4_bool square_textures);

void r1_end_decompression();

i4_bool r1_decompress_to_local_mip(i4_file_class *src_file,
                                   i4_file_class *dst_lowest_mip,
                                   char *network_file,
                                   char *local_file,                                   
                                   mipheader_t *old_mipheader,
                                   sw32 max_mip_dimention);

#endif
