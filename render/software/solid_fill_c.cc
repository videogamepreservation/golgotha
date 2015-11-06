/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "software/r1_software_globals.hh"

void texture_scanline_solid_fill(w16 *start_pixel,
                                 sw32 start_x,
                                 void *left,//solid_blend_span *left,
                                 sw32 width)
{
  w16 color = ((solid_blend_span *)left)->color;

  start_pixel = (w16 *)((w8 *)start_pixel + start_x);

  while (width)
  {
    *start_pixel = color;
    start_pixel++;
    
    width--;
  }
}