/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "loaders/load.hh"
#include "image/image.hh"
#include "error/error.hh"
#include "palette/pal.hh"
#include "status/status.hh"
#include "file/file.hh"
#include "memory/malloc.hh"
#include "error/error.hh"
#include "palette/pal.hh"

class i4_pcx_loader_class :  public i4_image_loader_class 
{
  public :
  w16 get_short(w32 offset, void *data) 
  {
    w8 buf[2];
    buf[0]=*(((w8 *)data)+offset);
    buf[1]=*(((w8 *)data)+offset+1);
    return (buf[1]<<8)|buf[0];
  }


  w16 max_header_size() { return 4; }

  virtual i4_bool recognize_header(w8 *buf)
  {
    if (buf[0]==10 &&   // manufacturer
        buf[2]==1  &&   // encoding
	buf[3]==8)      // bits per pixel
      return i4_T;
    else return i4_F;
  }


  int read_PCX8_line(i4_file_class *fp, w8 *start, sw16 skip, w16 w)
  {
    int n=0,i;    
    w8 c;
    do
    {
      if (!fp->read(&c,1)) return 0;
      if ((c&0xc0)==0xc0)
      {
	i=c&0x3f;
	if (!fp->read(&c,1)) return 0;	
	while (i--) 
	{
          *start=c;
	  start+=skip;
	  n++;	
	}
      }
      else        
      {
        *start=c;
	start+=skip;
	n++;
      }    
    } while (n<w);    
    return 1;
  }



  virtual i4_image_class *load(i4_file_class *fp,
                               i4_status_class *status)
  {
    w8 header[128];
    if (fp->read(header,128)!=128) return 0;
    w16 w=get_short(8,header)-get_short(4,header)+1;
    w16 h=get_short(10,header)-get_short(6,header)+1;
    if (w>5000 || h>5000)  
      i4_error("w(%d) or h(%d) proabbly bad",w,h);

    
    i4_image_class *im=i4_create_image(w,h, i4_pal_man.default_8());    
    w16 bpl=im->bpl;
    w8 *data=(w8 *)im->data;

    w16 y;
    for (y=0;y<h;y++,data+=bpl)
    {
      if (status)
        status->update(y/(float)h);

      if (!read_PCX8_line(fp,data,1,w))
      {
	delete im;
	return 0;
      }
    }

    w8 palette_confirm;  
    if (!fp->read(&palette_confirm,1) || !(palette_confirm==12))
    {    
      i4_warning("PCX load expect palette confirm to be 12, is this 256 color?");
      delete im;
      return 0;
    }
    else
    {
      w8 pal[256*3];
      if (fp->read(pal,256*3)!=256*3)
      {
	delete im;
	return 0;
      }
      w32 word_pal[256],x;
      for (x=0;x<256;x++) {
//         if (pal[x*3]==254 && pal[x*3+1]==2 && pal[x*3+2]==166) {
//           pal[x*3]=0;
//           pal[x*3+1]=0;
//           pal[x*3+2]=0;
//         }
        word_pal[x]=(pal[x*3]<<16)|(pal[x*3+1]<<8)|pal[x*3+2];
      }

      i4_pixel_format fmt;
      fmt.pixel_depth=I4_8BIT;
      fmt.lookup=word_pal;

      im->set_pal(i4_pal_man.register_pal(&fmt));
    }
    return im;  
  }

} i4_pcx_loader_instance;

