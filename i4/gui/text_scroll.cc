/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifdef _MANGLE_INC
#include "gui/TEXT_~ZG.HH"
#else
#include "gui/text_scroll.hh"
#endif
#include "window/style.hh"
#include "area/rectlist.hh"


static inline int fmt_char(char c)
{
  if ((c>='a' && c<='z') || (c>='A' && c<='Z'))
    return 1;
  return 0;
}

void i4_text_scroll_window_class::printf(char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);

  while (*fmt)
  {
    if (*fmt=='%')
    {
      char *fmt_end=fmt;
      while (!fmt_char(*fmt_end) && *fmt_end) fmt_end++;
      char f[10], out[500]; 
      memcpy(f, fmt, fmt_end-fmt+1);
      f[fmt_end-fmt+1]=0;
      out[0]=0;

      switch (*fmt_end)
      {
        case 's' : 
        {
          char *str=va_arg(ap,char *);
          output_string(str);
        } break;          

        case 'd' :
        case 'i' :
        case 'x' :
        case 'X' :
        case 'c' :
        case 'o' :
        {
          ::sprintf(out,f,va_arg(ap,int));
          output_string(out);
        } break;

        case 'f' :
        case 'g' :
          ::sprintf(out,f,va_arg(ap,double));
          output_string(out);
          break;

        default :
          ::sprintf(out,f,va_arg(ap,void *));
          output_string(out);
          break;
      }
      fmt=fmt_end;
      if (*fmt)
        fmt++;
    }
    else
    {
      output_char(*fmt);
      fmt++;
    }
  }
  va_end(ap);
}



i4_text_scroll_window_class::i4_text_scroll_window_class(i4_graphical_style_class *style,
                                                         i4_color text_foreground,
                                                         i4_color text_background,
                                                         w16 width, w16 height)    // in pixels
  : i4_parent_window_class(width, height),
    style(style),
    fore(text_foreground),
    back(text_background)
{
  i4_font_class *fnt=style->font_hint->normal_font;

  term_height=height/fnt->largest_height();
  term_size=(width/fnt->largest_width() + 1) * term_height;

  if (!term_size)
    term_size=512;

  term_out=(i4_char *)i4_malloc(term_size, "terminal chars");

  draw_start=term_out;
  line_height=fnt->largest_height()+1;
  *term_out=0;
  dx=dy=tdx=tdy=0;
  need_clear=i4_T;
  used_size = 0;
}

void i4_text_scroll_window_class::skip_first_line()
{
  i4_font_class *fnt=style->font_hint->normal_font;
  w32 x=0, wd=width(), count=0;
  i4_char *s=term_out;
    
  while (s->value() && x<wd)
  {
    if (s->value()=='\n')
      x=wd;
    else
      x+=fnt->width(*s);
    count++;
    s++;
  }
  memmove(term_out, s, used_size-count+1);
  used_size-=count;
}

void i4_text_scroll_window_class::output_string(char *string)
{
  while (*string)
  {
    output_char( (i4_char)(*string));
    string++;
  }
}

void i4_text_scroll_window_class::output_char(const i4_char &ch)
{
  sw32 i;
  i4_font_class *fnt=style->font_hint->normal_font;

  if (used_size+2>term_size)
  {
    term_size+=100;
    i4_char *old_spot=term_out;
    term_out=(i4_char *)i4_realloc(term_out, term_size, "terminal chars");
    dx=dy=0;
    draw_start=term_out;        
    need_clear=i4_T;    
  }

  if (!redraw_flag)
    request_redraw();

  tdx+=fnt->width(ch);
  if (tdx>=width() || ch=='\n')
  {
    tdx=0;
    tdy+=line_height;
    if (tdy+line_height>height())
    {
      skip_first_line();
      tdy-=line_height;
      dx=dy=0;
      draw_start=term_out;        
      need_clear=i4_T;
    }
  }

  term_out[used_size++]=ch;  
  term_out[used_size]=0;
}

void i4_text_scroll_window_class::clear()
{
  draw_start=term_out;
  *term_out=0;
  dx=dy=tdx=tdy=0;
  need_clear=i4_T;
  used_size = 0;
  request_redraw(i4_F);
}

void i4_text_scroll_window_class::resize(w16 new_width, w16 new_height)
{
  i4_font_class *fnt=style->font_hint->normal_font;
  term_height=new_height/fnt->largest_height();
  
  int new_size=(new_width/fnt->largest_width() + 1) * term_height;
  if (new_size>term_size)
  {
    term_size=new_size;

    if (!term_size)
      term_size=512;

    if (term_out)
      term_out=(i4_char *)i4_realloc(term_out, term_size, "terminal chars");
    else
    {
      term_out=(i4_char *)i4_malloc(term_size, "terminal chars");
      term_out[0]=0;
    }
  }

  dx=dy=tdx=tdy=0;
  used_size = 0;

  i4_parent_window_class::resize(new_width, new_height);
}


void i4_text_scroll_window_class::parent_draw(i4_draw_context_class &context)
{
  i4_font_class *fnt=style->font_hint->normal_font;

  if (!undrawn_area.empty())
  {
    draw_start=term_out;
    dx=dy=0;
    need_clear=i4_T;
  }
    
  if (need_clear)
  {
    if (back==style->color_hint->neutral())
      style->deco_neutral_fill(local_image, 0,0, width()-1, height()-1, context);
    else
      local_image->clear(back, context);

    need_clear=i4_F;
  }
  fnt->set_color(fore);

  while (draw_start->value())
  {
    if (draw_start->value()=='\n')
      dx=width()+1;
    else
    {
      fnt->put_character(local_image, dx,dy, *draw_start, context);
      dx+=fnt->width(*draw_start);
    }

    if (dx>=width())
    {
      dx=0;
      dy+=fnt->largest_height();
    } 

    draw_start++;  
  } 
  tdx=dx;
  tdy=dy;
  i4_parent_window_class::parent_draw(context);
}

i4_bool i4_text_scroll_window_class::need_redraw() 
{ 
  if (draw_start->value()) 
    return i4_T;
  else 
    return i4_parent_window_class::need_redraw();
}      
