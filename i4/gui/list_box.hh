/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef I4_LIST_BOX_HH
#define I4_LIST_BOX_HH

#include "window/window.hh"
#include "memory/array.hh"
#include "menu/menu.hh"

class i4_button_class;
class i4_font_class;
class i4_graphical_style_class;
class i4_deco_window_class;
class i4_list_pull_down_class;

class i4_list_box_class : public i4_menu_class
{
  i4_button_class *down;
  i4_array<i4_menu_item_class *> entries;
  i4_menu_item_class *top;
  i4_list_pull_down_class *pull_down;
  i4_graphical_style_class *style;
  int current;
  i4_parent_window_class *root_window;
  w32 l,r,t,b;
  void set_top(i4_menu_item_class *item);

public:
  int get_current() { return current; }
  i4_menu_item_class *get_current_item() { return entries[current]; }
  i4_list_box_class(w16 width,                               // height is determined by first item
                    i4_graphical_style_class *style,         // uses normal font if none specified
                    i4_parent_window_class *root_window);
  ~i4_list_box_class();

  void add_item(i4_menu_item_class *item);
  void set_current_item(int entry_num);

  void show(i4_parent_window_class *show_on, i4_coord x, i4_coord y);
  void hide();
  void receive_event(i4_event *ev);
  void note_reaction_sent(i4_menu_item_class *who,       // this is who sent it
                          i4_event_reaction_class *ev,   // who it was to 
                          i4_menu_item_class::reaction_type type);

  void parent_draw(i4_draw_context_class &context);

  char *name() { return "list_box"; }
};


#endif
