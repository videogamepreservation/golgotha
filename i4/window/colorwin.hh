/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef __COLOR_WIN_HPP_
#define __COLOR_WIN_HPP_

#include "window/window.hh"
#include "palette/pal.hh"

class i4_graphical_style_class;
class i4_color_window_class : public i4_parent_window_class
{
protected:
  i4_color color;
  i4_graphical_style_class *style;

public:
  char *name() { return "color_window"; }  

  i4_color_window_class(w16 w, w16 h, i4_color color, i4_graphical_style_class *style) :
     i4_parent_window_class(w,h), color(color), style(style) { ; }

  virtual void parent_draw(i4_draw_context_class &context);

} ;

i4_parent_window_class *i4_add_color_window(i4_parent_window_class *parent, i4_color color, 
                                            i4_graphical_style_class *style,
                                            i4_coord x, i4_coord y, w16 w, w16 h);



#endif
