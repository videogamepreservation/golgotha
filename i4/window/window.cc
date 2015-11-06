/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "window/window.hh"
#include "area/rectlist.hh"
#include "device/kernel.hh"
#include "error/error.hh"
#include "window/win_evt.hh"
#include "image/image.hh"
#include "window/cursor.hh"
#include "time/profile.hh"

i4_profile_class pf_child_draw_prepare("window::child_prepare"),
  pf_parent_draw_prepare("window::parent_prepare");

#include <stdlib.h>

i4_parent_window_class *i4_window_class::root_window()
{
  if (parent)
    return parent->root_window();
  else return 0;
}

i4_parent_window_class *i4_parent_window_class::root_window()
{
  if (parent)
    return parent->root_window();
  else return this;
}

i4_bool i4_window_class::isa_parent(i4_window_class *who)
{
  if (who==0)
    return i4_F;
  else if (who==parent)
    return i4_T;
  else if (parent)
    return parent->isa_parent(who);
  else return i4_F;
}

void i4_window_class::receive_event(i4_event *ev)
{
  if (ev->type()==i4_event::MOUSE_MOVE)
  {
    CAST_PTR(move, i4_mouse_move_event_class, ev);
    mouse_x=move->x;
    mouse_y=move->y;
  }

  if (cursor && parent)  // if we have a cursor we need to load when we get a mouse focus
  {
    if (ev->type()==i4_event::WINDOW_MESSAGE)
    {
      CAST_PTR(wev,i4_window_message_class,ev);
      if (wev->sub_type==i4_window_message_class::GOT_MOUSE_FOCUS)
      {
        CAST_PTR(mev, i4_window_got_mouse_focus_class, ev);

        mouse_x=mev->x;
        mouse_y=mev->y;

        i4_window_change_cursor_class c(this,cursor);
        i4_kernel.send_event(parent,&c); 
      } else if (wev->sub_type==i4_window_message_class::LOST_MOUSE_FOCUS)
      {
        i4_window_change_cursor_class c(this,0);   // unload the cursor
        i4_kernel.send_event(parent,&c); 
      }
    }     
  }
}

void i4_window_class::set_cursor(i4_cursor_class *Cursor)
{ 
  if (cursor)
    delete cursor;
  
  if (Cursor)
    cursor=Cursor->copy();
  else
    cursor=0;

  if (parent)
  {
    i4_window_change_cursor_class c(this,cursor,i4_F);
    i4_kernel.send_event(parent,&c);
  }
}

i4_window_class::~i4_window_class()
{  
  if (cursor)
    delete cursor;
  
  if (parent)
    parent->remove_child(this);
}


void i4_window_class::resize(w16 new_width, w16 new_height)
{
  if (parent)
  {
    i4_window_notify_resize_class resize(this,new_width,new_height);
    i4_kernel.send_event(parent,&resize);
  }
  private_resize(new_width,new_height);
}

void i4_parent_window_class::resize(w16 new_width, w16 new_height)
{
  i4_window_class::resize(new_width, new_height);

  i4_window_notify_resize_class resize(this,new_width,new_height);

  for (win_iter c=children.begin(); c!=children.end(); ++c)   
    send_event_to_child(&*c,&resize);
}


void i4_window_class::private_resize(w16 new_width, w16 new_height)
{
  i4_rect_list_class parent_dirty;
  if (width() && height())
    parent_dirty.add_area(0,0,width()-1, height()-1);

  w=new_width;
  h=new_height;
  parent_dirty.remove_area(0,0,width()-1,height()-1);

  if (width() && height())
    note_undrawn(0,0,width()-1,height()-1);

    
  if (parent)
  {
    for (i4_rect_list_class::area_iter a=parent_dirty.list.begin();
         a!=parent_dirty.list.end(); ++a)
    {
      parent->note_undrawn(a->x1 + x() - parent->x(),
                           a->y1 + y() - parent->y(),
                           a->x2 + x() - parent->x(),
                           a->y2 + y() - parent->y());
                           
    }
  }

  request_redraw();
}


void i4_window_class::reparent(i4_image_class *draw_area, i4_parent_window_class *Parent)
{
  parent=Parent;
  if (parent)
  {
    mouse_x=parent->last_mouse_x()+parent->x()-x();
    mouse_y=parent->last_mouse_y()+parent->y()-y();
  }
  else
  {
    mouse_x=-10000;
    mouse_y=-10000;
  }

  if (local_image)
    local_image=0;

  global_image=draw_area;
  if (global_image)
  {
    local_image=global_image;
    
    if (width() & height())
      note_undrawn(0,0,width()-1,height()-1);

    request_redraw();
  }
}

