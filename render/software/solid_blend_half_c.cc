/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "software/r1_software_globals.hh"

void texture_scanline_solid_blend_half(w16 *start_pixel,
                                       sw32 start_x,
                                       void *left,//solid_blend_span *left,
                                       sw32 width)
{
  w16 color = (((solid_blend_span *)left)->color & pre_blend_and)>>1;

  start_pixel = (w16 *)((w8 *)start_pixel + start_x);

  while (width)
  {
    *start_pixel = color + ((*start_pixel & pre_blend_and)>>1);
    start_pixel++;
    
    width--;
  }
}

void texture_scanline_solid_true_alpha(w16 *start_pixel,
                                       sw32 start_x,
                                       void *left,//solid_blend_span *left,
                                       sw32 width)
{
  register w32 pixel;
  register w32 color = ((solid_blend_span *)left)->color;
  register w32 alpha = ((solid_blend_span *)left)->alpha & NUM_LIGHT_SHADES;

  if (!alpha)
    return;

  //lookup low bits
  register w32 output_color = ((w32 *)(software_color_tables))[alpha + (color & 0xFF)];

  //lookup high bits
  output_color += ((w32 *)(software_color_tables)+ctable_size)[alpha + (color>>8)];

  color = output_color;

  alpha ^= (NUM_LIGHT_SHADES<<8);
      
  if (alpha)
  {
    while (width)
    {
      output_color = color;

      pixel = (w32)(*start_pixel);

      //lookup low bits
      output_color += ((w32 *)(software_color_tables))[alpha + (pixel & 0xFF)];

      //lookup high bits
      output_color += ((w32 *)(software_color_tables)+ctable_size)[alpha + (pixel>>8)];

      *start_pixel = (w16)output_color;
      
      start_pixel++;
      width--;
    }
  }
  else
  {
    while (width)
    {
      *start_pixel = (w16)color;
      start_pixel++;
    
      width--;
    }
  }
}