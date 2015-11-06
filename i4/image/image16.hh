/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef __I4_IMAGE16_HPP_
#define __I4_IMAGE16_HPP_

#include "image/image.hh"
#include "error/error.hh"

class i4_image16 : public i4_image_class
{
public:
  w16 *paddr(int x, int y) { return (w16 *)(((w8 *)data)+y*bpl) + x; }
  w16 *typed_data() { return (w16 *)data; }
  
  i4_image16(w16 w, w16 h, const i4_pal *pal);
  i4_image16(w16 w, w16 h, const i4_pal *pal, void *data, int bpl);
  
  i4_color get_pixel(i4_coord x, i4_coord y);  
  void put_pixel(i4_coord x, i4_coord y, w32 color);

  void put_part_one_pixel_at_a_time(i4_image_class *to, 
                                    i4_coord _x,  i4_coord _y,                              
                                    i4_coord x1, i4_coord y1, i4_coord x2, i4_coord y2, 
                                    i4_draw_context_class &context);

  void put_part(i4_image_class *to, 
                i4_coord _x,  i4_coord _y,                              
                i4_coord x1, i4_coord y1, i4_coord x2, i4_coord y2, 
                i4_draw_context_class &context);

  void put_part_trans(i4_image_class *to, 
                      i4_coord _x,  i4_coord _y,                              
                      i4_coord x1, i4_coord y1, i4_coord x2, i4_coord y2,
                      i4_color trans_color,
                      i4_draw_context_class &context);

  void bar(i4_coord x1,    i4_coord y1, 
           i4_coord x2,    i4_coord y2, 
           i4_color color, i4_draw_context_class &context);
  
  void line(i4_coord ox1, i4_coord oy1, 
            i4_coord ox2, i4_coord oy2, 
            i4_color color, i4_draw_context_class &context);

  ~i4_image16();
};


#endif
