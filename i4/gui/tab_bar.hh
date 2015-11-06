/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef I4_TAB_BAR_HH
#define I4_TAB_BAR_HH

#include "window/window.hh"
#include "menu/menu.hh"

class i4_tab_bar_data;
class i4_graphical_style_class;

class i4_tab_bar : public i4_menu_class
{
  i4_tab_bar_data *data;
  void private_resize(w16 new_width, w16 new_height); 
  void note_reaction_sent(i4_menu_item_class *who,
                          i4_event_reaction_class *ev,
                          i4_menu_item_class::reaction_type type);
  virtual void show(i4_parent_window_class *show_on, i4_coord x, i4_coord y) { ; }
  virtual void hide() { ;}
public:
  i4_tab_bar(int width, int height, i4_graphical_style_class *style);
  void add_tab(i4_menu_item_class *tab_top, i4_window_class *tab_contents);
  void set_current_tab(int tab_number);

  virtual void parent_draw(i4_draw_context_class &context);
  ~i4_tab_bar();
  char *name() { return "tab_bar"; }
};


#endif