void i4_window_class::private_move(i4_coord x_offset, i4_coord y_offset)
{
  global_x+=x_offset;
  global_y+=y_offset; 

  if (width() && height())
    note_undrawn(0,0,width()-1,height()-1);


  request_redraw();
}

void i4_window_class::move(i4_coord x_offset, i4_coord y_offset,  i4_bool draw_under)
{
  private_move(x_offset,y_offset);
  if (parent)
  {
    i4_window_notify_move_class move(this,x_offset,y_offset,draw_under);
    i4_kernel.send_event(parent,&move);
  }
}

void i4_window_class::draw(i4_draw_context_class &context)
{
  redraw_flag=i4_F;
}

void i4_window_class::request_redraw(i4_bool for_a_child)
{
  redraw_flag=i4_T; 
    
  if (parent) 
  {
    if (transparent() && width() && height())
    {
      int x1=x()-parent->x(), y1=y()-parent->y();
      int x2=x1+width()-1, y2=y1+height()-1;

      parent->note_undrawn(x1,y1,x2,y2, i4_F);
    }

    parent->request_redraw(i4_T);
  }
}

void i4_window_class::note_undrawn(i4_coord x1, i4_coord y1, i4_coord x2, i4_coord y2,
                                   i4_bool propogate_to_children)
{
  request_redraw(i4_F);
}



void i4_parent_window_class::parent_draw(i4_draw_context_class &context)
{
  redraw_flag=i4_F;
}

// draw should redraw supplied area of self
void i4_parent_window_class::draw(i4_draw_context_class &context)

{  
  if (!undrawn_area.empty() || redraw_flag)
  {   
    pf_parent_draw_prepare.start();

    redraw_flag=i4_F;
    // copy the clip list, which should already be in our coordinates
    i4_rect_list_class child_clip(&context.clip,0,0);  
    
    // remove the area of other children above us    
    win_iter d=children.begin();
    for (;d!=children.end();++d)
    {
      if (!d->transparent())
        child_clip.remove_area(d->x()-x(),
                               d->y()-y(),
                               d->x()+d->width()-1-x(),
                               d->y()+d->height()-1-y());
    }

    child_clip.swap(&context.clip);

    pf_parent_draw_prepare.stop();

    parent_draw(context);

    pf_parent_draw_prepare.start();
    undrawn_area.delete_list();

    child_clip.swap(&context.clip);
    pf_parent_draw_prepare.stop();

  }

  if (child_need_redraw)
  {
    child_need_redraw=i4_F;
    win_iter c=children.begin();
    for (;c!=children.end();++c)   
    {
      if (c->need_redraw())
      {
        pf_child_draw_prepare.start();

	i4_coord x1=c->x()-x(),
          y1=c->y()-y(),
          x2=c->x()+c->width()-1-x(),
          y2=c->y()+c->height()-1-y();

        // copy the clip list, and move it to local coordinates
        i4_rect_list_class child_clip(&context.clip,
                                      -(c->x()-x()),
                                      -(c->y()-y()));

        // intersect the clip with what our area is supposed to be
        child_clip.intersect_area(0,0,
                                  c->width()-1,
                                  c->height()-1);   

        // save the old context xoff and yoff
        sw16 old_xoff=context.xoff,old_yoff=context.yoff;   
          
        // move the context offset to the child's x,y
        context.xoff+=(c->x()-x());                         
        context.yoff+=(c->y()-y());

	  // remove the area of other children above us
        i4_window_class *d=c->next;
        for (;d;d=d->next)
          child_clip.remove_area(d->x()-c->x(), 
                                 d->y()-c->y(),
                                 d->x()+d->width()-1-c->x(),
                                 d->y()+d->height()-1-c->y()); 

        c->call_stack_counter++;  // make sure window doesn't get deleted during it's draw

        // the child is covered completely by other windows
        // tell it this in case it depends on draw()
        if (child_clip.empty())
          c->forget_redraw();   
        else
        {
          child_clip.swap(&context.clip);
          
          child_rerequested_redraw = i4_F;
         
          pf_child_draw_prepare.stop();

          c->draw(context);         // the child is ready to draw

          pf_child_draw_prepare.start();
          if (!child_rerequested_redraw)          
            c->i4_window_class::forget_redraw();

          child_clip.swap(&context.clip);   // restore the old clip list
        }
        
        context.xoff=old_xoff;    // restore the context's x & y offsets
        context.yoff=old_yoff;

        c->call_stack_counter--;
        pf_child_draw_prepare.stop();

      }      
    }
  } 

}
    
