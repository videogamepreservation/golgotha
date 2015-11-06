/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "image/image8.hh"
#include "memory/malloc.hh"
#include "palette/pal.hh"

#include <string.h>



i4_color i4_image8::get_pixel(i4_coord x, i4_coord y)
{    
  return i4_pal_man.convert_to_32(*(typed_data() + bpl*y + x), pal);
}
  
void i4_image8::put_pixel(i4_coord x, i4_coord y, w32 color)
{
  *(typed_data() + bpl*y + x)=i4_pal_man.convert_32_to(color, &pal->source);
}


i4_image8::i4_image8(w16 _w, w16 _h, const i4_pal *_pal)
{
  w=_w;
  h=_h;
  bpl=_w;
  set_pal(_pal);
  data=i4_malloc(w*h,"");  
}

i4_image8::i4_image8(w16 _w, w16 _h, const i4_pal *_pal,
                       void *_data, int _bpl)
{
  data=_data;
  bpl=_bpl;
  pal=_pal;
  w=_w;
  h=_h;

  dont_free_data=i4_T;
}


i4_image8::~i4_image8()
{
  if (!dont_free_data)
    i4_free(data);
}


i4_image_class *i4_image8::copy()
{
  i4_image_class *im=i4_create_image(width(), height(), pal);

  
  for (int y=0; y<h; y++)
    memcpy(((w8 *)im->data) + y*im->bpl,
           ((w8 *)data) + y*bpl,
           w);

  return im;
}
