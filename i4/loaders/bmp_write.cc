/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "loaders/bmp_write.hh"
#include "image/image.hh"
#include "palette/pal.hh"
#include "file/file.hh"

inline void i4_32bpp_to_rgb(w32 color, w8 &r, w8 &g, w8 &b)
{
  r=(color>>16)&0xff;
  g=(color>>8)&0xff;
  b=color&0xff;
}

i4_bool i4_write_bmp(i4_image_class *im, i4_file_class *fp)
{
  const i4_pal *pal=im->get_pal();

  if (pal->source.pixel_depth==I4_8BIT)
  {
    w32 w=im->width();
    w32 h=im->height();
    w32 store_width=(w+3)&(~3);
    sw32 i;

    w8 buf[10];
    buf[0]='B';    buf[1]='M';

    if (fp->write(buf,2)!=2) return i4_F;
    fp->write_32(54 + 
                 4*256 +
                 store_width * h);

    fp->write_32(0);
    fp->write_32(54 + 256*4);

    fp->write_32(40);
    fp->write_32(w);
    fp->write_32(h);
    fp->write_16(1);
    fp->write_16(8);
    fp->write_32(0);
    fp->write_32(0);
    fp->write_32(320);
    fp->write_32(200);
    fp->write_32(256);
    fp->write_32(256);

    w32 *p=pal->source.lookup;
    for (i=0; i<256; i++)
    {
      w8 r,g,b;
      i4_32bpp_to_rgb(*p,r,g,b);
     
      buf[0]=b;
      buf[1]=g;
      buf[2]=r;
      buf[3]=0;

      fp->write(buf,4);
      p++;
    }
  
    buf[0]=0;
    buf[1]=0;
    buf[2]=0;
                 
    
    w8 *data=(w8 *)im->data;
    for (i=h; i; i--)
    {
      fp->write(data, w);
      data-=w;

      if (w!=store_width)
        fp->write(buf,w&3);
    }
    return i4_T;
  }
  else
    return i4_F;
}

