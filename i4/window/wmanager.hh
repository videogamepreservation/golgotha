/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef __WMANAGER_HPP_
#define __WMANAGER_HPP_

#include "window/window.hh"
#include "video/display.hh"
#include "palette/pal.hh"
#include "string/string.hh"
#include "window/style.hh"
#include "loaders/load.hh"
#include "device/keys.hh"

class i4_window_manager_class : public i4_parent_window_class
{
private:
  i4_display_class *display;
  i4_device_class::device_flags devices_present;
  i4_graphical_style_class *style;
  i4_cursor_class *default_cursor;
  i4_bool no_cursor_installed;  // set to true when cursor is not the default cursor
  i4_bool drag_dropping; 
  i4_color background_color;
  void cleanup_old_mode();

  struct
  {
    i4_bool active;
    w32 reference_id;
    void *further_info;
    i4_window_class *originator;
  } drag_drop;

public:
  int key_modifiers_pressed;

  void set_background_color(w32 color);
  i4_bool control_pressed() { return (key_modifiers_pressed & I4_MODIFIER_CTRL) ? i4_T : i4_F; }
  i4_bool alt_pressed() { return (key_modifiers_pressed & I4_MODIFIER_ALT) ? i4_T : i4_F; }
  i4_bool shift_pressed() { return (key_modifiers_pressed & I4_MODIFIER_SHIFT) ? i4_T : i4_F; }

  char *name() { return "window_manager"; }

  i4_graphical_style_class *get_style();

  virtual void parent_draw(i4_draw_context_class &context);

  virtual void root_draw();     // call from application, will call everyone else's draw function
  virtual void receive_event(i4_event *ev);

  i4_window_manager_class();     // if style cannot be found the first available on is used instead

  i4_menu_class *create_menu(i4_color_hint_class *hint, i4_bool hide_on_pick)
  { return style->create_menu(hide_on_pick); }

  void set_default_cursor(i4_cursor_class *cursor);

  void prepare_for_mode(i4_display_class *display, i4_display_class::mode *mode);

  i4_color i4_read_color_from_resource(char *resource);

  ~i4_window_manager_class(); 
};


#endif
