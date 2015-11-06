/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_MENU_HH
#define G1_MENU_HH

#include "window/window.hh"
#include "string/string.hh"

class i4_menu_class;
class i4_graphical_style_class;


class g1_main_menu_class : public i4_parent_window_class
{
  i4_event_handler_class *notify;
  i4_image_class *deco;
  i4_graphical_style_class *style;

public:

  g1_main_menu_class(w16 w, w16 h, 
                     i4_event_handler_class *notify,
                     sw32 *ids,
                     i4_graphical_style_class *style);

  virtual void parent_draw(i4_draw_context_class &context);


  ~g1_main_menu_class();
  char *name() { return "g1_main_menu"; }
};

#endif
