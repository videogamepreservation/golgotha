/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "quantize/histogram.hh"
#include "error/error.hh"
#include "image/image.hh"
#include <memory.h>
#include "palette/pal.hh"
#include "file/file.hh"


i4_histogram_class::i4_histogram_class()
{
  memset(reference,0,sizeof(reference));
  memset(counts,0,sizeof(counts));
  tcolors=0;
  total_pixels=0;
}

void i4_histogram_class::add_image_colors(i4_image_class *im, int counts_per_pixel)
{
  if (im->get_pal()->source.pixel_depth!=I4_32BIT)
  {
    i4_warning("add_image_colors : image type not supported");
    return ;
  }
  
  

  w32 loop_count=im->width()*im->height();

  // this will iterate through all the pixels
  w32 *pixel=(w32 *)im->data;
  
  for (; loop_count; loop_count--)
  {
    i4_color c=*pixel;

    // convert the color to 16 bit
    c=  
      (((c&0xff0000) >> (16+3)) << (6+5)) |
      (((c&0x00ff00) >> (8+2))  << (5+0)) |
      (((c&0x0000ff) >> (0+3))  << (0+0)) ;

    increment_color(c,counts_per_pixel);

    // move to the next pixel in the image
    ++pixel;
  }

}

void i4_histogram_class::save(i4_file_class *fp)
{
  w32 i;
  fp->write_32(tcolors);
  fp->write_32(total_pixels);

  for (i=0; i<HIST_SIZE; i++)
  {
    reference[i]=s_to_lsb(reference[i]);
    counts[i]=l_to_lsb(counts[i]);
  }

  fp->write(reference, sizeof(reference));
  fp->write(counts, sizeof(counts));

  for (i=0; i<HIST_SIZE; i++)
  {
    reference[i]=s_to_lsb(reference[i]);
    counts[i]=l_to_lsb(counts[i]);
  }
}

void i4_histogram_class::load(i4_file_class *fp)
{
  w32 i;
  tcolors=fp->read_32();
  total_pixels=fp->read_32();

  fp->read(reference, sizeof(reference));
  fp->read(counts, sizeof(counts));

  for (i=0; i<HIST_SIZE; i++)
  {
    reference[i]=s_to_lsb(reference[i]);
    counts[i]=l_to_lsb(counts[i]);
  }
}
