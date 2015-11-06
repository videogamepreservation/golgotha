/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "software/r1_software_globals.hh"
#include "software/inline_fpu.hh"

void texture_scanline_affine_unlit_true_alpha(w16 *start_pixel,
                                              sw32 start_x,
                                              void *_left,//perspective_span *left,
                                              sw32 width)
{
  w32 accumulated_alpha = (1<<12);

  start_pixel = (w16 *)((w8 *)start_pixel + start_x);

  affine_span *left = (affine_span *)_left;
    
  left_s = left->s;
  left_t = left->t;

  width_global = width;

  while (width_global)
  {
    register w32 alpha_texel = *(r1_software_texture_ptr + (left_s>>16) + ((left_t>>16)<<r1_software_twidth_log2));
                
    register w32 l_lookup = (alpha_texel & (w16)(15<<12)) >> (8-NUM_LIGHT_SHADES_LOG2);

    if (l_lookup)
    {
      register w32 texel = (w32)alpha_table[alpha_texel & 4095];

      //lookup low bits
      register w32 output_texel = ((w32 *)(software_color_tables))[l_lookup + (texel & 0xFF)];

      //lookup high bits
      output_texel += ((w32 *)(software_color_tables)+ctable_size)[l_lookup + (texel>>8)];

      l_lookup ^= (NUM_LIGHT_SHADES<<8);
      
      if (l_lookup)
      {
        texel       = (w32)*start_pixel;

        //lookup low bits
        output_texel += ((w32 *)(software_color_tables))[l_lookup + (texel & 0xFF)];

        //lookup high bits
        output_texel += ((w32 *)(software_color_tables)+ctable_size)[l_lookup + (texel>>8)];
      }

      *start_pixel = (w16)(output_texel);
    }
    
    start_pixel++;

    left_s += temp_dsdx;
    left_t += temp_dtdx;

    width_global--;
  }
}