#if ( __linux || __sgi)
//extern void db_show();
#else
//void db_show() { ; }
#endif

void i4_window_class::show_context(i4_draw_context_class &context)
{
  i4_draw_context_class nc(0,0,local_image->width(),local_image->height());
  local_image->clear(0xffff,nc);
  i4_rect_list_class::area_iter c=context.clip.list.begin();
  for (; c!=context.clip.list.end(); ++c)
    local_image->rectangle(c->x1,c->y1,c->x2,c->y2,0,nc);

  //  db_show();
}


i4_window_class::i4_window_class(w16 w, w16 h) : w(w),h(h)
{
  mouse_x=-10000;
  mouse_y=-10000;

  global_x=0; 
  global_y=0;
  local_image=0;
  global_image=0;
  parent=0;
  cursor=0;
}


i4_parent_window_class::i4_parent_window_class(w16 w, w16 h) : i4_window_class(w,h)
{
  have_mouse_focus=i4_F;
  mouse_focus_grabbed=i4_F;
  key_focus=children.end();
  mouse_focus=children.end();
  drag_drop_focus=children.end();
}

i4_parent_window_class::win_iter i4_parent_window_class::find_window(i4_coord global_mouse_x, 
                                                                     i4_coord global_mouse_y)
{
  win_iter c=children.begin(),
    find=children.end();

  for (;c!=children.end();++c)
  {
    if (global_mouse_x>=c->x() &&  
        global_mouse_y>=c->y() && 
        global_mouse_x<c->x()+c->width() && 
        global_mouse_y<c->y()+c->height())
      find=c;
  }
  return find;
}

void i4_parent_window_class::drag_drop_move(i4_event *ev)
{
  CAST_PTR(move, i4_window_drag_drop_move_class, ev);
  i4_coord real_mx=move->x+x(),
    real_my=move->y+y();

  win_iter find=find_window(real_mx, real_my);
  if (find != drag_drop_focus)
  {
    if (drag_drop_focus != children.end())
    {
      i4_window_lost_drag_drop_focus_class lost(this);
      send_event_to_child(&*drag_drop_focus, &lost);
    }

    drag_drop_focus = find;
    if (find != children.end())
    {
      i4_window_class *prev_from=move->from_window;
      move->from_window=this;

      send_event_to_child(&*find, move);

      move->from_window=prev_from;
    }
  }

  if (drag_drop_focus != children.end())
    send_event_to_child(&*drag_drop_focus,ev);
}


i4_bool i4_parent_window_class::find_new_mouse_focus()
{
  if (!mouse_focus_grabbed)  // see if we need to change the mouse focus
  {
    win_iter new_mouse_focus=find_window(mouse_x + x(), mouse_y + y());

    if (new_mouse_focus!=mouse_focus)
    {
      change_mouse_focus(&*new_mouse_focus);
      return i4_T;
    }

    else return i4_F;
  }
  else return i4_F;
}

void i4_parent_window_class::mouse_move(i4_event *ev)
{
  CAST_PTR(move, i4_mouse_move_event_class, ev);
  mouse_x=move->x;
  mouse_y=move->y;

  find_new_mouse_focus();

  if (mouse_focus!=children.end())
    send_event_to_child(&*mouse_focus,ev);
}

void i4_parent_window_class::send_event_to_child(i4_window_class *w, i4_event *ev)
{
  sw32 xo=(sw32)x()-(sw32)w->x(), yo=(sw32)y()-(sw32)w->y();

  ev->move(xo, yo);
  i4_kernel.send_event(w, ev);
  ev->move(-xo, -yo);
}

