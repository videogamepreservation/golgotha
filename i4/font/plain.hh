/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef __PLAIN_FONT_HPP_
#define __PLAIN_FONT_HPP_

#include "font/font.hh"
#include "image/image.hh"

/* This is a quick and dirty implementation of a font 
   It uses a bitmap arranged 32 across and 8 down and
   grabs a small part which is placed on the screen per letter.
   The color of the font is changed by loading in a new palette
   for the bitmap and letting copy_from() do the color remapping.

   Exmaple ussage :

   i4_const_str s=i4gets("font1_image_filename");   // get the filename from the resource file
   i4_image_class *font_image=i4_load_image(s);           // load the bitmap
   i4_plain_font_class *font=new i4_plain_font_class(im);       // create the font


   TODO :
   fonts should probably loaded through a resource name.
   i.e.
   i4_load_font(const i4_const_str &resource_name);
   */

class i4_plain_font_class : public i4_font_class
{
  i4_image_class *bitmap;
  w16 w,h;
  public :
  i4_plain_font_class(i4_image_class *bitmap);   // bitmap is copied and stored internally

  virtual void set_color(i4_color color);

  virtual void put_string(i4_image_class *screen, 
                          sw16 x, sw16 y, 
                          const i4_const_str &string, 
                          i4_draw_context_class &context);

  virtual void put_character(i4_image_class *screen, 
                             sw16 x, sw16 y, 
                             const i4_char &c, 
                             i4_draw_context_class &context);

  virtual w16 width(const i4_char &character)  { return w; }   // width in pixels
  virtual w16 height(const i4_char &character) { return h; }  // height in pixels

  virtual w16 width(const i4_const_str &string)  { return string.length()*w; }   // width in pixels
  virtual w16 height(const i4_const_str &string) { return h; }  // height in pixels

  virtual w16 largest_height() { return h; }
  virtual w16 largest_width() { return w; }
  virtual ~i4_plain_font_class();
} ;

#endif

