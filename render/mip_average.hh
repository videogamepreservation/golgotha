/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef MIP_AVERAGE_HH
#define MIP_AVERAGE_HH

struct i4_pixel_format;
#include "math/num_type.hh"
#include "time/profile.hh"

//regular shifting info
extern w32 mip_r_and; extern w8 mip_r_shift; //r and g shifts are to the left
extern w32 mip_g_and; extern w8 mip_g_shift; 
extern w32 mip_b_and; extern w8 mip_b_shift; //b shift is to the right

//chroma-key shifting info
extern w32 mip_c_r_and; extern w8 mip_c_r_shift; //r and g shifts are to the left
extern w32 mip_c_g_and; extern w8 mip_c_g_shift; 
extern w32 mip_c_b_and; extern w8 mip_c_b_shift; //b shift is to the right
extern w32 mip_c_a_and; extern w8 mip_c_a_shift; //a shift is to the left

//alpha texture shifting info
extern w32 mip_a_r_and; extern w8 mip_a_r_shift; //r and g shifts are to the left
extern w32 mip_a_g_and; extern w8 mip_a_g_shift; 
extern w32 mip_a_b_and; extern w8 mip_a_b_shift; //b shift is to the right
extern w32 mip_a_a_and; extern w8 mip_a_a_shift; //a shift is to the left

extern w8 mip_chroma_red,mip_chroma_green,mip_chroma_blue;

extern i4_profile_class pf_make_next_mip;
extern i4_profile_class pf_mip_24_to_16;

void setup_pixel_formats(i4_pixel_format &reg_fmt,
                         i4_pixel_format &chroma_fmt,
                         i4_pixel_format &alpha_fmt,
                         w32 chroma_color);

inline void remap_pixel(w16 *&dst, w8 *&src)
{
  //convert 24bit RGB to 16bit RGB
  
  //shifts must go first, ands last  
  *dst = (((w16)src[0] << mip_r_shift) & mip_r_and) |
         (((w16)src[1] << mip_g_shift) & mip_g_and) |
         (((w16)src[2] >> mip_b_shift) & mip_b_and);
}

inline void chroma_remap_pixel(w16 *&dst, w8 *&src)
{
  //convert 24bit RGB to argb  
  
  if (src[0]==mip_chroma_red && src[1]==mip_chroma_green && src[2]==mip_chroma_blue)
  {
    *dst = 0;
  }
  else
  {
    //alpha bit(s) is set if its not transparent
    *dst = (((w16)src[0] << mip_c_r_shift) & mip_c_r_and) |
           (((w16)src[1] << mip_c_g_shift) & mip_c_g_and) |
           (((w16)src[2] >> mip_c_b_shift) & mip_c_b_and) |          
           ((0xFFFFFFFF  << mip_c_a_shift) & mip_c_a_and);
  }
}

inline void alpha_remap_pixel(w16 *&dst, w8 *&src)
{
  //convert 32bit ARGB to 16bit ARGB  
  
  //alpha bit(s) is set if its not transparent
  *dst = (((w16)src[1] << mip_a_r_shift) & mip_a_r_and) |
         (((w16)src[2] << mip_a_g_shift) & mip_a_g_and) |
         (((w16)src[3] >> mip_a_b_shift) & mip_a_b_and) |          
         (((w16)src[0] << mip_a_a_shift) & mip_a_a_and);  
}

void r1_dos_paths(char *src);

void r1_unix_paths(char *src);

char *r1_remove_paths(char *src);

void r1_remove_extention(char *src);

void r1_concatenate_path(char *src, char *path);

void chroma_process(w8 *&p, float &r, float &g, float &b, sw8 &num_regular, sw8 &num_chroma);

void average_4x4_chroma(w16 *&dst, w8 *dst_24, w8 *&p0, w8 *p1, w8 *p2, w8 *p3);

void average_4x4_chroma(w8 *dst_24, w8 *&p0, w8 *p1, w8 *p2, w8 *p3);

void average_4x4_alpha(w8 *dst_24, w8 *&p0, w8 *p1, w8 *p2, w8 *p3);

void average_4x4_alpha(w16 *dst, w8 *dst_24, w8 *&p0, w8 *p1, w8 *p2, w8 *p3);

void average_4x4_normal(w16 *dst, w8 *dst_24, w8 *p0, w32 bpl);

void average_4x4_normal(w8 *dst_24, w8 *p0, w32 bpl);

void mip_24_to_16(w8 *mip24, w16 *mip16, sw32 &width, 
                         sw32 &height, sw32 &base_width, w8 &flags);

void make_next_mip(w8 *mip24, w16 *mip16, sw32 &width, sw32 &height, 
                          sw32 &base_width, w8 &flags);

void make_square(w8 *mip24, w8 *square_mip24, w8 base_pixel_size, sw32 base_width, 
                        sw32 base_height, sw32 new_dimention);

void generate_mip_offsets(sw32 base_width,sw32 base_height,
                          sw32 num_to_copy, 
                          sw32 *offsets, sw32 pixel_size);

#endif
