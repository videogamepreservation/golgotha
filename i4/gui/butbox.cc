/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/


#include "gui/butbox.hh"
#include "error/error.hh"
#include "device/kernel.hh"
#include "device/event.hh"
#include "app/app.hh"

void i4_button_box_class::parent_draw(i4_draw_context_class &context)
{
  // this code swiped from color_window
  i4_rect_list_class child_clip(&context.clip,0,0);
  child_clip.intersect_list(&undrawn_area);

  child_clip.swap(&context.clip);
  
  i4_graphical_style_class *style=i4_current_app->get_style();

  for (i4_rect_list_class::area_iter c=context.clip.list.begin();c!=context.clip.list.end();++c)
    style->deco_neutral_fill(local_image, c->x1, c->y1, c->x2, c->y2, context);

  child_clip.swap(&context.clip);
  i4_parent_window_class::parent_draw(context);
}

i4_button_box_class::i4_button_box_class(i4_event_handler_class *receiver,
                                         i4_bool require_one_down)
  : receiver(receiver),
    require_one_down(require_one_down)
{
  current_down=0;
}

void i4_button_box_class::add_child(i4_coord x, i4_coord y, i4_window_class *child) 
{ 
  i4_error("You must use add_button"); 
}

void i4_button_box_class::expand_if_needed()
{
  win_iter c=children.begin();
  w32 w=width(),h=height();
  for (;c!=children.end(); ++c)
  {
    if (c->x()+c->width()-x()>w)
      w=c->x()+c->width()-x();
    if (c->y()+c->height()-y()>h)
      h=c->y()+c->height()-y();
  }
  if ((w!=width() || h!=height()))
    resize(w,h);
}

// when adding a child, enlarge the button box window if nessary to encompass it
void i4_button_box_class::add_button(i4_coord _x, i4_coord _y, i4_button_class *child)
{
  i4_parent_window_class::add_child(_x,_y,child);
  child->set_menu_parent(this);
  expand_if_needed();
}

void i4_button_box_class::arrange_right_down()
{
  i4_parent_window_class::arrange_right_down();
  expand_if_needed();
}
 
void i4_button_box_class::arrange_down_right()
{
  i4_parent_window_class::arrange_down_right();
  expand_if_needed();
}


void i4_button_box_class::note_reaction_sent(i4_menu_item_class *who,       // this is who sent it
                                             i4_event_reaction_class *ev,   // who it was to 
                                             i4_menu_item_class::reaction_type type)
{
  i4_button_class *but=(i4_button_class *)who;

  if (type==i4_menu_item_class::PRESSED)
  {
    if (current_down)
    {
      if (current_down!=but)  // see if we need to depress the current button
      {
        i4_button_class *old_down=current_down;
        current_down=but;
        old_down->do_depress();
      }  
    } else current_down=but;
  }
  else if (type==i4_menu_item_class::DEPRESSED)
  {
    if (but==current_down && require_one_down)
      current_down->do_press();         // sorry we need you to stay down
  }
}


void i4_button_box_class::push_button(i4_button_class *which, i4_bool send_event)
{
  if (!which) return;

  if (current_down)
  {
    if (current_down!=which)
    {
      current_down->do_depress(); 
      current_down=which;    
      current_down->do_press();
    }
  } else
  {
    current_down=which;
    current_down->do_press();
  }

  if (send_event)
    current_down->send_event(current_down->send.press, i4_menu_item_class::PRESSED); 
}
