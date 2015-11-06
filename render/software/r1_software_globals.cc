/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "software/r1_software.hh"
#include "software/r1_software_globals.hh"

r1_software_class r1_software_class_instance;

//global information about the current rendering buffer

i4_bool r1_software_render_buffer_is_locked = i4_F;
w16    *r1_software_render_buffer_ptr       = 0; //pointer to buffer memory
sw32    r1_software_render_buffer_bpl       = 0; //bytes per line
sw32    r1_software_render_buffer_wpl       = 0; //words per line
sw32    r1_software_render_buffer_height    = 0;
w8      r1_software_render_expand_type      = 0; //expanding type

//the directdraw surface which "owns" the buffer
IDirectDrawSurface3 *r1_software_render_surface = 0;

//global texturemap information (describes the current
//texturemap being used to draw polygons)

w16 *r1_software_texture_ptr    = 0;
w8   r1_software_twidth_log2    = 0;
sw32 r1_software_texture_width  = 0;  
sw32 r1_software_texture_height = 0;

//------------------------
//rest of global variables
//------------------------

sw32  left_l;

sw32  left_s;
sw32  left_t;
float left_z;

sw32  right_s;
sw32  right_t;
float right_z;

sw32 dsdx_frac,dtdx_frac;
sw32 temp_dsdx,temp_dtdx;

sw32  s_t_carry[2];
sw32  dldx_fixed;
sw32  num_subdivisions;

sw32 s_mask;// = ((r1_software_texture_width -1)<<16) | 0xFFFF;
sw32 t_mask;// = ((r1_software_texture_height-1)<<16) | 0xFFFF;

float ooz_right;
float soz_right;
float toz_right;

w32 pre_blend_and;
w32 post_blend_and;

sw32  width_global;  
sw32  num_leftover;

//cur_grads is a global set of gradients used by the scanline texturemappers
//(it needs to be global so that no extra pointer referencing occurs in the asm code)
tri_gradients cur_grads;

//the 4444 alpha lookup table (2^4 * 2^4 * 2^4 = 2^12 = 4096)
w16 alpha_table[4096];

w8 small_poly_type;
w8 big_poly_type;

//we probably only need 16 of these lookups
float inverse_leftover_lookup[64];

void inverse_leftover_lookup_init()
{
  sw32 i;
  
  inverse_leftover_lookup[0] = 0.f;

  for (i=1; i<64; i++)
  {
    inverse_leftover_lookup[i] = 1.f / (float)i;
  }
}

//unfortunately there is no decent sorted line primitive. all lines get
//drawn at the end of the frame, if there are any.

software_line software_lines[max_soft_lines];
sw32          num_buffered_lines=0;

tri_area_struct triangle_info[32];
float           total_poly_area;

//fpu control word storage (used in inline_fpu.hh)
w16 old_ceil_word;
w16 new_ceil_word;

w16 old_trunc_word;
w16 new_trunc_word;

w16 old_single_word;
w16 new_single_word;

i4_bool do_amd3d = i4_F;