/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "font/plain.hh"
#include <string.h>

i4_plain_font_class::~i4_plain_font_class()
{
  delete bitmap;
}

i4_plain_font_class::i4_plain_font_class(i4_image_class *bitmap)
{
  i4_plain_font_class::bitmap=bitmap->copy();
  w=bitmap->width()/32;
  h=bitmap->height()/8;
}

void i4_plain_font_class::set_color(i4_color color)
{
  /*  w32 p[256];
  memset(p,0,sizeof(p));
  w8 r,g,b;

  i4_pixel_format *f=&pal.pal->source;
  w32 rgb=(((color & f->red_mask)>>f->red_shift)<<(16+(8-f->red_bits))) |
    (((color & f->green_mask)>>f->green_shift)<<(8+(8-f->green_bits))) |
    (((color & f->blue_mask)>>f->blue_shift)<<(16+(8-f->blue_bits)));

  p[1]=rgb;

  i4_pixel_format fmt;
  fmt.pixel_depth=I4_8BIT;
  fmt.lookup=p;

  bitmap->set_pal(i4_pal_man.register_pal(&fmt));
  return i4_T; */
}

void i4_plain_font_class::put_string(i4_image_class *screen, 
                                     sw16 x, sw16 y, 
                                     const i4_const_str &string, 
                                     i4_draw_context_class &context)
{
  if (!string.null())
  {
    i4_const_str::iterator p=string.begin();

    while (p!=string.end())
    {
      char ch=p.get().value();
      i4_coord x1=((ch)%32)*w;
      i4_coord y1=((ch)/32)*h;

      bitmap->put_part_trans(screen,x,y,x1,y1,x1+w-1,y1+h-1,0,context);
      x+=w;

      ++p;
    }
  }
} 


void i4_plain_font_class::put_character(i4_image_class *screen, 
                                        sw16 x, sw16 y, 
                                        const i4_char &c, 
                                        i4_draw_context_class &context)
{
  char ch=c.value();
  i4_coord x1=((ch)%32)*w;
  i4_coord y1=((ch)/32)*h;
  bitmap->put_part_trans(screen,x,y,x1,y1,x1+w-1,y1+h-1,0,context);
}
