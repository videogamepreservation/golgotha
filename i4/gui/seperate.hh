/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef I4_SEPERATOR_HH
#define I4_SEPERATOR_HH

#include "menu/menuitem.hh"

class i4_seperator_line_class : public i4_menu_item_class
{
  w16 lr,ud;
  i4_graphical_style_class *style;
  i4_color bg;
public:
  i4_seperator_line_class(i4_graphical_style_class *style,
                          i4_color background,
                          w16 left_right_space,
                          w16 up_down_space)
    : i4_menu_item_class(0,0, 0,0),
      style(style),
      lr(left_right_space),
      ud(up_down_space),
      bg(background)
  {
    w32 l,r,t,b;
    style->get_in_deco_size(l,t,r,b);
    resize(lr*2, t+b+ud*2);
  }
  
  void reparent(i4_image_class *draw_area, i4_parent_window_class *parent)
  {
    i4_menu_item_class::reparent(draw_area, parent);
    if (parent && draw_area)
    {
      w32 l,r,t,b;
      style->get_in_deco_size(l,t,r,b);
      resize(parent->width()-lr*2, t+b+ud*2);
    }

  }

  void draw(i4_draw_context_class &context)
  {
    style->deco_neutral_fill(local_image, 0,0, width()-1, height()-1, context);
    style->draw_in_deco(local_image, 0, ud, width()-1, height()-1-ud,
                        i4_F, context);
  }

  i4_menu_item_class *copy() { return new i4_seperator_line_class(style, bg, lr, ud); }

  char *name() { return "seperator_line"; }
};


#endif
