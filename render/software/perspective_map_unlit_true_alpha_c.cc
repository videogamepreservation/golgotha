/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "software/r1_software_globals.hh"
#include "software/inline_fpu.hh"

void texture_scanline_perspective_unlit_true_alpha(w16 *start_pixel,
                                                   sw32 start_x,
                                                   void *_left,//perspective_span *left,
                                                   sw32 width)
{
  //temporary stuff for lighting calculations  
  w32 accumulated_alpha = (1<<12);

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
        w16 alpha_texel = *(r1_software_texture_ptr + (left_s>>16) + ((left_t>>16)<<r1_software_twidth_log2));
                
        w32 l_lookup_1 = (alpha_texel & (w16)(15<<12)) >> (8-NUM_LIGHT_SHADES_LOG2);
        w32 l_lookup_2 = l_lookup_1 ^ (NUM_LIGHT_SHADES<<8);

        w16 texel = alpha_table[alpha_texel & 4095];

        //lookup low bits
        w16 t1 = ((w32 *)(software_color_tables))[l_lookup_1 + (texel & 0xFF)];

        //lookup high bits
        w16 t2 = ((w32 *)(software_color_tables)+ctable_size)[l_lookup_1 + (texel>>8)];

        texel = *start_pixel;

        //lookup low bits
        w16 t3 = ((w32 *)(software_color_tables))[l_lookup_2 + (texel & 0xFF)];

        //lookup high bits
        w16 t4 = ((w32 *)(software_color_tables)+ctable_size)[l_lookup_2 + (texel>>8)];
    
        *start_pixel = (w16)(t1+t2+t3+t4);

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
  
        num_leftover--;
      }
    }
    else
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
    }
  }
}