// passes events to mouse_focus or key_focus depending on the event type
void i4_parent_window_class::receive_event(i4_event *ev)
{
  switch (ev->type())
  {
    case i4_event::IDLE_MESSAGE :     // pass idle message to current mouse focus
    {
      if (mouse_focus!=children.end())
        send_event_to_child(&*mouse_focus,ev);
    } break;

    case i4_event::MOUSE_MOVE :
    {
      mouse_move(ev);
    } break;

    case i4_event::MOUSE_BUTTON_DOWN :
    {
      if (mouse_focus!=children.end())
        send_event_to_child(&*mouse_focus,ev);
    } break;

    case i4_event::MOUSE_BUTTON_UP :
    {
      if (mouse_focus!=children.end())
        send_event_to_child(&*mouse_focus,ev);
    } break;

    case i4_event::KEY_PRESS :
    case i4_event::KEY_RELEASE :
    {
      if (key_focus!=children.end())
        send_event_to_child(&*key_focus,ev);
      else if (mouse_focus!=children.end())
        send_event_to_child(&*mouse_focus,ev);
    } break;


    case i4_event::WINDOW_MESSAGE :
    {
      CAST_PTR(mess,i4_window_message_class,ev);
      switch (mess->sub_type)
      {	
        case i4_window_message_class::GOT_DROP :
        {
          if (drag_drop_focus != children.end())
          {
            send_event_to_child(&*drag_drop_focus, ev);
            drag_drop_focus=children.end();
          }
          else
          {
            CAST_PTR(dev, i4_window_got_drop_class, ev);

            i4_parent_window_class::win_iter w=find_window(dev->drag_info.x+x(),
                                                           dev->drag_info.y+y());
            if (w!=children.end())
              send_event_to_child(&*w, ev);
          }
        } break;

        case i4_window_message_class::REQUEST_DRAG_DROP_START :
        case i4_window_message_class::REQUEST_DRAG_DROP_END :
        {
          if (parent)
            parent->receive_event(ev);
        } break;
        case i4_window_message_class::DRAG_DROP_MOVE :
        {
          drag_drop_move(ev);
        } break;

        case i4_window_message_class::CHANGE_CURSOR :
        {
          CAST_PTR(cc,i4_window_change_cursor_class,ev);
          if (parent && (!cc->only_if_active || cc->from()==&*mouse_focus))
          {
            i4_window_change_cursor_class scope(this,cc->cursor,cc->only_if_active);
            parent->receive_event(&scope);
          }

        } break;
	case i4_window_message_class::REQUEST_KEY_GRAB :
	{
          CAST_PTR(grab,i4_window_request_key_grab_class,ev);
          grab->return_result=i4_T;

          if (parent)
          {
            i4_window_request_key_grab_class ask_parent(this);
            i4_kernel.send_event(parent,&ask_parent);
            if (ask_parent.return_result==i4_F)
              grab->return_result=i4_F;
          }

          if (grab->return_result)
          {
            win_iter c=children.begin();
            for (;c!=children.end() && c!=mess->from();++c);
            if (c!=children.end())
            {
              if (c != key_focus)
                change_key_focus(&*c);
            }
            else
              i4_warning("got key grab from unknown child");
          }
	} break;
	case i4_window_message_class::REQUEST_DELETE :
	{
	  remove_child(mess->from());
	  delete mess->from();
	} break;
	
	case i4_window_message_class::REQUEST_NEXT_KEY_FOCUS : next_key_focus(); break;
	case i4_window_message_class::REQUEST_LEFT_KEY_FOCUS : left_key_focus(); break;
	case i4_window_message_class::REQUEST_RIGHT_KEY_FOCUS : right_key_focus(); break;
	case i4_window_message_class::REQUEST_UP_KEY_FOCUS : up_key_focus(); break;
	case i4_window_message_class::REQUEST_DOWN_KEY_FOCUS : down_key_focus(); break;
	case i4_window_message_class::REQUEST_MOUSE_GRAB :
	{
	  CAST_PTR(grab,i4_window_request_mouse_grab_class,ev);

          grab->return_result=i4_T;    // defulat return is false

	  if (!mouse_focus_grabbed)  // see if we need to change the mouse focus	  
	  {
	    if (parent)
	    {
	      i4_window_request_mouse_grab_class ask_parent(this);
	      i4_kernel.send_event(parent,&ask_parent);
	      if (ask_parent.return_result==i4_F)
	        grab->return_result=i4_F;
              else grab->return_result=i4_T;
	    }

	    if (grab->return_result)
            {
              mouse_focus_grabbed=i4_T;
  
              if (&(*mouse_focus) != grab->from())
                change_mouse_focus(grab->from());
            }

	  } else 
	    grab->return_result=i4_F;
	} break;

	case i4_window_message_class::REQUEST_MOUSE_UNGRAB :
	{
	  if (mouse_focus_grabbed)
          {
            mouse_focus_grabbed=i4_F;
            if (parent)
              i4_kernel.send_event(parent,ev);	  

            find_new_mouse_focus();
          }
          else i4_warning("mouse not grabbed");

	} break;


	case i4_window_message_class::NOTIFY_RESIZE :
	{
	  CAST_PTR(resize,i4_window_notify_resize_class,ev);
          i4_window_class *cf=resize->from();

          if (cf!=parent)    // if this is from our parent, ignore it
          {
            i4_rect_list_class dirty;
            if (cf->width() && cf->height())
              dirty.add_area   (cf->x(),cf->y(),cf->x()+cf->width()-1,cf->y()+cf->height()-1);

            if (resize->new_width && resize->new_height)
              dirty.remove_area(cf->x(),cf->y(),
                                cf->x()+resize->new_width-1,
                                cf->y()+resize->new_height-1);

            if (width() && height())
              dirty.intersect_area(x(),y(),x()+width()-1,y()+height()-1);

            if (resize->draw_covered)
            {
              // check to see if any child were under this window 
              // will need to be redraw because of this
              win_iter c=children.begin();
              for (;c!=children.end() && c!=cf;++c)
              {                 
                if (c->width() && c->height() &&
                    !dirty.clipped_away(c->x(),c->y(),c->x()+c->width()-1,c->y()+height()-1))
                {
                  i4_rect_list_class::area_iter a=dirty.list.begin();
                  for (;a!=dirty.list.end();++a)
                  {
                    if (c->width() && c->height())
                    {
                      i4_coord x1=a->x1-c->x(),y1=a->y1-c->y(),x2=a->x2-c->x(),y2=a->y2-c->y();

                      if (x1<0) x1=0;
                      if (y1<0) y1=0;
                      if (x2>=c->width()) x2=c->width()-1;
                      if (y2>=c->height()) y2=c->height()-1;
                      if (x1<=x2 && y1<=y2)
                        c->note_undrawn(x1,y1,x2,y2);
                    }
                  }
                }
              }

              // add this dirty area to the parent's undrawn_area list, 
              // so the parent knows what part of itself it needs to redraw
              i4_rect_list_class::area_iter a=dirty.list.begin();
              for (;a!=dirty.list.end();++a)
                undrawn_area.add_area(a->x1-x(),    // make sure we add in parent-local coordinates
                                      a->y1-y(),
                                      a->x2-x(),
                                      a->y2-y());
            }
          }
          else 
            request_redraw(i4_F);
	} break;

	case i4_window_message_class::NOTIFY_MOVE :
	{
	  CAST_PTR(move_event,i4_window_notify_move_class,ev);
	  i4_window_class *child=move_event->from();
	  i4_coord old_x=child->x()-move_event->x_offset,old_y=child->y()-move_event->y_offset;

	  i4_coord new_x=child->x(),new_y=child->y();

	  if (move_event->draw_covered)
	  {
	    i4_rect_list_class dirty;

            if (child->width() && child->height())
              dirty.add_area(old_x,old_y,old_x+child->width()-1,old_y+child->height()-1);

            if (width() && height())
              dirty.intersect_area(x(),y(),x()+width()-1,y()+height()-1);

            if (child->width() && child->height())
              dirty.remove_area(new_x,new_y,new_x+child->width()-1,new_y+child->height()-1);
	    

      // check to see if any child were under this window will need to be redraw because of this
	    win_iter c=children.begin();
	    for (;c!=children.end() && c!=child;++c)
            {
              if (c->width() && c->height() &&
                  !dirty.clipped_away(c->x(),c->y(),c->x()+c->width()-1,c->y()+height()-1))
              {
                i4_rect_list_class::area_iter a=dirty.list.begin();
                for (;a!=dirty.list.end();++a)
                {
                  if (c->width() && c->height())
                  {
                    i4_coord x1=a->x1-c->x(),y1=a->y1-c->y(),x2=a->x2-c->x(),y2=a->y2-c->y();

                    if (x1<0) x1=0;
                    if (y1<0) y1=0;
                    if (x2>=c->width()) x2=c->width()-1;
                    if (y2>=c->height()) y2=c->height()-1;
                    if (x1<=x2 && y1<=y2)
                      c->note_undrawn(x1,y1,x2,y2);
                  }
                }
              }
            }

	    // add this dirty area to the parent's dirty_area list, 
            // so the parent knows what part of itself it needs to redraw
	    i4_rect_list_class::area_iter a=dirty.list.begin();
	    for (;a!=dirty.list.end();++a)
	      undrawn_area.add_area(a->x1-x(),   // add in local coordinates
                                    a->y1-y(),
                                    a->x2-x(),
                                    a->y2-y());
	  }

	} break;

        case i4_window_message_class::GOT_MOUSE_FOCUS :
        {
          have_mouse_focus=i4_T;

          CAST_PTR(mev, i4_window_got_mouse_focus_class, ev);
          mouse_x=mev->x;
          mouse_y=mev->y;

          i4_window_change_cursor_class c(this,cursor);
          i4_kernel.send_event(parent,&c); 

          find_new_mouse_focus();
        } break;


        case i4_window_message_class::LOST_DROP_FOCUS :
        {
          if (drag_drop_focus!=children.end())
          {
            send_event_to_child(&*drag_drop_focus,ev);
            drag_drop_focus=children.end();
          }
          i4_window_class::receive_event(ev);
        } break;

        case i4_window_message_class::LOST_MOUSE_FOCUS :
        {
          if (mouse_focus!=children.end())
          {
            send_event_to_child(&*mouse_focus,ev);
            mouse_focus=children.end();
          }
          i4_window_class::receive_event(ev);
          have_mouse_focus=i4_F;

        } break;
        case i4_window_message_class::LOST_KEYBOARD_FOCUS :
        {
          change_key_focus(0);
        } break;
        default :
          i4_window_class::receive_event(ev);
          
      }
    } break;
    default :
      i4_window_class::receive_event(ev);
          
  }
}

