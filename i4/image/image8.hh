/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef __I4_IMAGE8_HPP_
#define __I4_IMAGE8_HPP_

#include "image/image.hh"

class i4_image8 : public i4_image_class
{
public:
  w8 *typed_data() { return (w8 *)data; }
  i4_image8(w16 w, w16 h, const i4_pal *pal);
  i4_image8(w16 w, w16 h, const i4_pal *pal, void *data, int bpl);
  i4_color get_pixel(i4_coord x, i4_coord y);  
  void put_pixel(i4_coord x, i4_coord y, w32 color);

  i4_image_class *copy();
  
  ~i4_image8();
};


#endif


