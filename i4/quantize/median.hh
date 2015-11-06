/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef _MEDIAN_HPP_
#define _MEDIAN_HPP_


// median cut finds a good 256 palette to fit a 32 bit image
// this is returned as an array of 256 32bit words.  This
// array should be freed using delete

class i4_histogram_class;

void i4_median_cut(i4_histogram_class *hist, 
                   w32 skip_colors,
                   w32 t_colors,
                   w32 *return_palette);
#endif