void i4_parent_window_class::change_key_focus(i4_window_class *new_focus)
{
  if (key_focus!=children.end())
  {
    i4_window_message_class lost(i4_window_message_class::LOST_KEYBOARD_FOCUS,this);
    send_event_to_child(&*key_focus,&lost);    
  }

  key_focus=new_focus;

  if (new_focus)
  {
    i4_window_message_class got(i4_window_message_class::GOT_KEYBOARD_FOCUS,this);
    send_event_to_child(&*key_focus,&got);  
  }
}


void i4_parent_window_class::change_mouse_focus(i4_window_class *new_focus)
{
  if (mouse_focus!=children.end())
  {
    i4_window_lost_mouse_focus_class lost(this, new_focus);
    send_event_to_child(&*mouse_focus,&lost);    
  }


  mouse_focus=new_focus;

  if (mouse_focus!=children.end())
  {
    i4_window_got_mouse_focus_class got(this, mouse_x, mouse_y);
    send_event_to_child(&*mouse_focus,&got);  
  }
}


void i4_parent_window_class::next_key_focus()
{
  if (key_focus!=children.end())
    change_key_focus(key_focus->next);
}


void i4_parent_window_class::left_key_focus()
{
  if (key_focus!=children.end())
  {
    win_iter c=children.begin(),closest=children.end();
    w32 closest_distance=0xffffffff;
    i4_coord cx1=key_focus->x()+key_focus->width()/2,
      cy1=key_focus->y()+key_focus->height()/2;


    for (;c!=children.end();++c)
    {
      if (c->x()<key_focus->x())
      {
	i4_coord cx2=c->x()+c->width()/2,cy2=c->y()+c->height()/2;

	w32 dist=(cx2-cx1)*(cx2-cx1)+(cy2-cy1)*(cy2-cy1);
	if (dist<closest_distance)
	{
	  closest_distance=dist;
	  closest=c;
	}
      }
    }

    if (closest!=children.end())
      change_key_focus(&*closest);
  }
}


