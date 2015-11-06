/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "menu/pull.hh"
#include "window/win_evt.hh"
#include "device/device.hh"
#include "device/keys.hh"

i4_pull_menu_class::i4_pull_menu_class(i4_graphical_style_class *style, 
                                       i4_parent_window_class *root_window)
  : i4_menu_class(i4_F),
    menu_colors(*style->color_hint),
    root(root_window),
    style(style)
{
  active_top=0;
  active_sub=0;
  sub_focused=i4_F;
  watching_mouse=i4_F;

  menu_colors.selected_text_foreground=i4_read_color_from_resource("selected_menu_foreground");
  menu_colors.selected_text_background=i4_read_color_from_resource("selected_menu_background");
}

void i4_pull_menu_class::watch()
{
  if (!watching_mouse)
  {
    watching_mouse=i4_T;
    i4_kernel.request_events(this,     
                             i4_device_class::FLAG_KEY_PRESS |
                             i4_device_class::FLAG_KEY_RELEASE |
                             i4_device_class::FLAG_MOUSE_BUTTON_DOWN |
                             i4_device_class::FLAG_MOUSE_BUTTON_UP);

  }
}

void i4_pull_menu_class::unwatch()
{
  if (watching_mouse)
  {
    i4_kernel.unrequest_events(this,
                               i4_device_class::FLAG_KEY_PRESS |
                               i4_device_class::FLAG_KEY_RELEASE |
                               i4_device_class::FLAG_MOUSE_BUTTON_DOWN |
                               i4_device_class::FLAG_MOUSE_BUTTON_UP);

    watching_mouse=i4_F;
  }
}

i4_pull_menu_class::~i4_pull_menu_class()
{
  unwatch();
  hide_active();

  sub_menus.destroy_all();
}


void i4_pull_menu_class::show_active(i4_menu_item_class *who)
{
  win_iter name=children.begin(), sub=sub_menus.begin(), last=sub_menus.end();
  while (who!=&*name)
  {
    last=sub;
    ++name;
    ++sub;
  }

  sw32 px,py;

  py=who->y()+who->height();
  px=who->x();

  i4_menu_class *m=((i4_menu_class *)(&*sub));

  if (last==sub_menus.end())
    sub_menus.erase();
  else
    sub_menus.erase_after(last);


  m->show(root, px-root->x(), py-root->y());

  if (sub->height()+sub->y()>root->height())
  {
    m->hide();
    py=who->y()-sub->height();
    m->show(root, px-root->x(), py-root->y());
  }

   
  if (sub->width()+sub->x()>root->width())
  {
    m->hide();
    px=root->width()-sub->width();
    m->show(root, px-root->x(), py-root->y());
  }    

  active_top=who;
  active_sub=m;
  watch();
}

void i4_pull_menu_class::note_reaction_sent(i4_menu_item_class *who,       // this is who sent it
                                            i4_event_reaction_class *ev,   // who it was to 
                                            i4_menu_item_class::reaction_type type)
{
  i4_menu_class::note_reaction_sent(who, ev, type);

  if (type==i4_menu_item_class::PRESSED)
  {
    who->do_depress();
    if (!active_sub)
      show_active(who);
  } else if (type==i4_menu_item_class::ACTIVATED)
  {
    if (active_sub)
    {
      hide_active();
      show_active(who);
    }
      
    active_top=who;
  } else if (type==i4_menu_item_class::DEACTIVATED)
  {
    if (active_sub)
      who->do_activate();
  }
}

void i4_pull_menu_class::hide_active()
{
  if (active_sub)
  {
    active_sub->hide();

    if ((i4_window_class *)active_top==&*children.begin())
      sub_menus.insert(*active_sub);
    else
    {
      win_iter s=sub_menus.begin(), t=children.begin();
      ++t;
      for (;(*&t)!=active_top;)
      {
        ++s;
        ++t;
      }
      sub_menus.insert_after(s, *active_sub);
    }
    unwatch();
  }

  if (active_top)
    active_top->do_deactivate();

  active_top=0;
  active_sub=0;
}

void i4_pull_menu_class::receive_event(i4_event *ev)
{
  if (ev->type()==i4_event::OBJECT_MESSAGE)
  {
    CAST_PTR(lev, i4_menu_focus_event_class, ev);

    if (lev->focus_state==i4_menu_focus_event_class::lost_focus)
      sub_focused=i4_F;
    else
      sub_focused=i4_T;
  } 
  else if (ev->type()==i4_event::WINDOW_MESSAGE)
  {
    CAST_PTR(wev, i4_window_message_class, ev);
    if (wev->sub_type==i4_window_message_class::LOST_MOUSE_FOCUS)
      focused=i4_F;
    else if (wev->sub_type==i4_window_message_class::GOT_MOUSE_FOCUS)
      focused=i4_T;      
    
    i4_menu_class::receive_event(ev);
  }
  else if (ev->type()==i4_event::MOUSE_BUTTON_DOWN ||
           ev->type()==i4_event::MOUSE_BUTTON_UP)
  {
    if (!focused && !sub_focused)
      hide_active();

    if (focused)
      i4_menu_class::receive_event(ev);
  }
  else if (ev->type()==i4_event::KEY_PRESS)
  {
    CAST_PTR(kev, i4_key_press_event_class, ev);
    if (kev->key_code==I4_ESC)
      hide_active();
    else
      i4_menu_class::receive_event(ev);
  }  
  else if (focused)
    i4_menu_class::receive_event(ev);
}


void i4_pull_menu_class::parent_draw(i4_draw_context_class &context)
{
  i4_rect_list_class child_clip(&context.clip,0,0);
  child_clip.intersect_list(&undrawn_area);

  child_clip.swap(&context.clip);

  style->deco_neutral_fill(local_image, 0,0, width()-1, height()-1, context);


  //  local_image->clear(menu_colors.text_background,context);

  child_clip.swap(&context.clip);
  i4_parent_window_class::parent_draw(context);
}

void i4_pull_menu_class::add_sub_menu(i4_menu_item_class *name,
                                      i4_menu_class *sub_menu)
{
  add_item(name);
  name->set_menu_parent(this);
  sub_menu->notify_focus_change(this);

  sub_menus.insert_end(*sub_menu);

  reparent(local_image, parent);
}

void i4_pull_menu_class::reparent(i4_image_class *draw_area, i4_parent_window_class *parent)
{
  i4_parent_window_class::reparent(draw_area, parent);
  if (parent)
  {
    resize(parent->width(), 0);
    arrange_right_down();

    w32 max_h=0;
    for (win_iter c=children.begin(); c!=children.end(); ++c)
      if (c->y()-y()+c->height()>max_h)
        max_h=c->y()-y()+c->height();

    resize(parent->width(), max_h);
  }
}

void i4_pull_menu_class::resize(w16 new_width, w16 new_height)
{
  i4_window_class::resize(new_width, new_height);
  arrange_right_down();
}



void i4_pull_menu_class::hide()
{
  if (parent)     
  {
    parent->remove_child(this);
    hide_active();
  }
  parent=0;
}
