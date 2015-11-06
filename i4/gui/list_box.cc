/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "gui/list_box.hh"
#include "gui/button.hh"
#include "gui/deco_win.hh"
#include "gui/image_win.hh"

void i4_list_box_class:: set_top(i4_menu_item_class *item)
{
  if (top)
    delete top;

  top=item->copy();

  int h=top->height() >= height()-(t+b) ? top->height() : height()-(t+b);

  top->set_menu_parent(this);
  top->resize(width()-(l+r)-down->width(), h);
  top->do_deactivate();

  i4_parent_window_class::add_child(l,t, top);
}

i4_list_box_class::~i4_list_box_class()
{
  hide();

  for (int i=0; i<entries.size(); i++)
    delete entries[i];

  if (top)
    delete top;

}

i4_list_box_class::i4_list_box_class(w16 w,         
                                     i4_graphical_style_class *style,
                                     i4_parent_window_class *root_window)

  : i4_menu_class(i4_F), entries(0,8), style(style), root_window(root_window)
{
  i4_image_class *down_im=style->icon_hint->down_icon;

  down=new i4_button_class(0, new i4_image_window_class(down_im), style,
                           0,
                           new i4_event_reaction_class(this, 1));
  down->set_popup(i4_T);

  l=1;  r=1;  t=1;  b=1;

  resize(w, down->height()+(t+b));



  i4_parent_window_class::add_child(w-down->width()-r, t, down);
  top=0;
  pull_down=0;
  current=0;
}


void i4_list_box_class::parent_draw(i4_draw_context_class &context)
{
  local_image->clear(0, context);
  //  style->draw_in_deco(local_image, 0,0,width()-1, height()-1, i4_F, context);
}


void i4_list_box_class::add_item(i4_menu_item_class *item)
{  


  if (item->height()+t+b>height())
    resize(width(), item->height()+t+b);

  if (entries.size()==0)
    set_top(item);

  entries.add(item);

  item->set_menu_parent(this);
}


void i4_list_box_class::set_current_item(int entry_num)
{
  if (entry_num>=0 && entry_num<entries.size())
  {
    set_top(entries[entry_num]);
    current=entry_num;
  }
}

class i4_list_pull_down_class : public i4_deco_window_class
{
  i4_window_class *buddy;
public:
  i4_list_pull_down_class(w16 w, w16 h, i4_window_class *buddy, i4_graphical_style_class *style) 
    : i4_deco_window_class(w,h, i4_T, style), buddy(buddy)
  {
  }
 
  void grab()
  {
    i4_window_request_mouse_grab_class grab(this);
    i4_kernel.send_event(parent, &grab);
  }

  void receive_event(i4_event *ev)
  {
    if (ev->type()==i4_event::MOUSE_BUTTON_DOWN)
    {
      CAST_PTR(mev, i4_mouse_move_event_class, ev);
      if (mev->x<0 || mev->y<0 || mev->x>=width() || mev->y>=height())
      {        
        i4_window_request_mouse_ungrab_class ungrab(this);
        i4_kernel.send_event(parent, &ungrab);

        i4_user_message_event_class uev(1);
        i4_kernel.send_event(buddy, &uev);
      }
    }

    i4_deco_window_class::receive_event(ev);
  }

  char *name() { return "list_box_pull_down"; }
};

void i4_list_box_class::show(i4_parent_window_class *show_on, i4_coord px, i4_coord py)
{
  if (!pull_down)
  {

    int i, y=0, x;

    for (i=0; i<entries.size(); i++)
      y+=entries[i]->height();

    pull_down=new i4_list_pull_down_class(width()-(l+r), y, this, style);
    y=pull_down->get_y1();
    x=pull_down->get_x1();

    for (i=0; i<entries.size(); i++)
    {
      entries[i]->resize(width(), entries[i]->height());

      pull_down->add_child(x,y, entries[i]);
      y+=entries[i]->height();
    }

    show_on->add_child(px,py, pull_down);
    pull_down->grab();
  }
}


void i4_list_box_class::hide()
{
  if (pull_down)
  {

    for (int i=0; i<entries.size(); i++)
      pull_down->remove_child(entries[i]);

    root_window->remove_child(pull_down);
    delete pull_down;
    pull_down=0;
  }
}



void i4_list_box_class::receive_event(i4_event *ev)
{
  if (ev->type()==i4_event::USER_MESSAGE)
  {
    if (!pull_down) 
      show(root_window, x(), y()+height());
    else hide();
  }
  else 
    i4_menu_class::receive_event(ev);
}


void i4_list_box_class::note_reaction_sent(i4_menu_item_class *who,       // this is who sent it
                                           i4_event_reaction_class *ev,   // who it was to 
                                           i4_menu_item_class::reaction_type type)
{
  if (type==i4_menu_item_class::PRESSED)
  {
    if (who==top)
      show(root_window, x(), y()+height());
    else
    {
      for (int i=0; i<entries.size(); i++)
        if (entries[i]==who)        
          set_current_item(i);

      hide();
    }
  }
}