void i4_parent_window_class::right_key_focus()
{
  if (key_focus!=children.end())
  {
    win_iter c=children.begin(),closest=children.end();
    w32 closest_distance=0xffffffff;
    i4_coord cx1=key_focus->x()+key_focus->width()/2,
      cy1=key_focus->y()+key_focus->height()/2;


    for (;c!=children.end();++c)
    {
      if (c->x()>cx1)
      {
	i4_coord cx2=c->x()+c->width()/2,
          cy2=c->y()+c->height()/2;       
	w32 dist=(cx2-cx1)*(cx2-cx1)+(cy2-cy1)*(cy2-cy1);
	if (dist<closest_distance)
	{
	  closest_distance=dist;
	  closest=c;
	}
      }
    }

    if (closest!=children.end())
      change_key_focus(&*closest);
  }
}


void i4_parent_window_class::up_key_focus()
{
  if (key_focus!=children.end())
  {
    win_iter c=children.begin(),closest=children.end();
    w32 closest_distance=0xffffffff;
    i4_coord cx1=key_focus->x()+key_focus->width()/2,
      cy1=key_focus->y()+key_focus->height()/2;


    for (;c!=children.end();++c)
    {
      if (c->y()<key_focus->y())
      {
	i4_coord cx2=c->x()+c->width()/2,
          cy2=c->y()+c->height()/2;       
	w32 dist=(cx2-cx1)*(cx2-cx1)+(cy2-cy1)*(cy2-cy1);
	if (dist<closest_distance)
	{
	  closest_distance=dist;
	  closest=c;
	}
      }
    }

    if (closest!=children.end())
      change_key_focus(&*closest);
  }
}


