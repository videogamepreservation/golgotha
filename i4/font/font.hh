/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef __FONT_HPP_
#define __FONT_HPP_

#include "image/image.hh"
#include "string/string.hh"

// this is just the base defintion of what a font is

//   expected implementations included normal single color, aliased, shadow, and perhaps
//   some fancy stuff like beveled and texture mapped.

// implementations should be aware that 16 bit characters sets might occur
// by making each call pass in a i4_const_str rather than char *, translation can be more 
// easily seperated.

// See font/plain.c for an actual implemenation.
// Fonts are loaded by a i4_graphical_style_class (window/style.hh)

class i4_font_class
{
  void put_line(i4_image_class *screen, i4_const_str::iterator s, i4_const_str::iterator e, 
                int x, int y, float space_width, i4_draw_context_class &context);

  i4_bool get_line(const i4_const_str &string,
                   i4_const_str::iterator &s, i4_const_str::iterator &e, 
                   int &t_words, int &t_spaces, int &chars_width, int line_width);
  public :
  virtual void set_color(i4_color color) = 0;

  enum justification_type { LEFT, RIGHT, CENTER, FULL } ;
  void put_paragraph(i4_image_class *screen, 
                     sw16 x, sw16 y, 
                     const i4_const_str &string, 
                     i4_draw_context_class &context,
                     int space_between_lines=0,
                     justification_type justification=LEFT,
                     int line_width_in_pixels=-1); // words wrap after this distance unless -1
    
                     

  virtual void put_string(i4_image_class *screen, 
                          sw16 x, sw16 y, 
                          const i4_const_str &string, 
                          i4_draw_context_class &context) = 0;

  virtual void put_character(i4_image_class *screen, 
                             sw16 x, sw16 y, 
                             const i4_char &c, 
                             i4_draw_context_class &context) = 0;

  virtual w16 width(const i4_char &character) = 0;   // width in pixels
  virtual w16 height(const i4_char &character) = 0;  // height in pixels

  virtual w16 width(const i4_const_str &string) = 0;   // width in pixels
  virtual w16 height(const i4_const_str &string) = 0;  // height in pixels

  virtual w16 largest_height() = 0;  // height of tallest character
  virtual w16 largest_width() = 0;   // width of widest character

  virtual ~i4_font_class() { ; }
} ;

#endif
