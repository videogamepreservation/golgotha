/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef __MENU_HPP_
#define __MENU_HPP_

#include "window/window.hh"
#include "error/error.hh"
#include "menu/menuitem.hh"
#include "string/string.hh"

class i4_menu_class : public i4_menu_item_parent_class
{
  // when a menu_item sends it's a press event should the menu hide itself?
  i4_bool           hide_on_pick;  
  i4_bool           deleted;             // debugging tool so we can see if we already got events

  i4_event_handler_class *focus_inform;
public:

  virtual void notify_focus_change(i4_event_handler_class *who)
  { focus_inform=who; }

  // show should make the menu visible on the screen
  virtual void show(i4_parent_window_class *show_on, i4_coord x, i4_coord y) = 0;

  virtual void hide() = 0;

  // apps should call hide() before add_item()
  virtual void add_item(i4_menu_item_class *item);
  
  virtual void receive_event(i4_event *ev);

  i4_menu_class(i4_bool hide_on_pick) 
    : hide_on_pick(hide_on_pick)
  {
    focus_inform=0;
    deleted=i4_F;
  }
  virtual ~i4_menu_class();


  virtual void add_child(i4_coord x, i4_coord y, i4_window_class *child)
  { i4_error("use add_item"); }

  virtual void note_reaction_sent(i4_menu_item_class *who,       // this is who sent it
                                  i4_event_reaction_class *ev,   // who it was to 
                                  i4_menu_item_class::reaction_type type);
} ;

class i4_menu_focus_event_class : public i4_object_message_event_class
{
public:
  enum focus_state_type { got_focus, lost_focus } focus_state;
  i4_window_class *lost_to;

  i4_menu_focus_event_class(i4_menu_class *menu,
                            focus_state_type state,
                            i4_window_class *lost_to)
    :  i4_object_message_event_class(menu),
       focus_state(state),
       lost_to(lost_to)
  {}

  virtual i4_event  *copy() 
  { 
    return new i4_menu_focus_event_class((i4_menu_class *)object,
                                              focus_state,
                                              lost_to); 
  }
  
  char *name() { return "menu lost focus"; }
};


#endif
