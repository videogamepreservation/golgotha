/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef R1_FONT_HH
#define R1_FONT_HH

#include "font/font.hh"
#include "tex_id.hh"
class i4_image_class;
class r1_render_api_class;

class r1_font_class : public i4_font_class
{

  struct char_def
  {
    w8 x,y,w;
  };


  float xs,ys;
  char_def pos[256];
  int longest_w, largest_h;
  r1_texture_handle texture;
  
  r1_render_api_class *api;
  float r,g,b;

  i4_bool expand(i4_image_class *from, i4_image_class *to, int start_ch);

public:

  r1_font_class(r1_render_api_class *api, i4_image_class *im, int start_ch=33);

  virtual void set_color(i4_color color);

  virtual void put_string(i4_image_class *screen, 
                          sw16 x, sw16 y, 
                          const i4_const_str &string, 
                          i4_draw_context_class &context);

  virtual void put_character(i4_image_class *screen, 
                             sw16 x, sw16 y, 
                             const i4_char &c, 
                             i4_draw_context_class &context);

  virtual w16 width(const i4_char &character) { return pos[character.ascii_value()].w; }
  virtual w16 height(const i4_char &character) { return largest_h; }

  virtual w16 width(const i4_const_str &string) 
  {
    int w=0;
    for (i4_const_str::iterator i=string.begin(); i!=string.end(); ++i)
      w+=width(i.get());
    return w;
  }

  virtual w16 height(const i4_const_str &string) { return largest_h; }

  virtual w16 largest_height() { return largest_h; }
  virtual w16 largest_width() { return longest_w; }

  virtual ~r1_font_class() { ; }
};





#endif
