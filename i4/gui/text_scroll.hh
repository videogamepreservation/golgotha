/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef TEXT_SCROLL_WIN
#define TEXT_SCROLL_WIN

#include "window/window.hh"
#include "font/font.hh"
class i4_graphical_style_class;

class i4_text_scroll_window_class : public i4_parent_window_class
{
  i4_color fore,back;
  i4_char *term_out, *draw_start;
  w32 term_size, used_size;
  sw32 dx, dy, tdx, tdy;

  sw32 term_height;    // in characters
  w32 line_height;
  i4_bool need_clear;
  i4_graphical_style_class *style;

public:
  char *name() { return "text_scroll_window"; }

  void resize(w16 new_width, w16 new_height);

  i4_text_scroll_window_class(i4_graphical_style_class *style,
                              i4_color text_foreground,
                              i4_color text_background,
                              w16 width, w16 height);               // in pixels

  void clear();
  void skip_first_line();

  void output_char(const i4_char &ch);
  void output_string(char *string);
  void printf(char *fmt, ...);

  void parent_draw(i4_draw_context_class &context);

  virtual i4_bool need_redraw();

};



#endif
