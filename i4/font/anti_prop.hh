/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef ANTI_ALIASED_HH
#define ANTI_ALIASED_HH


#include "font/font.hh"
class i4_image_class;
class i4_anti_image_class;

class i4_anti_proportional_font_class : public i4_font_class
{
  w16 offsets[256];
  w16 widths[256]; 
  i4_anti_image_class *aim;
  int longest_w, h;
  public :

  i4_anti_proportional_font_class(i4_image_class *im, int start_ch=33);
  virtual void set_color(i4_color color);

  virtual void put_string(i4_image_class *screen, 
                          sw16 x, sw16 y, 
                          const i4_const_str &string, 
                          i4_draw_context_class &context);

  virtual void put_character(i4_image_class *screen, 
                             sw16 x, sw16 y, 
                             const i4_char &c, 
                             i4_draw_context_class &context);

  virtual w16 width(const i4_char &character) { return widths[character.ascii_value()]; }
  virtual w16 height(const i4_char &character) { return h; }

  virtual w16 width(const i4_const_str &string) 
  {
    int w=0;
    for (i4_const_str::iterator i=string.begin(); i!=string.end(); ++i)
      w+=width(i.get());
    return w;
  }

  virtual w16 height(const i4_const_str &string) { return h; }

  virtual w16 largest_height() { return h; }
  virtual w16 largest_width() { return longest_w; }

  virtual ~i4_anti_proportional_font_class();
};


#endif


