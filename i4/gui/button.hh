/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef __BUTTON_HPP_
#define __BUTTON_HPP_

#include "menu/menuitem.hh"
#include "window/style.hh"
#include "time/timedev.hh"

class i4_button_class : public i4_menu_item_class
{
  protected :

  i4_window_class *decore;
  i4_bool grabbing;
  i4_bool repeat_down;
  i4_bool popup;

  enum { WAIT_CLICK, WAIT_DELAY, WAIT_REPEAT } state;
  i4_time_device_class::id time_id;
  i4_event_reaction_class *repeat_event;

  public :
  char *name() { return "button_class"; }
  i4_button_class(const i4_const_str *idle_context_help,  // can be null
                  i4_window_class *child,
                  i4_graphical_style_class *hint,
                  i4_event_reaction_class *press=0,
                  i4_event_reaction_class *depress=0,
                  i4_event_reaction_class *activate=0,
                  i4_event_reaction_class *deactivate=0      );

  virtual void reparent(i4_image_class *draw_area, i4_parent_window_class *parent);
  virtual void receive_event(i4_event *ev);
  virtual void parent_draw(i4_draw_context_class &context);

  void set_repeat_down(i4_bool value, i4_event_reaction_class *repeat_event=0);
  void set_popup(i4_bool value);
    
  i4_menu_item_class *copy() { return new i4_button_class(context_help, 
                                                          get_nth_window(0),
                                                          hint,
                                                          send.press, send.depress,
                                                          send.activate, send.deactivate); }

  ~i4_button_class();
} ;

#endif


