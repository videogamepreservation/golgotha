/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef __LIST_PICK_HH
#define __LIST_PICK_HH

#include "window/window.hh"
class i4_menu_item_class;

class i4_list_pick : public i4_parent_window_class
{
  w32 total_items;
  i4_menu_item_class **items;
  i4_color background;
  w32 length;
  w32 scroll_event_id;
  sw32 start,end;
  i4_bool need_draw_all, free_items;

public:
  char *name() { return "list_pick"; }
  
  i4_list_pick(w16 width, w16 height,
               w32 total_items,
               i4_menu_item_class **items,
               w32 scroll_event_id,
               i4_color background,
               i4_bool free_item=i4_T);

  ~i4_list_pick();

  virtual void reposition_start(w32 new_start);
  
  virtual void parent_draw(i4_draw_context_class &context);
  virtual void draw(i4_draw_context_class &context);

  virtual void receive_event(i4_event *ev);
};


#endif

