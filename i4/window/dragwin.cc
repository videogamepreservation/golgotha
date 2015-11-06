/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "arch.hh"
#include "window/dragwin.hh"
#include "device/kernel.hh"
#include "error/error.hh"
#include "window/win_evt.hh"
#include "area/rectlist.hh"
#include "image/image.hh"

class i4_drag_frame_class : public i4_window_class
{
  i4_coord ghost_x,ghost_y;

  public :
  i4_bool need_remove_ghost,
    draw_self;

  char *name() { return "drag_frame"; }

  i4_drag_frame_class(w16 w, w16 h) : 
    i4_window_class(w,h)
  {
    need_remove_ghost=i4_F;
    draw_self=i4_T;
  }

  virtual void draw(i4_draw_context_class &context)
  {
    // this draw is special in that it should not be subject to clipping and
    // window coordinate transformations

    i4_rect_list_class no_clip(0,0,local_image->width()-1,local_image->height()-1);
    context.clip.swap(&no_clip);
    i4_coord old_xoff=context.xoff,old_yoff=context.yoff;
    context.xoff=0;
    context.yoff=0;

    if (need_remove_ghost)
    {
      i4_coord x1=ghost_x,y1=ghost_y;
      i4_coord x2=x1+width()-1,y2=y1+height()-1;

      i4_color c=0xffffffff;

      local_image->xor_bar(x1,y1,x2,y1,c,context);
      local_image->xor_bar(x2,y1,x2,y2,c,context);
      local_image->xor_bar(x1,y1,x1,y2,c,context);
      local_image->xor_bar(x1,y2,x2,y2,c,context);      
    }

    if (draw_self)
    {
      i4_coord x1=x(),y1=y();
      i4_coord x2=x1+width()-1,y2=y1+height()-1;
      i4_color c=0xffffffff;

      local_image->xor_bar(x1,y1,x2,y1,c,context);
      local_image->xor_bar(x2,y1,x2,y2,c,context);
      local_image->xor_bar(x1,y1,x1,y2,c,context);
      local_image->xor_bar(x1,y2,x2,y2,c,context);
      ghost_x=x();
      ghost_y=y();
    }
    draw_self=i4_T;
    need_remove_ghost=i4_T;

    context.xoff=old_xoff;
    context.yoff=old_yoff;
    context.clip.swap(&no_clip);
  }

  virtual void show_self(w32 indent) 
  {    
    char fmt[50];
    sprintf(fmt,"%%%ds drag_window",indent);
    i4_warning(fmt," ");
  }

} ;


i4_draggable_window_class::i4_draggable_window_class(w16 w, w16 h) : i4_parent_window_class(w,h)
{
  drag_frame=0;
  last_mouse_x=0;
  last_mouse_y=0;
}

void i4_draggable_window_class::receive_event(i4_event *ev)
{
  if (ev->type()==i4_event::WINDOW_MESSAGE)
  {
    CAST_PTR(message,i4_window_request_drag_start_class,ev);
    // must have a parent in order to move/be dragged
    if (message->sub_type==i4_window_message_class::REQUEST_DRAG_START && parent)  
    {
      // see if we can grab the mouse     
      i4_window_request_mouse_grab_class grab(message->from());

      // ask ourself, we will check with our parent..
      i4_kernel.send_event(this,&grab);

      if (grab.return_result)
      {
        drag_frame=new i4_drag_frame_class(width(),height());
        parent->add_child(x(),y(),drag_frame);
        
        message->return_result=i4_T;      
      }
      else 
      {
        message->return_result=i4_F;
        i4_warning("draggable window:: unable to grab mouse");
      } 
    }
    // must have a parent in order to move/be dragged
    else if (message->sub_type==i4_window_message_class::REQUEST_DRAG_END && parent)  
    {
      if (!drag_frame)
        i4_error("got drag end message and not dragging");

      i4_window_request_mouse_ungrab_class ungrab(this);
      i4_kernel.send_event(this,&ungrab);

      // move ourself to the final position of the drag frame
      move(drag_frame->x()-x(),drag_frame->y()-y());
      request_redraw();

      // tell ourselves to delete the dragger next tick, 
      // so it gets a chance to clear itself off the screen
      i4_kernel.delete_handler(drag_frame);

      drag_frame->need_remove_ghost=i4_T;
      drag_frame->draw_self=i4_F;
      drag_frame=0;   // assume it is deleted now
    }
    else i4_parent_window_class::receive_event(ev);
  } else if (ev->type()==i4_event::MOUSE_MOVE)
  { 
    CAST_PTR(move,i4_mouse_move_event_class,ev);
    current_mouse_x=move->x+x();
    current_mouse_y=move->y+y();
    if (drag_frame &&
	((current_mouse_x!=last_mouse_x ||
          current_mouse_y!=last_mouse_y)))
    {
      if (current_mouse_x<0)
        current_mouse_x=0;
      if (current_mouse_y<0)
        current_mouse_y=0;
      i4_coord xo=current_mouse_x-last_mouse_x,yo=current_mouse_y-last_mouse_y;
      drag_frame->move(xo,yo,i4_F);
      drag_frame->request_redraw();
    }

    last_mouse_x=current_mouse_x;
    last_mouse_y=current_mouse_y;
    i4_parent_window_class::receive_event(ev);
  } else i4_parent_window_class::receive_event(ev);
}






