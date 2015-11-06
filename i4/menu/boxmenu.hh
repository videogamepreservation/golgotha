/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef __BOXMENU_HPP_
#define __BOXMENU_HPP_

#include "menu/menu.hh"
#include "window/window.hh"
#include "error/error.hh"
#include "menu/menuitem.hh"

class i4_graphical_style_class;

class i4_box_menu_class : public i4_menu_class
{

  i4_graphical_style_class *style;
  
  public :
  // show should make the menu visible on the screen
  virtual void show(i4_parent_window_class *show_on, i4_coord x, i4_coord y);
  virtual void hide();

  virtual void parent_draw(i4_draw_context_class &context);
  i4_box_menu_class(i4_graphical_style_class *style, 
                    i4_bool hide_on_pick) : 
    i4_menu_class(hide_on_pick),
    style(style)
  { ; }

  char *name() { return "box_menu"; }

} ;


#endif
