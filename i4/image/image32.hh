/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef __I4_IMAGE32_HPP_
#define __I4_IMAGE32_HPP_

#include "image/image.hh"

class i4_image32 : public i4_image_class
{
public:
  w32 *paddr(int x, int y) { return (w32 *)(((w8 *)data)+x*4+y*bpl); }
  

  i4_image32(w16 w, w16 h, const i4_pal *pal);
  i4_image32(w16 w, w16 h, const i4_pal *pal, void *data, int bpl);
  i4_color get_pixel(i4_coord x, i4_coord y);  
  void put_pixel(i4_coord x, i4_coord y, w32 color);
  
  i4_image_class *quantize(const i4_pal *pal,
                           w32 skip_colors,
                           i4_coord x1, i4_coord y1,
                           i4_coord x2, i4_coord y2);
  
  ~i4_image32();
};


#endif
