/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "window/colorwin.hh"
#include "image/image.hh"
#include "window/style.hh"

void i4_color_window_class::parent_draw(i4_draw_context_class &context)
{     
  i4_rect_list_class child_clip(&context.clip,0,0);
  child_clip.intersect_list(&undrawn_area);

  child_clip.swap(&context.clip);

  
  if (color==style->color_hint->neutral())
  {
    for (i4_rect_list_class::area_iter c=context.clip.list.begin();c!=context.clip.list.end();++c)
      style->deco_neutral_fill(local_image, c->x1, c->y1, c->x2, c->y2, context);
  }
  else local_image->clear(color,context);

  child_clip.swap(&context.clip);
  i4_parent_window_class::parent_draw(context);
}

i4_parent_window_class *i4_add_color_window(i4_parent_window_class *parent, i4_color color, 
                                            i4_graphical_style_class *style,
                                            i4_coord x, i4_coord y, w16 w, w16 h)
{
  i4_color_window_class *cw=new i4_color_window_class(w,h,color, style);
  if (parent)
    parent->add_child(x,y,cw);
  return cw;
}

