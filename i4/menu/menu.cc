/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "menu/menu.hh"
#include "device/event.hh"
#include "window/win_evt.hh"

class i4_depress_menu_item : public i4_object_message_event_class
{
  public :

  i4_menu_item_class *item;
  
  i4_depress_menu_item(i4_menu_class *menu, 
                       i4_menu_item_class *item)
    : i4_object_message_event_class(menu),item(item) {}


  virtual i4_event  *copy() 
  { 
    return new i4_depress_menu_item((i4_menu_class *)object,item); 
  }

} ;

void i4_menu_class::add_item(i4_menu_item_class *item)
{
  item->set_menu_parent(this);
  i4_parent_window_class::add_child(0,0,item);
}


void i4_menu_class::note_reaction_sent(i4_menu_item_class *who,       // this is who sent it
                                       i4_event_reaction_class *ev,   // who it was to 
                                       i4_menu_item_class::reaction_type type)
{
  // if the item was pressed, send a delayed event to ourself to depress it
  if (type==i4_menu_item_class::PRESSED)
  {
    i4_depress_menu_item dn(this,who);
    i4_kernel.send_event(this,&dn);
    
    if (hide_on_pick)
      hide();
  }
}

void i4_menu_class::receive_event(i4_event *ev)
{
  if (deleted)
    i4_warning("getting events after death, talk about wierd");

  if (ev->type()==i4_event::OBJECT_MESSAGE)
  {
    CAST_PTR(r,i4_depress_menu_item,ev);
    if (r->object==this)
      r->item->do_depress();
    else i4_parent_window_class::receive_event(ev);
  }
  else
  {
    if (ev->type()==i4_event::WINDOW_MESSAGE && focus_inform)
    {
      CAST_PTR(wev,i4_window_message_class,ev);
      if (wev->sub_type==i4_window_message_class::LOST_MOUSE_FOCUS)
      {
        CAST_PTR(lost, i4_window_lost_mouse_focus_class, ev);

        i4_menu_focus_event_class mlost(this, 
                                       i4_menu_focus_event_class::lost_focus,
                                       lost->lost_to);

        i4_kernel.send_event(focus_inform, &mlost );
      }
      else if (wev->sub_type==i4_window_message_class::GOT_MOUSE_FOCUS)
      {
        i4_menu_focus_event_class got(this, 
                                      i4_menu_focus_event_class::got_focus,
                                      0);

        i4_kernel.send_event(focus_inform, &got );
      }        
    }
    i4_parent_window_class::receive_event(ev);
  }
}

i4_menu_class::~i4_menu_class()
{
  deleted=i4_T;
}