void i4_parent_window_class::down_key_focus()
{
  if (key_focus!=children.end())
  {
    win_iter c=children.begin(),closest=children.end();
    w32 closest_distance=0xffffffff;
    i4_coord cx1=key_focus->x()+key_focus->width()/2,
      cy1=key_focus->y()+key_focus->height()/2;

    for (;c!=children.end();++c)
    {
      if (c->y()>key_focus->y())
      {
	i4_coord cx2=c->x()+c->width()/2,
          cy2=c->y()+c->height()/2;       
	w32 dist=(cx2-cx1)*(cx2-cx1)+(cy2-cy1)*(cy2-cy1);
	if (dist<closest_distance)
	{
	  closest_distance=dist;
	  closest=c;
	}
      }
    }

    if (closest!=children.end())
      change_key_focus(&*closest);
  }
}

void i4_parent_window_class::reparent(i4_image_class *draw_area, i4_parent_window_class *_parent)
{
  // reparent children first, in case the have a mouse grab and need to tell our
  // original parent to ungrab
  for (win_iter c=children.begin();c!=children.end();++c)
    c->reparent(draw_area,this);

  i4_window_class::reparent(draw_area,_parent);

  if (have_mouse_focus)
    find_new_mouse_focus();
}

void i4_parent_window_class::add_child(i4_coord x_, i4_coord y_, i4_window_class *child)
{
  children.insert_end(*child);
  child->reparent(global_image,this);
  child->private_move(x()+x_-child->x(),y()+y_-child->y());

  if (have_mouse_focus)
    find_new_mouse_focus();
}

void i4_parent_window_class::add_child_front(i4_coord x_, i4_coord y_, i4_window_class *child)
{
  children.insert(*child);
  child->reparent(global_image,this);
  child->private_move(x()+x_-child->x(),y()+y_-child->y());

  if (have_mouse_focus)
    find_new_mouse_focus();
}


void i4_parent_window_class::forget_redraw()
{
  undrawn_area.delete_list();  
  for (win_iter c=children.begin();c!=children.end();++c)
    c->forget_redraw();
}

void i4_parent_window_class::transfer_children(i4_parent_window_class *other_parent, 
                                               i4_coord x_offset, i4_coord y_offset)
{ 
  while (children.begin() != children.end())
  {
    i4_window_class *c=&*children.begin();
    i4_coord xn=c->x()-x();
    i4_coord yn=c->y()-y();

    remove_child(c);
    other_parent->add_child(xn+x_offset,
                            yn+y_offset,c);
  }

  if (have_mouse_focus)
    find_new_mouse_focus();

  if (other_parent->has_mouse_focus())
    other_parent->find_new_mouse_focus();
}

void i4_parent_window_class::remove_child(i4_window_class *child)
{
  if (&*key_focus==child)
    change_key_focus(0); 

  if (&*mouse_focus==child)
  {
    if (mouse_focus_grabbed)
    {
      if (parent)
      {
        i4_window_request_mouse_ungrab_class ungrab(this);
        i4_kernel.send_event(parent, &ungrab);
      }

      mouse_focus_grabbed=i4_F;
    }
  }

  if (child==&*children.begin())
    children.erase();
  else
  {
    win_iter f=children.begin(),last=children.end();
    for (;f!=children.end() && f!=&*child;)
    {
      last=f;
      ++f;
    }
    if (f!=children.end())
      children.erase_after(last);
    else
      i4_error("child not found");
  }

  if (child->width() && child->height())
  {
    note_undrawn(child->x()-x(),
                 child->y()-y(),
                 child->x()+child->width()-1-x(),
                 child->y()+child->height()-1-y());
  }

  if (&*mouse_focus==child)
  {
    i4_window_lost_mouse_focus_class lost(this, 0);
    send_event_to_child(child, &lost);    
    mouse_focus=children.end();
  }


  child->reparent(0,0);  // tell the child it doesn't have a parent or a draw_area anymore

  if (have_mouse_focus)
    find_new_mouse_focus();

}

