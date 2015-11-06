/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef __TEXTITEM_HPP_
#define __TEXTITEM_HPP_

#include "menu/menu.hh"
#include "font/font.hh"
#include "window/style.hh"

class i4_text_item_class : public i4_menu_item_class
{
  protected :

  i4_color_hint_class *color;
  i4_font_class *font;

  i4_str *text;
  w16 pad_lr;

  public :
  w32 bg_color;

  
  i4_text_item_class(
               const i4_const_str &_text,      
               i4_graphical_style_class *style,
         
               i4_color_hint_class *color_hint=0,
               i4_font_class *font=0,
         

               i4_event_reaction_class *press=0,
               i4_event_reaction_class *depress=0,
               i4_event_reaction_class *activate=0,
               i4_event_reaction_class *deactivate=0,
               w16 pad_left_right=0,
               w16 pad_up_down=0
               );
  ~i4_text_item_class()
  {
    delete text;
  }

  char *name() { return "text_item"; }

  virtual void parent_draw(i4_draw_context_class &context);
  virtual void receive_event(i4_event *ev);
  void change_text(const i4_const_str &new_st);
  i4_const_str get_text() { return *text; }

  virtual i4_menu_item_class *copy();
} ;

#endif
