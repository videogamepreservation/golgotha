/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef I4_BROWSE_TREE_HH
#define I4_BROWSE_TREE_HH


#include "window/window.hh"
#include "menu/menuitem.hh"
#include "error/error.hh"

class i4_graphical_style_class;

class i4_browse_window_class : public i4_parent_window_class
{
public:
  char *name() { return "browse_window"; }

  i4_bool expanded;
  i4_parent_window_class *title_area;
  i4_window_class *child_object, *toggle_button;

  i4_graphical_style_class *style;

  i4_browse_window_class(i4_graphical_style_class *style,
                         i4_window_class *title_object,
                         i4_window_class *child_object,
                         i4_bool show_plus_minus,
                         i4_bool expanded);


  virtual void compress();
  virtual void expand();


  void add_arranged_child(i4_window_class *child);
  virtual void replace_object(i4_window_class *object);
  virtual sw32 x_start();
  virtual void receive_event(i4_event *ev);
  virtual void parent_draw(i4_draw_context_class &context);
};


class i4_vertical_compact_window_class : public i4_parent_window_class
{
  i4_color color;
  i4_bool center;
public:
  char *name() { return "vertical_compact"; }

  virtual void receive_event(i4_event *ev);
  virtual void parent_draw(i4_draw_context_class &context);
  void compact();
  i4_vertical_compact_window_class(i4_color color, i4_bool center=i4_F) 
    : i4_parent_window_class(0,0), color(color), center(center) {}
  virtual void remove_child(i4_window_class *child)
  {
    i4_parent_window_class::remove_child(child);
    compact();
  }
};

class i4_horizontal_compact_window_class : public i4_parent_window_class
{
  i4_color color;
  i4_bool center;
public:
  char *name() { return "vertical_compact"; }

  virtual void receive_event(i4_event *ev);
  virtual void parent_draw(i4_draw_context_class &context);
  void compact();
  i4_horizontal_compact_window_class(i4_color color, i4_bool center=i4_F)
    : i4_parent_window_class(0,0), color(color), center(center) {}
  virtual void remove_child(i4_window_class *child)
  {
    i4_parent_window_class::remove_child(child);
    compact();
  }
};

#endif









