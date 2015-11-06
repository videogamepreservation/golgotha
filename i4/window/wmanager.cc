/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "window/wmanager.hh"
#include "error/error.hh"
#include "window/style.hh"
#include "error/error.hh"
#include "window/win_evt.hh"



i4_color i4_window_manager_class::i4_read_color_from_resource(char *resource)
{
  i4_const_str::iterator i=i4gets(resource).begin();
  i4_color red=i.read_number();
  i4_color grn=i.read_number();
  i4_color blue=i.read_number();


  return i4_pal_man.convert_32_to( (red<<16)|(grn<<8)|(blue),
                                   &local_image->get_pal()->source);

}

void i4_window_manager_class::set_background_color(w32 color) 
{ 
  background_color=color; 
}

void i4_window_manager_class::parent_draw(i4_draw_context_class &context)
{     
  i4_rect_list_class child_clip(&context.clip,0,0);
  child_clip.intersect_list(&undrawn_area);   // intersect clipped area with area that needs to be drawn

  child_clip.swap(&context.clip);                 // make this the display's new clip list

  local_image->clear(background_color,context);          // clear out this area

  child_clip.swap(&context.clip);                 // restore the display's clip list
}

void i4_window_manager_class::root_draw()  // called by application
{
//   i4_draw_context_class c(0,0,639,439);
//   display->get_screen()->clear(0, c);

  i4_parent_window_class::draw(*display->get_context());

  display->flush();                                   // tell the display to page-flip or copy-dirty rects
}
 
  // a window manager does not actually get associated with anything until prepare_for_mode
i4_window_manager_class::i4_window_manager_class() : i4_parent_window_class(0,0)
{
  drag_drop.active=i4_F;
  display=0;
  devices_present=0;
  style=0;
  default_cursor=0;
  background_color=0;
  no_cursor_installed=i4_T;
  key_modifiers_pressed=0;
}

i4_window_manager_class::~i4_window_manager_class()
{
  cleanup_old_mode();
}

void i4_window_manager_class::receive_event(i4_event *ev)
{
  if (ev->type()==i4_event::WINDOW_MESSAGE)
  {
    CAST_PTR(cc,i4_window_change_cursor_class,ev);
    if (cc->sub_type==i4_window_message_class::CHANGE_CURSOR)
    {
      if (cc->cursor)
      {
        display->set_mouse_shape(cc->cursor);
        no_cursor_installed=i4_F;
      }
      else if (default_cursor)  // make sure we have default cursor first
        display->set_mouse_shape(default_cursor);
    }
    else if (cc->sub_type==i4_window_message_class::REQUEST_DRAG_DROP_START)
    {
      CAST_PTR(drag, i4_window_request_drag_drop_start_class, ev);
      drag_drop.active=i4_T;
      drag_drop.reference_id=drag->reference_id;
      drag_drop.further_info=drag->further_info;
      drag_drop.originator=drag->from();
      if (drag->drag_cursor)
        display->set_mouse_shape(drag->drag_cursor);
    } 
    else if (cc->sub_type==i4_window_message_class::REQUEST_DRAG_DROP_END)
    {
      i4_error("fixme");
//       drag_drop.active=i4_F;
//       if (default_cursor)
//         display->set_mouse_shape(default_cursor);

//       i4_window_class *prev_from=drag_frop

//       i4_window_got_drop_class drop(drag_drop.originator,
//                                     drag_drop.reference_id,
//                                     drag_drop.further_info);

//       i4_parent_window_class::receive_event(&drop);
    }
  
    else if (cc->sub_type==i4_window_message_class::REQUEST_MOUSE_GRAB)
    {
      CAST_PTR(grab,i4_window_request_mouse_grab_class,ev);
      
      i4_parent_window_class::receive_event(ev);
    }
    else if (cc->sub_type==i4_window_message_class::REQUEST_MOUSE_UNGRAB)
    {
      CAST_PTR(ungrab,i4_window_request_mouse_ungrab_class,ev);
      
      i4_parent_window_class::receive_event(ev);
      ungrab->return_result=i4_T;
    }  
    else i4_parent_window_class::receive_event(ev);

  } else if (ev->type()==i4_event::DISPLAY_CHANGE)
  {

    i4_image_class *im=display->get_screen();
    private_resize(im->width(),im->height());         // in case the resolution changed
    reparent(im,0);                           // grab the new image

  } 
  else
  {
    switch (ev->type())
    {
      case i4_event::KEY_PRESS :
        key_modifiers_pressed=((i4_key_press_event_class *)ev)->modifiers;
        break;

      case i4_event::KEY_RELEASE :
        key_modifiers_pressed=((i4_key_release_event_class *)ev)->modifiers;
        break;

      case i4_event::MOUSE_BUTTON_DOWN :
      case i4_event::MOUSE_BUTTON_UP :
      {
        CAST_PTR(bev, i4_mouse_button_event_class,ev);
        if (bev->time.milli_diff(bev->last_time)<style->time_hint->double_click)
          bev->double_click=i4_T;

      } break;
    }

    i4_parent_window_class::receive_event(ev);
  }


  if (drag_drop.active && ev->type()==i4_event::MOUSE_MOVE)
  {
    CAST_PTR(mouse_move, i4_mouse_move_event_class, ev);
    i4_window_drag_drop_move_class move(drag_drop.originator,
                                        mouse_move->x,
                                        mouse_move->y,
                                        drag_drop.reference_id,
                                        drag_drop.further_info);
    i4_parent_window_class::receive_event(&move);
                                        
  }
}

