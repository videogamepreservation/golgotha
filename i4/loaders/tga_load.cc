/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "loaders/load.hh"
#include "image/image.hh"
#include "status/status.hh"
#include "file/file.hh"
#include "memory/malloc.hh"
#include "error/error.hh"
#include "palette/pal.hh"

static inline w32 read_color(i4_file_class *fp, w8 bpp)
{
  w8 buf[4];

  if (bpp==32)
  {
    fp->read(buf,4);
    return (((w32)buf[3]<<24) |
            ((w32)buf[2]<<16) |
            ((w32)buf[1]<<8)  |
            ((w32)buf[0]));

  } else
  {
    fp->read(buf,3);
    return (((w32)buf[2]<<16) |
            ((w32)buf[1]<<8)  |
            ((w32)buf[0]));
  } 
}

class i4_tga_loader_class : public i4_image_loader_class
{
  public :
  // returns the maximum size you need to identify your file format
  virtual w16 max_header_size() { return 17; }
  virtual i4_bool recognize_header(w8 *buf)
  {
    if (buf[1]==0 &&                // no colormap
        buf[2]==2 || buf[2]==10 &&  // true color compressed or raw
        buf[16]==32 || buf[16]==24)  // 32 or 24 bpp 
      return i4_T;
    else 
      return i4_F;
  }


  // assume fp is at the start of the file
  virtual i4_image_class *load(i4_file_class *fp,
                               i4_status_class *status)
  {
    unsigned char color_map,im_type;

    /*
    w32 fsize = bs_fp->size();
  
    w8 *file_buffer = (w8 *)i4_malloc(bs_fp->size(),"tga ram file buffer");

    bs_fp->read(file_buffer,fsize);

    i4_ram_file_class *fp = new i4_ram_file_class(file_buffer,fsize);
    */
    w8 id_length=fp->read_8();
    color_map=fp->read_8();
    im_type=fp->read_8();

    if (color_map!=0)
    {
      return 0;
    }

    if (!(im_type==2 || im_type==10))
    { 
      return 0;
    }

    fp->read_16();  // skip color map info
    fp->read_16();
    fp->read_8();

    fp->read_16();  // skip origin x & y
    fp->read_16();


    w16 w=fp->read_16();   // image width & height
    w16 h=fp->read_16();

    w8  bpp=fp->read_8();
    w8  bits=fp->read_8();

    if (bpp!=32 && bpp!=24)
    {
      return 0;
    }

    fp->seek(fp->tell() + id_length);

    w32 *data=(w32 *)i4_malloc(w*h*4,"targa image");


    int x,y;
    unsigned char ctrl;

    w32 c,*sl;

    if (im_type==10)     // type 10 is compressed
    {    
      i4_error("Cannot read compressed targa files");

      for (y=0;y<h;y++)
      {
        if (status)
          status->update(y/(float)h);

        sl=data+(h-y-1)*w;   // get address of scan line

        for (x=0;x<w;)
        {
          ctrl=fp->read_8();  // control byte
          int npixels=(ctrl&(~0x80))+1;

          if (ctrl&0x80)         // run or raw?
          {
            c=read_color(fp,bpp);
            while (npixels--)
            {
              *sl=c;
              sl++;
              x++;
            }
          } else
          {
            while (npixels--)
            {
              c=read_color(fp,bpp);
              *sl=c;
              sl++;
              x++;
            }
          }
        }
      }    
    } else
    {
      for (y=0;y<h;y++)
      {
        if (status)
          status->update(y/(float)h);

        sl=data+(h-y-1)*w;   // get address of scan line

        for (x=0;x<w;x++)
        {
          *sl=read_color(fp,bpp);
          sl++;          
        }
      }

    }

    enum { REVERSE_HORZ=0x10,
           REVERSE_VERT=0x20 };

    if (bits & REVERSE_VERT)
    {
      w32 *s1=data, *s2=data+(h-1)*w;
      for (y=0; y<h/2; y++)
      {
        for (x=0; x<w; x++)
        {
          w32 c=s1[x];
          s1[x]=s2[x];
          s2[x]=c;
        }
        s2-=w;
        s1+=w;
      }
    }

    if (bits & REVERSE_HORZ)
    {
      w32 *s1=data, *s2=data+w-1;
      for (y=0; y<h; y++)
      {
        for (x=0; x<w; x++)
        {
          w32 c=*s1;
          *s1=*s2;
          *s2=c;
          s1++;
          s2--;
        }
        s2+=w;
      }
    }
    
    i4_pixel_format fmt;
    fmt.default_format();

    if (bpp==24)             // no alpha info in these images
    {
      fmt.alpha_mask=0;
      fmt.calc_shift();
    }
    
    i4_pal *pal=i4_pal_man.register_pal(&fmt);

    i4_image_class *im=i4_create_image(w,h,  // width & height
        pal,                            // palette (should be 32 bit by default)
        data,
        w*4);                            // bytes per line
    
    im->dont_free_data=i4_F;

    return im;
  }

} i4_tga_loader_instance;
