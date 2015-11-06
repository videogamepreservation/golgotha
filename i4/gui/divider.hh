/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef __DIVIDER_HH
#define __DIVIDER_HH

#include "window/window.hh"
#include "device/device.hh"

class i4_graphical_style_class;

class i4_divider_class : public i4_parent_window_class
{
  i4_event_handler_reference_class<i4_window_class> w1,w2;
  i4_bool split_up_down;
  i4_graphical_style_class *style;
  int split_value, min1, min2;

  i4_bool dragging, w_attached, h_attached;
  

  void get_drag_area(int &x1, int &y1, int &x2, int &y2);
  virtual void reparent(i4_image_class *draw_area, i4_parent_window_class *parent);
public:
  void resize(w16 new_width, w16 new_height);

  i4_divider_class(int w, int h,               // -1 uses parent w or height
                   i4_bool split_up_down,
                   int split_x_or_y,           // -1 splits the middle
                   i4_window_class *window1,
                   i4_window_class *window2,
                   i4_graphical_style_class *style,
                   int window1_min_size=1,
                   int window2_min_size=1);

  void parent_draw(i4_draw_context_class &context);
  void receive_event(i4_event *ev);
  char *name() { return "divider"; }
};

#endif