void i4_window_manager_class::set_default_cursor(i4_cursor_class *cursor)
{
  if (default_cursor)
    delete default_cursor;

  default_cursor=cursor->copy();

  if (no_cursor_installed)
    display->set_mouse_shape(cursor);
}


i4_graphical_style_class *i4_window_manager_class::get_style()
{
  return style;
}

void i4_window_manager_class::prepare_for_mode(i4_display_class *display, 
                                               i4_display_class::mode *mode)
{
  cleanup_old_mode();

  i4_window_manager_class::display=display;

  const i4_const_str s=i4gets("window_manager",i4_F);
  
  if (!s.null())
  {
    char buf[100];
    i4_const_str::iterator i=s.begin();
    i.read_ascii(buf,100);

    style=i4_style_man.find_style(buf);
  }
  else style=0;

  if (!style)
    style=i4_style_man.first_style();


  if (!style)
     i4_error("No window styles installed");




  i4_image_class *im=display->get_screen();
  resize(im->width(),im->height());
  reparent(im,0);


  style->prepare_for_mode(local_image->get_pal(), mode);

  i4_kernel.request_events(this,
                           i4_device_class::FLAG_MOUSE_BUTTON_UP|
                           i4_device_class::FLAG_MOUSE_BUTTON_DOWN|
                           i4_device_class::FLAG_MOUSE_MOVE|
                           i4_device_class::FLAG_KEY_PRESS|
                           i4_device_class::FLAG_KEY_RELEASE|
                           i4_device_class::FLAG_DISPLAY_CHANGE|
                           i4_device_class::FLAG_DRAG_DROP_EVENTS|
                           i4_device_class::FLAG_IDLE);

  default_cursor=style->cursor_hint->normal_cursor()->copy();
  display->set_mouse_shape(default_cursor);


  i4_const_str::iterator col=i4gets("root_background_color").begin();
  w32 r=col.read_number();
  w32 g=col.read_number();
  w32 b=col.read_number();


  background_color=i4_pal_man.convert_32_to( (r<<16)|(g<<8)|(b),  
                                            &local_image->get_pal()->source);
}

void i4_window_manager_class::cleanup_old_mode()
{
  if (style)  // has a previous mode been used?
  {    
    i4_kernel.unrequest_events(this,
                               i4_device_class::FLAG_MOUSE_BUTTON_UP|
                               i4_device_class::FLAG_MOUSE_BUTTON_DOWN|
                               i4_device_class::FLAG_MOUSE_MOVE|
                               i4_device_class::FLAG_KEY_PRESS|
                               i4_device_class::FLAG_KEY_RELEASE|
                               i4_device_class::FLAG_DISPLAY_CHANGE |
                               i4_device_class::FLAG_DRAG_DROP_EVENTS|
                               i4_device_class::FLAG_IDLE
                               );
    if (default_cursor)
    {
      delete default_cursor;
      default_cursor=0;
      no_cursor_installed=i4_T;      
    }
  }
}


