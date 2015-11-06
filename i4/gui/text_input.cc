/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/


#include "gui/text_input.hh"

#include "time/timedev.hh"
#include "device/event.hh"
#include "window/win_evt.hh"
#include "device/keys.hh"
#include "device/key_man.hh"

class cursor_blink_class : public i4_object_message_event_class
{
  public :
  cursor_blink_class(void *object) : i4_object_message_event_class(object) {}
  virtual i4_event  *copy() { return new cursor_blink_class(object); }  
} ;

sw32 i4_text_input_class::get_number()
{
  i4_str::iterator is=get_edit_string()->begin();
  return is.read_number();
}

i4_text_input_class::i4_text_input_class(i4_graphical_style_class *style,
                                         const i4_const_str &default_string,
                                         w32 width,       // width in pixels of window
                                         w32 _max_width,
                                         i4_event_handler_class *change_notify,
                                         i4_font_class *_font)
  : style(style),
    i4_window_class(0,0),
    change_notify(change_notify)

{
  if (!_font)
    font=style->font_hint->normal_font;
  else
    font=_font;

  max_width=_max_width;
  sent_change=i4_F;
  changed=i4_F;
  selected=i4_F;
  cursor_on=i4_F;
  selecting=i4_F;
  need_cancel_blink=i4_F;
    
  w32 l,t,r,b;
  style->get_in_deco_size(l,t,r,b);
  private_resize(width+l+r, font->height(default_string)+1+t+b);

  if (max_width<default_string.length())         // need a string at least as long as default
    max_width=default_string.length();

  st=new i4_str(default_string,max_width+1);

  cursor_x=default_string.length();

  hi_x1=0;   // nothing hilighted by default
  hi_x2=0;
  left_x=0;
}


void i4_text_input_class::draw_deco(i4_draw_context_class &context)
{
  style->draw_in_deco(local_image, 0,0, width()-1, height()-1, selected, context);
}

void i4_text_input_class::draw(i4_draw_context_class &context)
{        
  sw32 cx1,cy1,cy2;
  w32 char_on=0;
  w32 x;
  i4_color fg;
  i4_const_str::iterator c=st->begin();
  w32 l,t,r,b;

  local_image->add_dirty(0,0, width()-1, height()-1, context);

  style->get_in_deco_size(l,t,r,b);
  cx1=l;
  cy1=t;
  cy2=height()-1-b;


  for (x=0; x<left_x; x++)
  {
    ++c;
    char_on++;
  }

  style->deco_neutral_fill(local_image, 0,0, width()-1, height()-1, context);
  //  local_image->clear(style->color_hint->text_background,context);
  w32 proper_hi_x1=hi_x1>hi_x2 ? hi_x2 : hi_x1;
  w32 proper_hi_x2=hi_x1>hi_x2 ? hi_x1 : hi_x2;
      

  for (x=0; x<width() && c!=st->end(); )
  {

    if (char_on>=proper_hi_x1 && char_on<proper_hi_x2)
    {
      fg=style->color_hint->selected_text_foreground;
      local_image->bar(cx1+x,               cy1+1,
                       cx1+x+font->width(c.get()), cy2,
                       style->color_hint->selected_text_background,
                       context);
    } else
      fg=style->color_hint->text_foreground;

    font->set_color(fg);
    if (c!=st->end())
    {
      font->put_character(local_image,cx1+x,cy1+1,c.get(),context);
      if (cursor_on && cursor_x==char_on)
        local_image->bar(cx1+x,cy1+1,
                         cx1+x,cy2-2,style->color_hint->white,context);

      x+=font->width(c.get());
      ++c;
    }
    else if (cursor_on && cursor_x==char_on)
        local_image->bar(cx1+x,cy1+1, cx1+x,cy2-2,style->color_hint->white,context);



    char_on++;
  }

  if (cursor_on && cursor_x==char_on)
    local_image->bar(cx1+x,cy1+1,
                     cx1+x,cy2-2,style->color_hint->white,context);



  draw_deco(context);

}


// this will find the character the mouse is on, this should work for non-constantly spaced
// character strings as well
w32 i4_text_input_class::mouse_to_cursor()
{
  i4_const_str::iterator c=st->begin();

  w32 char_on=0;
  w32 x,fw;
  if (last_x<0)
    last_x=0;

  for (x=0; x<left_x && c!=st->end(); x++)
  {
    ++c;
    char_on++;
  }
    
  for (x=0; x<width() && c!=st->end(); )
  {
    fw=font->width(c.get());
    if (last_x<=x+fw/2+1)
      return char_on;

    x+=fw;
    char_on++;
    ++c;
  }
  return char_on;
}

