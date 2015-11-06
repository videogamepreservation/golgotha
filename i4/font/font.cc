/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "font/font.hh"
#include "math/num_type.hh"


i4_bool i4_font_class::get_line(const i4_const_str &string,
                                i4_const_str::iterator &s, i4_const_str::iterator &e, 
                                int &t_words, int &t_spaces,
                                int &chars_width, int line_width)
{
  t_words=0;
  chars_width=0;
  t_spaces=0;


  int x=0, last_word_width=0, last_word_spaces=0;
  if (line_width<0) line_width=1000000;
  int in_a_word=0;
  i4_const_str::iterator word_start=s;

  while (e!=string.end() && e.get().ascii_value()!='\n')
  {
    i4_char c=e.get();

    int c_width=width(c);

    if (x+c_width>line_width)      // we exceeded the space available for this line
    {
      if (in_a_word)
      {
        chars_width=last_word_width;
        e=word_start;
        t_spaces=last_word_spaces;
      }

      return t_words ? i4_T : i4_F;
    }


    x+=c_width;

    if (!c.is_space())
    {
      if (!in_a_word)
      {
        last_word_width=chars_width;
        word_start=e;
       
      }

      in_a_word=1;
      chars_width+=c_width;
    }
    else
    {
      if (in_a_word)
      {
        in_a_word=0;
        t_words++;
        last_word_spaces=t_spaces;
      }

      t_spaces++;
    }


    ++e;
  }

  if (in_a_word)
    t_words++;

  if (e!=string.end() && e.get().ascii_value()=='\n')
    ++e;

  return t_words ? i4_T : i4_F;

}

void i4_font_class::put_line(i4_image_class *screen,
                             i4_const_str::iterator s, i4_const_str::iterator e, 
                             int x, int y,
                             float space_width,
                             i4_draw_context_class &context)
{
  float fx=x;
  while (s!=e)
  {
    put_character(screen, i4_f_to_i(fx), y, s.get(), context);
    if (s.get().is_space())
      fx+=space_width;
    else
      fx+=width(s.get());
    ++s;
  }
}

void i4_font_class::put_paragraph(i4_image_class *screen, 
                                  sw16 x, sw16 y, 
                                  const i4_const_str &string, 
                                  i4_draw_context_class &context,
                                  int space_between_lines,
                                  justification_type justification,
                                  int line_width_in_pixel)
{

  i4_const_str::iterator start=string.begin(),end=string.begin();
  int t_words, chars_width;

  if (line_width_in_pixel==-1 && justification!=LEFT)
    i4_error("cannot do justification without line width");

  int space_width=width(i4_const_str(" "));

  int t_spaces=0;
  while (get_line(string, start,end, t_words, t_spaces, chars_width, line_width_in_pixel))
  {
    switch (justification)
    {
      case LEFT : put_line(screen, start, end, x, y, space_width, context); break;
      case RIGHT : 
        put_line(screen, start, end, 
                 x+line_width_in_pixel-(chars_width + space_width * t_spaces), 
                 y, space_width, context); break;

      case CENTER :
        put_line(screen, start, end, 
                 x+line_width_in_pixel/2-(chars_width + space_width * t_spaces)/2, 
                 y, space_width, context); break;

      case FULL :
        float sw;
        if (t_words>1)
          sw=(line_width_in_pixel-chars_width)/(float)t_spaces;
        else
          sw=0;

        put_line(screen, start, end, x, y, sw, context);
        break;
    }

    if (end!=string.end() && end.get().is_space())
    {
      start=end;
      ++start;
      end=start;
    }
    else start=end;

    y+=largest_height() + space_between_lines;
  }

}
    



