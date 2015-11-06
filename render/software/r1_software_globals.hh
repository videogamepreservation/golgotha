/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef R1_SOFTWARE_GLOBALS
#define R1_SOFTWARE_GLOBALS

extern i4_bool r1_software_render_buffer_is_locked;
extern w16 *r1_software_render_buffer_ptr;
extern w8   r1_software_render_expand_type;
extern sw32 r1_software_render_buffer_bpl;
extern sw32 r1_software_render_buffer_wpl;
extern sw32 r1_software_render_buffer_height;

extern IDirectDrawSurface3 *r1_software_render_surface;

extern w16 *r1_software_texture_ptr;
extern w8   r1_software_twidth_log2;
extern sw32 r1_software_texture_width;  
extern sw32 r1_software_texture_height;

//for the ASM and C texturemappers

extern float left_z;
extern sw32  left_s;
extern sw32  left_t;
extern sw32  left_l;

extern float right_z;
extern sw32  right_s;
extern sw32  right_t;

extern sw32 s_t_carry[2];
extern sw32 dldx_fixed;

extern sw32 dsdx_frac,dtdx_frac;
extern sw32 temp_dsdx,temp_dtdx;

extern sw32 s_mask;// = ((r1_software_texture_width -1)<<16) | 0xFFFF;
extern sw32 t_mask;// = ((r1_software_texture_height-1)<<16) | 0xFFFF;

extern float ooz_right;
extern float soz_right;
extern float toz_right;

//for blending scanline functions
extern w32   pre_blend_and;
extern w32   post_blend_and;

extern sw32  width_global;
extern sw32  num_subdivisions;
extern sw32  num_leftover;

void inverse_leftover_lookup_init();
extern float inverse_leftover_lookup[];

extern tri_gradients cur_grads;

extern w16 alpha_table[];

extern software_line software_lines[];
extern sw32          num_buffered_lines;

enum {max_soft_lines = 1024};

extern tri_area_struct triangle_info[];
extern float total_poly_area;

extern w8 small_poly_type;
extern w8 big_poly_type;

//fpu control word storage (used in inline_fpu.hh)
extern w16 old_ceil_word;
extern w16 new_ceil_word;

extern w16 old_trunc_word;
extern w16 new_trunc_word;

extern w16 old_single_word;
extern w16 new_single_word;

extern i4_bool do_amd3d;

#endif