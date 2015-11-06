/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "software/r1_software_globals.hh"
#include "software/inline_fpu.hh"

void texture_scanline_affine_unlit(w16 *start_pixel,
                                   sw32 start_x,
                                   void *_left,//perspective_span *left,
                                   sw32 width)
{
  start_pixel = (w16 *)((w8 *)start_pixel + start_x);

  affine_span *left = (affine_span *)_left;
    
  left_s = left->s + cur_grads.s_adjust;
  left_t = left->t + cur_grads.t_adjust;

  temp_dsdx = qftoi(cur_grads.dsdx);
  temp_dtdx = qftoi(cur_grads.dtdx);

  width_global = width;

  while (width_global)
  {
    *start_pixel = *(r1_software_texture_ptr + (left_s>>16) + ((left_t>>16)<<r1_software_twidth_log2));
                
    start_pixel++;

    left_s += temp_dsdx;
    left_t += temp_dtdx;

    width_global--;
  }
}
