/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "window/window.hh"
#include "font/font.hh"
#include "window/style.hh"
#include "area/rectlist.hh"
#include "editor/dialogs/debug_win.hh"
#include <stdarg.h>

#ifdef DEBUG

class g1_debug_window_class : public i4_parent_window_class
{
  i4_color fore,back;
  char *screen;
  sw32 cx, cy, starty;
  sw32 cols, lines, col_width, line_height;

  i4_graphical_style_class *style;
  i4_bool need_clear;

public:
  char *name() { return "debug_window"; }

  g1_debug_window_class(i4_graphical_style_class *style,
			i4_color text_foreground,
			i4_color text_background,
			w16 width, w16 height, // in pixels
			sw32 starting_line = 0);

  void newline();
  void putch(char ch);
  void puts(char *string);
  void putsxy(int x, int y, char *string);

  void parent_draw(i4_draw_context_class &context);
};

i4_event_handler_reference_class<g1_debug_window_class> debug_window;
i4_event_handler_reference_class<i4_parent_window_class> debug_mp;

g1_debug_window_class::g1_debug_window_class(i4_graphical_style_class *style,
					     i4_color text_foreground,
					     i4_color text_background,
					     w16 width, w16 height,
					     sw32 starting_line)
  : i4_parent_window_class(width, height),
    style(style),
    fore(text_foreground),
    back(text_background),
    starty(starting_line),
    need_clear(i4_T)
{
  i4_font_class *fnt=style->font_hint->normal_font;

  col_width   = fnt->largest_height()+1;
  line_height = fnt->largest_width()+1;
  cols = width/col_width;
  lines = height/line_height;

  screen = (char *)i4_malloc(cols*lines, "debug screen");
  memset(screen, 0, cols*lines*sizeof(char));
  cx = 0;
  cy = starty;
}

void g1_debug_window_class::newline()
{
  cx = 0;
  if (++cy>=lines)
    cy = starty;

  if (!redraw_flag)
    request_redraw();
}

void g1_debug_window_class::putch(char ch)
{
  I4_ASSERT(cy>=0 && cy<lines && cx>=0 && cx<cols, 
	    "g1_debug_window_class::putch - Cursor out of bounds");

  switch (ch) {
  case '\n': 
    newline(); 
    break;
  default :   
    screen[cy*cols+cx] = ch;
    if (++cx>=cols) 
      newline();
    break;
  }

  if (!redraw_flag)
    request_redraw();
}

void g1_debug_window_class::puts(char *s)
{
  while (*s)
    putch(*s++);
}

void g1_debug_window_class::putsxy(int x, int y, char *s)
{
  while (*s && x<cols)
    if (x<cols)
      screen[y*cols + x++] = *s++;

  if (!redraw_flag)
    request_redraw();
}

void g1_debug_window_class::parent_draw(i4_draw_context_class &context)
{
  i4_font_class *fnt=style->font_hint->normal_font;

  if (!undrawn_area.empty())
    need_clear=i4_T;

  need_clear=i4_F;
    
  local_image->clear(back, context);
  fnt->set_color(fore);

  char *p;
  for (sw32 y=0; y<lines; y++) {
    p = screen + cols*y;
    for (sw32 x=0; x<cols; x++)
      fnt->put_character(local_image, x*col_width, y*line_height, p[x], context);
  }
  i4_parent_window_class::parent_draw(context);
}

int g1_debug_printf(char *s, ... )
//{{{
{
  char buf[256];
  va_list arg;
  int ret;

  if (debug_window.get()==0)
    return 0;

  va_start(arg,s);
  ret = vsprintf(buf, s, arg);
  va_end(arg);
  debug_window->puts(buf);

  return ret;
}

int g1_debug_printfxy(int x,int y, char *s, ...)
{
  char buf[256];
  va_list arg;
  int ret;

  if (debug_window.get()==0)
    return 0;

  va_start(arg,s);
  ret = vsprintf(buf, s, arg);
  va_end(arg);
  debug_window->putsxy(x,y,buf);

  return ret;
}

int g1_debug_open(i4_graphical_style_class *style,
		  i4_parent_window_class *parent, 
		  const i4_const_str &title,
		  int status_lines,
		  i4_event_reaction_class *debug_closed)
{
  sw32 x=0, y=0, w = 500, h=50;
	
  if (debug_mp.get())
    return 0;

  debug_mp=style->create_mp_window(x, y, w, h, title, debug_closed);
  debug_window = new g1_debug_window_class(style, 0xffffff, 0, w, h, status_lines);
  debug_mp->add_child(0,0,debug_window.get());
  
  return 1;
}

int g1_debug_close(i4_graphical_style_class *style)
{
  if (debug_mp.get()==0)
    return 0;

  style->close_mp_window(debug_mp.get());

  return 1;
}
//}}}
#endif
