/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "image/image.hh"
#include "loaders/tga_write.hh"
#include "file/file.hh"
#include "palette/pal.hh"


i4_bool i4_tga_write(i4_image_class *im, i4_file_class *fp, int include_alpha)
{
  fp->write_8(0);   // no id field
  fp->write_8(0);   // no color map
  fp->write_8(2);   // type = True color uncompressed
  
  fp->write_16(0);  // first color map entry
  fp->write_16(0);  // color map length
  fp->write_8(0);   // color map entry size

  fp->write_16(0);  // origin x & y
  fp->write_16(0);

  sw32 w=im->width(), h=im->height(), x,y;
  fp->write_16(w);   // image width & height
  fp->write_16(h);

  if (include_alpha)
    fp->write_8(32);   // bits per pixel
  else
    fp->write_8(24);   // bits per pixel

  fp->write_8(0);


  w8 out[4];
  i4_color color;

  const i4_pal *pal=im->get_pal();
  i4_pixel_format to;
  to.default_format();

  if (include_alpha)
  {
    for (y=h-1; y>=0; y--)
    {
      for (x=0; x<w; x++)
      {
        color=im->get_pixel(x,y);
            
        out[0]=color&0xff;
        out[1]=(color>>8)&0xff;
        out[2]=(color>>16)&0xff;
        out[3]=(color>>24)&0xff;
        fp->write(out,4);
      }
    }
  }
  else
  {  
    for (y=h-1; y>=0; y--)
    {
      for (x=0; x<w; x++)
      {
        color=im->get_pixel(x,y);
            
        out[0]=color&0xff;
        out[1]=(color>>8)&0xff;
        out[2]=(color>>16)&0xff;
        fp->write(out,3);
      }
    }
  }

  return i4_T;
    
}
