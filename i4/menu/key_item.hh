/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef I4_KEY_ITEM_HH
#define I4_KEY_ITEM_HH


#include "menu/menu.hh"
#include "font/font.hh"
#include "window/style.hh"
#include "device/keys.hh"

class i4_key_item_class : public i4_menu_item_class
{
  protected :

  i4_color_hint_class *color;
  i4_font_hint_class *font;

  i4_str *text;
  w16 pad_lr, pad_ud;
  w16 use_key, key_modifiers;
  i4_bool key_focused, valid;

  public :
  i4_bool has_keyboard_focus() { return key_focused; }

  i4_key_item_class(
               const i4_const_str &_text,
               i4_color_hint_class *color_hint,
               i4_font_hint_class *font_hint,
               i4_graphical_style_class *style,
               w16 key=I4_NO_KEY,
               w16 key_modifiers=0,
               w16 pad_left_right=0,
               w16 pad_up_down=0
               );


  ~i4_key_item_class();

  char *name() { return "key_item"; }

  virtual void parent_draw(i4_draw_context_class &context);
  virtual void receive_event(i4_event *ev);

  // called when selected or key is pressed
  virtual void action() = 0;

  // if usage is disallowed then the item will be grayed out on the menu
  void allow_use()    { valid=i4_T; }
  void disallow_use() { valid=i4_F; } 
  virtual void do_press()
  {
    if (!valid) return;
    else i4_menu_item_class::do_press();
  }

  i4_menu_item_class *copy()
  {
    return 0;      // don't use this key_item anymore..
  }

} ;

class i4_key_accel_watcher_class : public i4_event_handler_class
{
  struct key_item_pointer_type
  {    
    i4_key_item_class *modkey[8];
    i4_key_item_class **get_from_modifiers(w16 modifiers);
  } 
  user[I4_NUM_KEYS];
  

  w32 total;
  i4_bool initialized;
public:
  i4_key_accel_watcher_class();
  void watch_key(i4_key_item_class *who, w16 key, w16 modifiers);
  void unwatch_key(i4_key_item_class *who, w16 key, w16 modifiers);
  void receive_event(i4_event *ev);
  
  char *name() { return "key accel watcher"; }
};



#endif



