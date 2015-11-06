/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef __MENUITEM_HPP_
#define __MENUITEM_HPP_

#include "arch.hh"
#include "device/event.hh"
#include "device/device.hh"
#include "window/window.hh"
#include "device/kernel.hh"
#include "string/string.hh"

class i4_menu_item_parent_class;
class i4_graphical_style_class;

class i4_menu_item_class : public i4_parent_window_class
{
  protected :
  i4_str *context_help;
  i4_event_handler_reference_class<i4_window_class> context_help_window;

  i4_bool active,    // if the mouse is on us, or we have the key focus
          pressed,   // if the mouse clicked us or ENTER was pressed while key focus
          disabled;  // if this button is not currently enabled right now

  // if this is non null, the menu_parent will be notified of events sent
  i4_menu_item_parent_class *menu_parent;
  i4_graphical_style_class *hint;

  public :

  void set_menu_parent(i4_menu_item_parent_class *_menu_parent)
  { menu_parent=_menu_parent; }


  enum reaction_type { PRESSED, DEPRESSED, ACTIVATED, DEACTIVATED };

  struct   // these event are only sent out, the are not noticed if sent in
  {    
    i4_event_reaction_class *press,    *depress;
    i4_event_reaction_class *activate, *deactivate;

  } send;

  

  i4_menu_item_class(const i4_const_str *idle_context_help,  // can be null
                     i4_graphical_style_class *hint,         // can be null if idle_help is null
                     w16 w, w16 h,
                     i4_event_reaction_class *press=0,
                     i4_event_reaction_class *depress=0,
                     i4_event_reaction_class *activate=0,
                     i4_event_reaction_class *deactivate=0
                     );            

  void set_context_help(const i4_const_str *help_string)
  {
    if (context_help)
      delete context_help;
    if (help_string)
      context_help = new i4_str(*help_string);
    else
      context_help = 0;
  }

  void send_event(i4_event_reaction_class *ev, reaction_type type);

  virtual void receive_event(i4_event *ev);

  virtual void do_activate();
  virtual void do_deactivate();
  virtual void do_disable();
  virtual void do_undisable();
  virtual void do_press();
  virtual void do_depress();
  virtual void do_idle();

  virtual i4_menu_item_class *copy() = 0;

  ~i4_menu_item_class();
} ;


// if a menu item has a pointer to it's parent, then it send it's events to the
// parent instead of to who the event_reaction specifies
class i4_menu_item_parent_class : public i4_parent_window_class
{
public:
  i4_menu_item_parent_class() : i4_parent_window_class(0,0) {}

  virtual void note_reaction_sent(i4_menu_item_class *who,       // this is who sent it
                                  i4_event_reaction_class *ev,   // who it was to 
                                  i4_menu_item_class::reaction_type type) { ; }
};

#endif
