/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef I4_DECO_WIN_HH
#define I4_DECO_WIN_HH

#include "window/style.hh"

class i4_deco_window_class : public i4_parent_window_class
{
  i4_bool in;
  sw32 x1,y1;
  i4_graphical_style_class *style;

public:
  i4_deco_window_class(w16 w, w16 h,   // size not including deco border
                       i4_bool in,     // pressed in or out look?
                       i4_graphical_style_class *style)
    : i4_parent_window_class(0,0),
      in(in),
      style(style)
  {
    w32 l,r,t,b;
    if (in)
      style->get_in_deco_size(l,r,t,b);
    else
      style->get_out_deco_size(l,r,t,b);

    x1=l; y1=r;
    resize(w+l+r, h+t+b);
  }

  sw32 get_x1() { return x1; }
  sw32 get_y1() { return y1; }

  virtual void parent_draw(i4_draw_context_class &context)
  {
    w32 l,r,t,b;
    if (in)
    {
      style->get_in_deco_size(l,r,t,b);
      style->draw_in_deco(local_image, 0,0, width()-1, height()-1, i4_F, context);
    }
    else
    {
      style->get_out_deco_size(l,r,t,b);
      style->draw_in_deco(local_image, 0,0, width()-1, height()-1, i4_F, context);
    }


    style->deco_neutral_fill(local_image, l,t, width()-r, height()-b, context);
//     local_image->bar(l,t, width()-r, height()-b, 
//                      style->color_hint->window.passive.medium, 
//                      context);
  }

  char *name() { return "deco_window"; }
} ;

#endif
