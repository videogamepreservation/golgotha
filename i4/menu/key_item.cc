/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "device/keys.hh"
#include "menu/key_item.hh"
#include "window/win_evt.hh"

static i4_key_accel_watcher_class i4_key_accel_watcher_instance;

enum { key_space=5 };

i4_key_item_class::~i4_key_item_class()
{
  delete text;
  if (use_key!=I4_NO_KEY)
    i4_key_accel_watcher_instance.unwatch_key(this, use_key, key_modifiers);
}

i4_key_item_class::i4_key_item_class(const i4_const_str &_text,
                                     i4_color_hint_class *color_hint,
                                     i4_font_hint_class *font_hint,
                                     i4_graphical_style_class *style,
                                     w16 key,
                                     w16 key_modifiers,
                                     w16 pad_left_right,
                                     w16 pad_up_down
                                     )
  : i4_menu_item_class(0,
                       style,
                       font_hint->normal_font->width(_text)+pad_left_right*2,
                       font_hint->normal_font->height(_text)+pad_up_down*2,
                       0,0,0,0),     
    key_modifiers(key_modifiers),
    color(color_hint),
    font(font_hint),
    text(new i4_str(_text,_text.length()+1)),
    pad_lr(pad_left_right),
    pad_ud(pad_up_down),
    use_key(key)
{
  key_focused=i4_F;
  valid=i4_T;
  
  w32 add_width=0;
  i4_font_class *fnt=font_hint->normal_font;

  if (key!=I4_NO_KEY)
  {
    i4_key_accel_watcher_instance.watch_key(this, key, key_modifiers);
    i4_str *key_name=i4_key_name(key, key_modifiers);
    add_width=fnt->width(*key_name)+key_space;
    delete key_name;
  }

  resize(fnt->width(_text) + add_width + pad_left_right*2,
         fnt->height(_text) + pad_up_down*2);
}

void i4_key_item_class::receive_event(i4_event *ev)
{
  if (ev->type()==i4_event::MOUSE_BUTTON_DOWN)
  {
    CAST_PTR(b,i4_mouse_button_down_event_class,ev);
    if (b->but==i4_mouse_button_down_event_class::LEFT)
    {
      do_depress();
      send_event(send.depress, PRESSED);
      action();
    }
    else i4_menu_item_class::receive_event(ev);

  } else if (ev->type()==i4_event::KEY_PRESS)
  {
    CAST_PTR(k,i4_key_press_event_class,ev);

    if (k->modifiers & I4_MODIFIER_CTRL)
      k->modifiers |= I4_MODIFIER_CTRL;
    if (k->modifiers & I4_MODIFIER_ALT)
      k->modifiers |= I4_MODIFIER_ALT;
    if (k->modifiers & I4_MODIFIER_SHIFT)
      k->modifiers |= I4_MODIFIER_SHIFT;
      
    if (k->key_code==use_key && k->modifiers==key_modifiers)
      action();
    else if (k->key==I4_ENTER)
    {
      do_depress();
      send_event(send.depress, PRESSED);
      action();
    }
    else if (k->key==I4_TAB)
    {
      i4_window_message_class nf(i4_window_message_class::REQUEST_NEXT_KEY_FOCUS,this);
      i4_kernel.send_event(parent, &nf);
    }
    else if (k->key==I4_LEFT)
    {
      i4_window_message_class l(i4_window_message_class::REQUEST_LEFT_KEY_FOCUS,this);
      i4_kernel.send_event(parent, &l);
    }
    else if (k->key==I4_RIGHT)
    {
      i4_window_message_class r(i4_window_message_class::REQUEST_RIGHT_KEY_FOCUS,this);
      i4_kernel.send_event(parent, &r);
    }
    else if (k->key==I4_UP)
    {
      i4_window_message_class u(i4_window_message_class::REQUEST_UP_KEY_FOCUS,this);
      i4_kernel.send_event(parent, &u);
    }
    else if (k->key==I4_DOWN)
    {
      i4_window_message_class d(i4_window_message_class::REQUEST_DOWN_KEY_FOCUS,this);
      i4_kernel.send_event(parent, &d);
    }
    else 
      i4_menu_item_class::receive_event(ev);
  } 
  else if (ev->type()==i4_event::WINDOW_MESSAGE)
  {
    CAST_PTR(wev, i4_window_message_class, ev);
    if (wev->sub_type==i4_window_message_class::GOT_KEYBOARD_FOCUS)
      key_focused=i4_T;
    else if (wev->sub_type==i4_window_message_class::LOST_KEYBOARD_FOCUS)
      key_focused=i4_F;
    else if (wev->sub_type==i4_window_message_class::NOTIFY_RESIZE)
    {
      CAST_PTR(rev, i4_window_notify_resize_class, ev);
      if (rev->from()==parent)
      {
        w32 nw=rev->new_width-(x()-parent->x())*2;
        if (nw!=width())
          resize(nw, height());
      }
    }

    i4_menu_item_class::receive_event(ev);
  }
  else 
    i4_menu_item_class::receive_event(ev);
}


