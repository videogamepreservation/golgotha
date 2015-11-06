/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "memory/malloc.hh"
#include "image/image32.hh"
#include "image/image8.hh"
#include "palette/pal.hh"
#include "error/error.hh"
#include <string.h>

i4_color i4_image32::get_pixel(i4_coord x, i4_coord y)
{    
  return i4_pal_man.convert_to_32(*paddr(x,y), pal);
}
  
void i4_image32::put_pixel(i4_coord x, i4_coord y, w32 color)
{
  *paddr(x,y)=i4_pal_man.convert_32_to(color, &pal->source);
}

i4_image32::i4_image32(w16 _w, w16 _h, const i4_pal *_pal)
{
  w=_w;
  h=_h;
  bpl=_w*4;
  set_pal(_pal);
  data=i4_malloc(w*h*4,"");  
}

i4_image32::i4_image32(w16 _w, w16 _h, const i4_pal *_pal,
                       void *_data, int _bpl)
{
  data=_data;
  bpl=_bpl;
  pal=_pal;
  w=_w;
  h=_h;

  dont_free_data=i4_T;
}


i4_image32::~i4_image32()
{
  if (!dont_free_data)
    i4_free(data);
}

inline w32 color_dist(w32 c1, w32 c2)
{
  sw32 r1=(c1 & 0xff0000)>>16;
  sw32 r2=(c2 & 0xff0000)>>16;

  sw32 g1=(c1 & 0xff00)>>8;
  sw32 g2=(c2 & 0xff00)>>8;

  sw32 b1=(c1 & 0xff);
  sw32 b2=(c2 & 0xff);

  return (r1-r2)*(r1-r2) + (g1-g2)*(g1-g2) + (b1-b2)*(b1-b2);
}

inline w16 _32_to_16_565(w32 c)
{
  w8 r = ((c&0xff0000)>>16)>>3;

  w8 g = ((c&0xff00)>>8)>>2;

  w8 b = (c&0xff)>>3;

  return (r<<11) | (g<<5) | b;    
}


i4_image_class *i4_image32::quantize(const i4_pal *pal,
                                     w32 skip_colors,
                                     i4_coord x1, i4_coord y1,
                                     i4_coord x2, i4_coord y2)
{

  if (pal->source.pixel_depth!=I4_8BIT)
    i4_error("palette handle, should be 8 bit");

  // first check to make sure the sub image is located within this one
  if (x1<0) x1=0;
  if (y1<0) y1=0;

  if (x2>=width())  
    x2 = width()-1;

  if (y2>=height()) 
    y2 = height()-1;  

  if (x2<x1 || y2<y1) 
    return 0;


  i4_image8 *im8 = new i4_image8(x2-x1+1,
                                 y2-y1+1,
                                 pal);
      

  w32 *pixel32 = (w32 *)((w8 *)data + x1*4 + y1*bpl);
  w8 *pixel8 = (w8 *)im8->data;

  w32 skip_32 = bpl-(x2-x1+1);   // pixels to skip per line
  w32 x,y;

  w8  closest_color;
  w32 closest_distance,
      distance,
      *color_index;

  w32 *pal_data=pal->source.lookup;

  
  // maps 16 bits color space into closest 8 bit color
  w8 *lookup_table=(w8 *)i4_malloc(0x10000, "lookup");
   
  // indicates if above color has been calculated yet
  w8 *table_calced=(w8 *)i4_malloc(0x10000, "table_calced");
  
  memset(table_calced,0,0x10000);     // initially no mappings are calculated

  for (y=y2-y1+1; y; y--)
  {
    for (x=x2-x1+1; x; x--)
    {
      i4_color color=*pixel32;


      w16 c16 = _32_to_16_565(color);
      if (table_calced[c16])           // have we found the closest color to this yet?
        *pixel8=lookup_table[c16];
      else
      {
        // find the closest color to this pixel
        color_index = pal_data+skip_colors;
        closest_distance = 0xffffffff;

        for (w32 c=skip_colors; c<256; c++)
        {
          distance=color_dist(*color_index,color);
          if (distance<closest_distance)
          {
            closest_distance=distance;
            closest_color=c;
          }
          color_index++;
        }

        table_calced[c16]=1;
        lookup_table[c16]=closest_color;
        *pixel8=closest_color;
      }

      ++pixel32;
      ++pixel8;
    }

    pixel32+=skip_32;
  }

  i4_free(lookup_table);
  i4_free(table_calced);

  return im8;
}