void i4_text_input_class::request_blink()
{
  if (!need_cancel_blink)
  {
    blink_timer=i4_time_dev.request_event(this,
                                          new cursor_blink_class(this),
                                          500);
    need_cancel_blink=i4_T;
  }
}

void i4_text_input_class::stop_blink()
{
  if (need_cancel_blink)
  {
    i4_time_dev.cancel_event(blink_timer);
    need_cancel_blink=i4_F;
  }
}

void i4_text_input_class::sustain_cursor()
{
  cursor_on=i4_T;
  request_redraw();
}

void i4_text_input_class::del_selected()
{
        
  if (hi_x1!=hi_x2)
  {
    i4_coord swap;
    if (hi_x1>hi_x2)
    {
      swap=hi_x1;
      hi_x1=hi_x2;
      hi_x2=swap;
    }

    cursor_x=hi_x1;

    i4_str::iterator start=st->begin(),end=st->begin();
    while (hi_x1)
    {
      ++start;
      hi_x2--;
      hi_x1--;
    }
    end=start;
    while (hi_x2)
    {
      ++end;
      hi_x2--;
    }
    st->remove(start,end);
  }
  request_redraw();
}

void i4_text_input_class::move_cursor_right()
//{{{
{
  if (cursor_x<st->length())
  {
    cursor_x++;

    i4_const_str::iterator c=st->begin();
    w32 cur_left,x;

    for (x=0; x<left_x && c!=st->end(); x++)
      ++c;
 

    for (cur_left=cursor_x-left_x; cur_left && c!=st->end(); cur_left--)
    {
      x+=font->width(c.get());
      ++c;      
    }
    if (x>=width())
    {
      left_x+=4;
    }
    sustain_cursor();
  }
}
//}}}


void i4_text_input_class::move_cursor_left()
{
  if (cursor_x)
  {
    cursor_x--;
    if (cursor_x<left_x)
    {
      if (left_x>4)
        left_x-=4;
      else left_x=0;
    }
    sustain_cursor();
  }
}


void i4_text_input_class::move_cursor_end()
//{{{
{
  w32 left_pos, pos;
  i4_const_str::iterator c;

  cursor_x = st->length();
  left_x = 0;
  pos = 0;
  for (c=st->begin(); c!=st->end(); ++c)
    pos += font->width(c.get());
  left_pos = pos - width();

  pos = 0;
  for (c=st->begin(); c!=st->end() && pos<left_pos; ++c)
  {
    pos += font->width(c.get());
    left_x++;
  }

  sustain_cursor();
}
//}}}


void i4_text_input_class::become_active()
{
  i4_window_request_key_grab_class kgrab(this);
  send_to_parent(&kgrab);
      
  selected=i4_T;
  request_redraw();
  if (!cursor_on)      // make sure cursor is not draw when we are not in focus
    cursor_on=i4_T;
  request_blink();
}

void i4_text_input_class::become_unactive()
{
  selected=i4_F;
  request_redraw();

  if (cursor_on)      // make sure cursor is not draw when we are not in focus
    cursor_on=i4_F;

  if (hi_x1!=hi_x2)
  {
    hi_x1=hi_x2;
    request_redraw();
  }

  stop_blink();

  if (selecting)
  {
    i4_window_request_mouse_ungrab_class ungrab(this);
    send_to_parent(&ungrab);
    selecting=i4_F;
  }

}
  
