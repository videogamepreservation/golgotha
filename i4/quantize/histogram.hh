/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef HISTOGRAM_HH
#define HISTOGRAM_HH

#include "arch.hh"
#include "image/image.hh"

class i4_file_class;

class i4_histogram_class
{
  public :
  enum { HIST_SIZE=0x10000,  // 16 bit histogram table
         MAX_COLORS=256 };

  w16 reference[HIST_SIZE];
  w32 counts[HIST_SIZE];
  w32 tcolors;              // total original colors in the histogram [length of reference]
  w32 total_pixels;         // total pixels accounted for in image (affected by counts_per_pixel)


  i4_histogram_class();
  
  void increment_color(w16 color,  // this is expected to be a 16 bit color (5 6 5)
                       w32 count)
  {
    if (!counts[color])            // is this an original color?
    {
      reference[tcolors]=color;    // add this color to the reference list
      tcolors++;
    }
    counts[color]+=count;          // increment the counter for this color
    total_pixels+=count;           // count total pixels we've looked at
  }

  void reset()
  {
    for (int i=0; i<tcolors; i++)    
      counts[reference[i]]=0;
    tcolors=0;
    total_pixels=0;
  }

  void save(i4_file_class *fp);
  void load(i4_file_class *fp);

  // counts_per_pixel can be used to give smaller images more emphasis
  // I use this to give mip maps of original images more double emphasis
  // right now only image's of type i4_image32 will work
  void add_image_colors(i4_image_class *image, int counts_per_pixel=1);

} ;



#endif
