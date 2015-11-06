/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "software/r1_software_globals.hh"
#include "software/inline_fpu.hh"

void texture_scanline_perspective_unlit_holy(w16 *start_pixel,
                                             sw32 start_x,
                                             void *_left,//perspective_span *left,
                                             sw32 width)
{                  
  //temporary stuff for lighting calculations  
  w16 texel;  
  w32 t1,t2;
  w32 l_lookup;

  start_pixel = (w16 *)((w8 *)start_pixel + start_x);

  perspective_span *left = (perspective_span *)_left;
    
  left_z = 1.f / left->ooz;
  left_s = qftoi(left->soz * left_z) + cur_grads.s_adjust;
  left_t = qftoi(left->toz * left_z) + cur_grads.t_adjust;
  
  sw32 had_subdivisions = width & (~15);

  num_subdivisions = width >> 4;
  num_leftover     = width & 15;
  
  if (num_subdivisions)
  {
    ooz_right = left->ooz + (cur_grads.doozdxspan);
    soz_right = left->soz + (cur_grads.dsozdxspan);
    toz_right = left->toz + (cur_grads.dtozdxspan);
    
    right_z = 1.f / ooz_right;

    while (num_subdivisions)
    {

      right_s = qftoi(soz_right * right_z) + cur_grads.s_adjust;
      if (right_s < 0)
        right_s = 0;
      else
      if (right_s > s_mask)
        right_s = s_mask;

      right_t = qftoi(toz_right * right_z) + cur_grads.t_adjust;
      if (right_t < 0)
        right_t = 0;
      else
      if (right_t > t_mask)
        right_t = t_mask;

      temp_dsdx = (right_s - left_s) >> 4;
      temp_dtdx = (right_t - left_t) >> 4;

      if (num_subdivisions!=1)
      {
        ooz_right += (cur_grads.doozdxspan);
        soz_right += (cur_grads.dsozdxspan);
        toz_right += (cur_grads.dtozdxspan);
        
        right_z = 1.f / ooz_right;
      }

      width_global = 16;

      while (width_global)
      {
        texel = *( r1_software_texture_ptr + (left_s>>16) + ((left_t>>16) << r1_software_twidth_log2) );

        if (texel)
          *start_pixel = texel;

        start_pixel++;

        left_s += temp_dsdx;
        left_t += temp_dtdx;

        width_global--;
      }
  
      left_s = right_s;
      left_t = right_t;
          
      num_subdivisions--;    
    }
  }
    
  if (num_leftover)
  {        
    if (num_leftover > 1)
    {
      if (had_subdivisions!=0)
      {
        ooz_right += (cur_grads.doozdx * num_leftover);
        soz_right += (cur_grads.dsozdx * num_leftover);
        toz_right += (cur_grads.dtozdx * num_leftover);
    
        right_z = 1.f / ooz_right;
      }
      else
      {
        ooz_right = left->ooz + (cur_grads.doozdx * num_leftover);
        soz_right = left->soz + (cur_grads.dsozdx * num_leftover);
        toz_right = left->toz + (cur_grads.dtozdx * num_leftover);
    
        right_z = 1.f / ooz_right;
      }

      right_s = qftoi(soz_right * right_z) + cur_grads.s_adjust;
      if (right_s < 0)
        right_s = 0;
      else
      if (right_s > s_mask)
        right_s = s_mask;

      right_t = qftoi(toz_right * right_z) + cur_grads.t_adjust;
      if (right_t < 0)
        right_t = 0;
      else
      if (right_t > t_mask)
        right_t = t_mask;

      temp_dsdx = qftoi((float)(right_s - left_s) * inverse_leftover_lookup[num_leftover]);
      temp_dtdx = qftoi((float)(right_t - left_t) * inverse_leftover_lookup[num_leftover]);

      while (num_leftover)
      {
        texel = *(r1_software_texture_ptr + (left_s>>16) + ((left_t>>16)<<r1_software_twidth_log2));
          
        if (texel)
          *start_pixel = texel;

        start_pixel++;

        left_s += temp_dsdx;
        left_t += temp_dtdx;
  
        num_leftover--;
      }
    }
    else
    {
      *start_pixel = *(r1_software_texture_ptr + (left_s>>16) + ((left_t>>16)<<r1_software_twidth_log2));
    }
  }
}
