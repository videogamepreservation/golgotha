/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "menu/textitem.hh"
#include "device/keys.hh"
#include "window/win_evt.hh"
#include "device/key_man.hh"
#include "device/keys.hh"

void i4_text_item_class::change_text(const i4_const_str &new_st)
{
  delete text;
  text=new i4_str(new_st, new_st.length()+1);
  request_redraw(i4_F);
}

i4_menu_item_class *i4_text_item_class::copy() 
{
  return new i4_text_item_class(*text, hint, color, font, 
                                send.press ? send.press->copy() : 0,
                                send.depress ? send.depress->copy() : 0,
                                send.activate ? send.activate->copy() : 0,
                                send.deactivate ? send.deactivate->copy() : 0);
}


i4_text_item_class::i4_text_item_class(const i4_const_str &_text,
                                       i4_graphical_style_class *style,
                                       i4_color_hint_class *_color,
                                       i4_font_class *_font,
                                       i4_event_reaction_class *press,
                                       i4_event_reaction_class *depress,
                                       i4_event_reaction_class *activate,
                                       i4_event_reaction_class *deactivate,
                                       w16 pad_left_right,
                                       w16 pad_up_down)
      
  : i4_menu_item_class(0, style, 0,0, press,depress,activate,deactivate),
    color(_color),
    font(_font),
    text(new i4_str(_text,_text.length()+1)),
    pad_lr(pad_left_right)
{
  if (!color) color=style->color_hint;
  if (!font)  font=style->font_hint->normal_font;

  bg_color=style->color_hint->neutral();

  resize(font->width(_text)+pad_left_right*2, font->height(_text)+pad_up_down*2);
  
  // show the keyboard short cut for commands
  if (press && press->event && press->event->type()==i4_event::DO_COMMAND)
  {
    CAST_PTR(dev, i4_do_command_event_class, press->event);

    i4_key key;
    w16 mod;

    if (i4_key_man.get_key_for_command(dev->command_id, key, mod))
    {      
      i4_str *key_name=i4_key_name(key, mod);
      private_resize(width() + font->width(*key_name)+5, height());
      delete key_name;
    }
  }


}


void i4_text_item_class::parent_draw(i4_draw_context_class &context)
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

  if (!active)
  {
    if (bg_color==color->neutral())
    {
      for (i4_rect_list_class::area_iter c=context.clip.list.begin();c!=context.clip.list.end();++c)
        hint->deco_neutral_fill(local_image, c->x1, c->y1, c->x2, c->y2, context);
    }
    else local_image->clear(bg_color, context);

  }
  else
    local_image->clear(bg, context);

  font->set_color(fg);

  int dy=height()/2-font->largest_height()/2;
  font->put_string(local_image,
                   pad_lr,
                   dy,
                   *text,context);  
  

  // draw key name if there is one
  if (send.press && send.press->event && send.press->event->type()==i4_event::DO_COMMAND)
  {
    CAST_PTR(dev, i4_do_command_event_class, send.press->event);

    i4_key key;
    w16 mod;

    if (i4_key_man.get_key_for_command(dev->command_id, key, mod))
    {      
      i4_str *key_name=i4_key_name(key, mod);
      font->put_string(local_image, width() - pad_lr - 
                       font->width(*key_name) + 1, 
                       dy, *key_name, context);
    }
  }


}


void i4_text_item_class::receive_event(i4_event *ev)
{
  if (ev->type()==i4_event::MOUSE_BUTTON_DOWN)
  {
    CAST_PTR(b,i4_mouse_button_down_event_class,ev);
    if (b->but==i4_mouse_button_down_event_class::LEFT)
    {        
      do_press();
      send_event(send.press, PRESSED);

      do_depress();
      send_event(send.depress, DEPRESSED);

    } else i4_menu_item_class::receive_event(ev);
  } else if (ev->type()==i4_event::KEY_PRESS)
  {
    CAST_PTR(k,i4_key_press_event_class,ev);
    if (k->key==I4_ENTER)
    {
      do_press();
      send_event(send.press, PRESSED);

      do_depress();
      send_event(send.depress, DEPRESSED);
    } else if (k->key==I4_TAB)
    {
      i4_window_message_class tab(i4_window_message_class::REQUEST_NEXT_KEY_FOCUS,this);
      i4_kernel.send_event(parent, &tab);      
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
    else i4_menu_item_class::receive_event(ev);
  } else 
    i4_menu_item_class::receive_event(ev);
}
