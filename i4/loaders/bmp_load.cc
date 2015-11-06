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

#include <string.h>
#include <stdio.h>

class i4_bmp_loader_class : public i4_image_loader_class 
{

  public :
  w16 get_short(w32 offset, void *data) 
  {
    w8 buf[2];
    buf[0]=*(((w8 *)data)+offset);
    buf[1]=*(((w8 *)data)+offset+1);
    return (buf[1]<<8)|buf[0];
  }

  w32 get_long(w32 offset, void *data) 
  {
    w8 buf[4];
    buf[0]=*(((w8 *)data)+offset);
    buf[1]=*(((w8 *)data)+offset+1);
    buf[2]=*(((w8 *)data)+offset+2);
    buf[3]=*(((w8 *)data)+offset+3);
    return (buf[3]<<24)|(buf[2]<<16)|(buf[1]<<8)|buf[0];
  }


  public :
  virtual w16 max_header_size() { return 2; }
  virtual i4_bool recognize_header(w8 *buf) { return (i4_bool)(buf[0]=='B' && buf[1]=='M'); }
  virtual i4_image_class *load(i4_file_class *fp,
                               i4_status_class *status)
  {
    void *buf[54];
    if (fp->read(buf,54)!=54) return 0;
    w32 width   =get_long (18,buf);

    //width=(width+3)&(~3);

    w32 fsize   =get_long (2,buf);
    w32 offset  =get_long (10,buf);
    w32 height  =get_long (22,buf);
    w16 planes  =get_short(26,buf);
    w16 bits    =get_short(28,buf);
    w32 compress=get_long (30,buf);
    w32 colors  =get_long (46,buf);


    if (bits==8 && planes==1)
    {
      w32 pdata[256],i;

      colors = (colors>0)? colors : 256;
      memset(pdata,0,sizeof(pdata));
      for (i=0;i<colors;i++)
      {
        w8 b[4];
        fp->read(b,4);
        pdata[i]=(b[2]<<16)|(b[1]<<8)|b[0];
      }

      i4_pixel_format fmt;
      fmt.pixel_depth=I4_8BIT;
      fmt.lookup=pdata;

      const i4_pal *pal=i4_pal_man.register_pal(&fmt);
      i4_image_class *im=i4_create_image(width,height,pal);
      
      if (compress==0)
      {
        int end_of_line_skip=(fsize-54-colors*4)/height - width;
        fp->seek(offset);

        for (i=0;i<height;i++)  
        {
          if (status)
            status->update(i/(float)height);

          w32 read_amount=fp->read((w8 *)im->data+(height-i-1)*width,width);
          if (read_amount!=width)
          {
            i4_warning("bmp file short by %d scan_lines & %d bytes\n",
                       height-i-1,
                       width-read_amount);
            delete im;
            return 0;
          }

          if (end_of_line_skip)   // each row is 0-padded to a 4-byte boundary
            fp->read(buf, end_of_line_skip);
          
        }
        return im;
      } else if (compress==1)
      {
        i4_bool done=i4_F;
        w8 *d=(w8 *)im->data;
        w32 x=0,y=height-1;

        while (!done)
        {
          w8 pair[2];
          fp->read(pair,2);
          if (*pair==0)                   // escaped
          {         
            if (pair[1]==0)               // end of line
            {
              x=0;
              y--;
              d=(w8 *)im->data+y*width;

              if (status)
                status->update((height-y)/(float)height);


            } else if (pair[1]==1)        // end of bitmap
              done=i4_T;
            else if (pair[1]==2)          // goto xy
            {
              fp->read(pair,2);
              x+=pair[0];

              y-=pair[1];
              d=(w8 *)im->data+y*width+x;                   
            } else         // run of data (even padded)
            {             
              fp->read(d,pair[1]);
             
              if (pair[1]&1)
                fp->read(pair,1);

              d+=pair[1];
              x+=pair[1];
            }
          } else                          // run of colors
          {
            memset(d,pair[1],pair[0]);
            d+=pair[0];
            x+=pair[0];
          }
        }
        return im;
      }
      else
        delete im;
    } else if (bits==24 && planes==1)
    {
      i4_pixel_format fmt;
      fmt.default_format();

      fmt.alpha_mask=0;           // no alpha info
      fmt.calc_shift();

    
      w16 x,y;
      const i4_pal *pal32=i4_pal_man.register_pal(&fmt);
      i4_image_class *im=i4_create_image(width,height,pal32);
      memset(im->data, 0, width*height*4);

      w32 *sl=(w32 *)((w8*)im->data + (height-1)*im->bpl);

      int end_of_line_skip=(fsize-54)/height - width*3;
      fp->seek(offset);

      for (y=height;y;y--)
      {
        if (status)
          status->update((height-y)/(float)height);


        for (x=0;x<width;x++)
        {
          w8 d[3];            
          if (fp->read(d,3)!=3)
          {
            delete im;
            return 0;
          }
          // d[0] == blue
          // d[1] == green
          // d[2] == red

          *sl=d[0]|(d[1]<<8)|(d[2]<<16);
          ++sl;
        }
        sl+=(-(width*2));

        if (end_of_line_skip)   // each row is 0-padded to a 4-byte boundary
          fp->read(buf, end_of_line_skip);



      }
      return im;

    }

    return 0;
  }
} bmp_loader_instance;