void i4_text_input_class::receive_event(i4_event *ev)
{
  switch (ev->type())
  {
    case i4_event::WINDOW_MESSAGE :        
    {
      CAST_PTR(wev,i4_window_message_class,ev);
      switch (wev->sub_type)
      {
        case i4_window_message_class::GOT_DROP :
        {
          CAST_PTR(dev, i4_window_got_drop_class, ev);

          if (dev->drag_info.drag_object_type==i4_drag_info_struct::FILENAMES)
          {
            if (dev->drag_info.t_filenames==1)
            {
              change_text(*dev->drag_info.filenames[0]);
              if (change_notify)
              {
                i4_text_change_notify_event o(this, st);
                i4_kernel.send_event(change_notify, &o);                
                sent_change=i4_T;
              }
            }
          }

        } break;

        case i4_window_message_class::GOT_MOUSE_FOCUS :
        {
          set_cursor(style->cursor_hint->text_cursor()->copy());
        } break;

        case i4_window_message_class::LOST_MOUSE_FOCUS :
        {
          set_cursor(0);
        } break;


        case i4_window_message_class::LOST_KEYBOARD_FOCUS :
        {
          become_unactive(); 

          if (change_notify && changed && !sent_change)
          {
            i4_text_change_notify_event o(this, st);
            i4_kernel.send_event(change_notify, &o);
            sent_change=i4_T;
          }
        } break;
          
        default :          
          i4_window_class::receive_event(ev);
      }
    } break;

    case i4_event::OBJECT_MESSAGE :
    {
      CAST_PTR(oev,i4_object_message_event_class,ev);
      if (oev->object==this)                              // was this an event we created?
      {
        need_cancel_blink=i4_F;
        cursor_on=(i4_bool)(!cursor_on);
        request_redraw();
        request_blink();
      }
    } break;

    case i4_event::MOUSE_BUTTON_DOWN :
    {
      CAST_PTR(bev,i4_mouse_button_down_event_class,ev);
      if (bev->but==i4_mouse_button_down_event_class::LEFT)
      {
        become_active();

        selecting=i4_T;
          
        hi_x1=mouse_to_cursor();
        hi_x2=hi_x1;
        cursor_x=hi_x1;

        i4_window_request_mouse_grab_class grab(this);
        send_to_parent(&grab);


      }

    } break;

    case i4_event::MOUSE_BUTTON_UP :
    {       
      CAST_PTR(bev,i4_mouse_button_up_event_class,ev);
      if (bev->but==i4_mouse_button_up_event_class::LEFT && selecting)
      {
        selecting=i4_F;
        i4_window_request_mouse_ungrab_class ungrab(this);
        send_to_parent(&ungrab);

      }
    } break;


    case i4_event::MOUSE_MOVE : 
    {
      CAST_PTR(mev,i4_mouse_move_event_class,ev);
      last_x=mev->x;
      last_y=mev->y;
      if (selecting)
      {
        w32 old_hi_x2=hi_x2;
        hi_x2=mouse_to_cursor();
        if (old_hi_x2!=hi_x2)
        {
          request_redraw();
          cursor_x=hi_x2;
        }
      }
    } break;

    case i4_event::KEY_PRESS :
    {
      CAST_PTR(kev,i4_key_press_event_class,ev);

      switch (kev->key)
      {
        case I4_BACKSPACE :
        {
          if (hi_x1!=hi_x2)
            del_selected();
          else if (cursor_x)
          {
            cursor_x--;
            w32 cur=cursor_x;
            i4_str::iterator cursor1=st->begin(),cursor2=st->begin();
            while (cur)
            {
              cur--;
              ++cursor1;
            }
            cursor2=cursor1;
            ++cursor2;
            st->remove(cursor1,cursor2);
            sustain_cursor();
          }
          note_change();
        

        } break;
        
        case I4_LEFT :
        {
          move_cursor_left();
        } break;

        case I4_RIGHT :
        {
          move_cursor_right();
        } break;

        case I4_HOME :
        {
          cursor_x = 0;
          left_x = 0;
          sustain_cursor();
        } break;

        case I4_END :
        {
          cursor_x = 0;
          left_x = 0;
          sustain_cursor();
        } break;

        case I4_ENTER :
        {
          if (change_notify && changed && !sent_change)
          {
            i4_text_change_notify_event o(this, st);
            i4_kernel.send_event(change_notify, &o);
            sent_change=i4_T; 
          }
        } break;

        default :
        {
          if (kev->key>=' ' && kev->key<='~')
          {
            note_change();
        
            if (hi_x1!=hi_x2)
              del_selected();
            w32 cur=cursor_x;
            i4_str::iterator cursor=st->begin();
            while (cur)
            {
              cur--;
              ++cursor;
            }

            st->insert(cursor,kev->key);
            move_cursor_right();
          }

        } break;
      }
    } break;


    case i4_event::QUERY_MESSAGE :
    {
      CAST_PTR(qev,i4_query_text_input_class,ev);
      qev->copy_of_data=new i4_str(*st,st->length()+1);
    } break;

    default :
      i4_window_class::receive_event(ev);
  }
}

i4_text_input_class::~i4_text_input_class()
{
  if (change_notify && changed && !sent_change)
  {
    i4_text_change_notify_event o(this, st);
    i4_kernel.send_event(change_notify, &o);
    sent_change=i4_T; 
  }

  stop_blink();
  delete st;
}


i4_window_class *i4_create_text_input(i4_graphical_style_class *style,
                                      const i4_const_str &default_string,
                                      w32 width,        // width in pixels of window
                                      w32 max_width)
{
  i4_text_input_class *ti=new i4_text_input_class(style,
                                                  default_string,
                                                  width,
                                                  max_width);



  return ti;
}

void i4_text_input_class::change_text(const i4_const_str &new_st)
{
  delete st;
  st=new i4_str(new_st, max_width);
  request_redraw();
  left_x=0;
  cursor_x=0;
}