i4_key_accel_watcher_class::i4_key_accel_watcher_class()
{
  initialized=i4_F;
  memset(user,0,sizeof(user));
  total=0;
}


i4_key_item_class **i4_key_accel_watcher_class::
   key_item_pointer_type::get_from_modifiers(w16 modifiers)
{
  int index=0;

  if (modifiers & I4_MODIFIER_SHIFT)
    index |= 1;

  if (modifiers & I4_MODIFIER_CTRL)
    index |= 2;
    
  if (modifiers & I4_MODIFIER_ALT)
    index |= 3;

  return &modkey[index];
}

void i4_key_accel_watcher_class::watch_key(i4_key_item_class *who, w16 key, w16 modifiers)
{
  if (!initialized)
  {
    i4_kernel.request_events(this,  
                             i4_device_class::FLAG_KEY_PRESS |
                             i4_device_class::FLAG_KEY_RELEASE);
    initialized=i4_T;
  }

  I4_TEST(key<I4_NUM_KEYS, "Key out of range");
  if (*user[key].get_from_modifiers(modifiers))
  {
    char name[80];
    i4_warning("key alread has function [%s]\n", i4_get_key_name(key,modifiers,name));
  }
  else total++;

  *user[key].get_from_modifiers(modifiers)=who;
}

void i4_key_accel_watcher_class::unwatch_key(i4_key_item_class *who, w16 key, w16 modifiers)
{
  if (*user[key].get_from_modifiers(modifiers)==who)
  {
    *user[key].get_from_modifiers(modifiers)=0;
    total--;
    if (total==0)
    {      
      i4_kernel.unrequest_events(this,  
                                 i4_device_class::FLAG_KEY_PRESS |
                                 i4_device_class::FLAG_KEY_RELEASE);
      initialized=i4_F;
    }
  }
}



void i4_key_accel_watcher_class::receive_event(i4_event *ev)
{
  if (ev->type()==i4_event::KEY_PRESS)
  {
    CAST_PTR(kev, i4_key_press_event_class, ev);

    // if the object has the keyboard focus, then it will get the key through
    // normal window channels
    i4_key_item_class *i=*(user[kev->key_code].get_from_modifiers(kev->modifiers));
    if (i && !i->has_keyboard_focus())      
      i4_kernel.send_event(i, ev);
  }
}



void i4_key_item_class::parent_draw(i4_draw_context_class &context)
{
  local_image->add_dirty(0,0,width()-1,height()-1,context);

  i4_color fg,bg;
  

  if (active)
  {
    fg=color->selected_text_foreground;
    bg=color->selected_text_background;
  }
  else
  {
    fg=color->text_foreground;
    bg=color->text_background;
  }

  i4_str *key_name;

  if (use_key!=I4_NO_KEY)
    key_name=i4_key_name(use_key, key_modifiers);
  else key_name=0;

  if (!active)
    hint->deco_neutral_fill(local_image, 0,0, width()-1, height()-1, context);
  else
    local_image->clear(bg, context);

  i4_font_class *fnt=font->normal_font;

  if (!valid)
  {
    fg=color->window.passive.dark;

    fnt->set_color(color->window.passive.bright);
    fnt->put_string(local_image, pad_lr+1, pad_ud+1, *text,context);

    if (key_name)
      fnt->put_string(local_image, width() - pad_lr - fnt->width(*key_name) + 1, 
                      pad_ud + 1,
                      *key_name, context);

  }

  if (valid || !active)
  {
    font->normal_font->set_color(fg);
    font->normal_font->put_string(local_image, pad_lr, pad_ud, *text,context);
    if (key_name)
      fnt->put_string(local_image, width() - pad_lr - fnt->width(*key_name), 
                      pad_ud, *key_name, context);

  }

  if (key_name)
    delete key_name;
}