void i4_parent_window_class::request_redraw(i4_bool for_a_child)
{
  if (for_a_child)
  {
    child_need_redraw=i4_T;
    child_rerequested_redraw=i4_T;
  }
  else
    redraw_flag=i4_T;
  if (parent)
    parent->request_redraw(i4_T);
}

void i4_parent_window_class::note_undrawn(i4_coord x1, i4_coord y1, 
                                          i4_coord x2, i4_coord y2,
                                          i4_bool propogate_to_children)
{

  undrawn_area.add_area(x1,y1,x2,y2);
  request_redraw();
   
  win_iter c=children.begin();


  x1+=x();  y1+=y();     // transform to global coordinates
  x2+=x();  y2+=y();
  
  if (propogate_to_children)
  {
    for (;c!=children.end();++c)
    {
      if (c->width() && c->height())
      {
        if (!(c->x()>x2 || 
              c->y()>y2 || 
              c->x()+c->width()-1  < x1 || 
              c->y()+c->height()-1 < y1))
        {      
          i4_coord ax1=x1-c->x(),
            ay1=y1-c->y(),
            ax2=x2-c->x(),
            ay2=y2-c->y();
          if (ax1<0) ax1=0;
          if (ay1<0) ay1=0;
          if (ax2>=c->width()) ax2=c->width()-1;
          if (ay2>=c->height()) ay2=c->height()-1;
          if (ax1<=ax2 && ay1<=ay2)
            c->note_undrawn(ax1,ay1,ax2,ay2);
        }
      }
    }
  }
}



void i4_parent_window_class::private_move(i4_coord x_offset, i4_coord y_offset)
{
  i4_window_class::private_move(x_offset,y_offset);
  win_iter c=children.begin();
  for (;c!=children.end();++c)
  {
    c->private_move(x_offset,y_offset);  
  }
  // use private move so child doesn't tell us it moved (we know!)
}


void i4_parent_window_class::redraw_area(i4_coord x1, i4_coord y1, i4_coord x2, i4_coord y2)
{
  if (!(x2<0 || y2<0 || x1>=width() || y1>=height()))
    undrawn_area.add_area(x1,y1,x2,y2);
}

// arranges child windows from left to right then down
void i4_parent_window_class::arrange_right_down()  
{
  i4_coord x1=x(),y1=y(),mh=0;
  win_iter c=children.begin();
  for (;c!=children.end();++c)
  {

    if (c->width()+x1>x()+width())
    {
      y1+=mh;
      x1=x();
      mh=0;
    }

    c->move(x1-c->x(),y1-c->y());

    if (c->height()>mh)
      mh=c->height();

    x1+=c->width();


  }
}

void i4_parent_window_class::resize_to_fit_children()
{
  i4_coord x2=0,y2=0;
  win_iter c=children.begin();
  for (;c!=children.end();++c)
  {
    if (c->x()-x()+c->width()>x2)    
      x2=c->x()-x()+c->width();
    if (c->y()-y()+c->height()>y2)
      y2=c->y()-y()+c->height();
  }

  resize(x2,y2);
}

void i4_parent_window_class::arrange_down_right()                                 // arranges child windows from top to bottom then right
{
  i4_coord x1=x(),y1=y(),mw=0;
  win_iter c=children.begin();
  for (;c!=children.end();++c)
  {
    if (c->height()+y1>=height())
    {
      x1+=mw;
      y1=y();
      mw=0;
    }

    c->move(x1-c->x(),y1-c->y());
    y1+=c->height();

    if (c->width()>mw)
      mw=c->width();


  }
}


i4_parent_window_class::~i4_parent_window_class()
{
  while (children.begin() != children.end())
  {
    win_iter c=children.begin();
    children.erase();
    c->reparent(0,0);
    
    i4_kernel.delete_handler(&*c);
  }  
}

i4_window_class *i4_parent_window_class::get_nth_window(w32 win_num)
{
  win_iter i=children.begin();
  while (win_num && i!=children.end())
  {
    ++i;
    win_num--;
  }
  if (i==children.end())
    return 0;
  else return &*i;
}


#ifndef I4_RETAIL
void i4_window_class::debug_show()
{
  i4_warning("%s",name());
  if (parent)
    parent->debug_show();
}
#endif

i4_bool i4_parent_window_class::isa_child(i4_window_class *w)
{
  win_iter c=children.begin();
  for (;c!=children.end();++c)
    if (&*c==w)
      return i4_T;

  return i4_F;
}
