/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef __TEXT_HH_
#define __TEXT_HH_

#include "window/window.hh"
#include "string/string.hh"
#include "window/style.hh"

class i4_text_window_class : public i4_window_class
{
  i4_str *text;
  i4_graphical_style_class *hint;
  i4_font_class *font;

public:
  i4_text_window_class(const i4_const_str &text,
                       i4_graphical_style_class *hint,
                       i4_font_class *_font=0)
    : i4_window_class(0, 0),
      hint(hint),
      text(new i4_str(text)),
      font(_font)
  {
    if (!font)
      font=hint->font_hint->normal_font;
    
    resize(font->width(text), font->height(text));
  }
                    
  virtual void draw(i4_draw_context_class &context)
  {
    local_image->add_dirty(0,0,width()-1,height()-1,context);
    font->set_color(hint->color_hint->text_foreground);

    hint->deco_neutral_fill(local_image, 0,0, width()-1, height()-1, context);
    //    local_image->clear(hint->color_hint->window.passive.medium,context);
    if (text)
      font->put_string(local_image,0,0,*text,context);  
  }

  ~i4_text_window_class() { delete text; }

  void set_text(i4_str *new_text)
  {
    if (text)
      delete text;
    text = new_text;
    request_redraw();
  }

  char *name() { return "text_window"; }
};


#endif
