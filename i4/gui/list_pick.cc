/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/


#ifdef _MANGLE_INC
#include "LIST_~LE.HH"
#else
#include "list_pick.hh"
#endif
#include "menu/menuitem.hh"
#include "image/image.hh"
#ifdef _MANGLE_INC
#include "gui/SCROL~OK.HH"
#else
#include "gui/scroll_bar.hh"
#endif

i4_list_pick::i4_list_pick(w16 width, w16 height,
                           w32 total_items,
                           i4_menu_item_class **items,
                           w32 scroll_event_id,
                           i4_color background,
                           i4_bool free_items)

  : i4_parent_window_class(width, height),
    total_items(total_items),
    items(items),
    background(background),
    scroll_event_id(scroll_event_id),
    free_items(free_items)
{
  need_draw_all=i4_T;
  start=0;
  end=-1;
  for (w32 i=0; i<total_items; i++)
  {
    if (items[i]->width()>length)
      length=items[i]->width();
  }
  length+=8;
  reposition_start(0);
}




i4_list_pick::~i4_list_pick()
{
  if (free_items)
  {
    for (w32 i=0; i<total_items; i++)
      delete items[i];
    i4_free(items);
  }

}

void i4_list_pick::reposition_start(w32 new_start)
{
  sw32 old_end=end,old_start=start,i;
  i4_coord x1=x(),y1=y(),x2=x()+width(),y2=y()+height();

  if (new_start!=start)
    need_draw_all=i4_T;

  i4_window_class *c;

  for (i=new_start; x1<x2 && i<total_items;)
  {
    c=items[i];


    if (c->height()+y1>=y2)
    {
      x1+=length;
      y1=y();
    }

    if (i>=end || i<start)
    {
      //      i4_warning("add    %d",i);
      add_child(x1,y1,c);
    }
    else
    {
      //      i4_warning("move   %d",i);
      c->move(x1-c->x(),y1-c->y());
    }

    y1+=c->height();
    i++;
  }

  start=new_start;
  end=i;

  if (old_start<start && old_end>=start)
  {
    for (i=old_start; i<start; i++)
    {
      //      i4_warning("remove %d",i);
      remove_child(items[i]);
    }
  }

  if (old_start<=end && old_end>end)
  {
    for (i=end; i<old_end; i++)
    {
      //      i4_warning("remove %d",i);
      remove_child(items[i]);
    }
  }


  if (old_end<start || old_start>end)
  {
    for (i=old_start; i<old_end; i++)
    {
      //      i4_warning("remove %d",i);
      remove_child(items[i]);
    }
  }
}
  
void i4_list_pick::parent_draw(i4_draw_context_class &context)
{
  if (need_draw_all)
  {
    local_image->add_dirty(0,0, width()-1, height()-1, context);
    local_image->clear(background,context);    
    need_draw_all=i4_F;
  }
  else
  {
    i4_rect_list_class child_clip(&context.clip,0,0);
    child_clip.intersect_list(&undrawn_area);

    child_clip.swap(&context.clip);

    local_image->clear(background,context);

    child_clip.swap(&context.clip);
  }
}


void i4_list_pick::receive_event(i4_event *ev)
{
  if (ev->type()==i4_event::USER_MESSAGE)
  {
    CAST_PTR(uev, i4_user_message_event_class, ev);
    if (uev->sub_type==scroll_event_id)
    {
      CAST_PTR(sc_msg, i4_scroll_message, ev);
      reposition_start(sc_msg->amount*total_items/sc_msg->scroll_total);
    } 
    else i4_parent_window_class::receive_event(ev);    
  } 
  else i4_parent_window_class::receive_event(ev);
}



// draw should redraw supplied area of self
void i4_list_pick::draw(i4_draw_context_class &context)
{  
  if (!undrawn_area.empty() || redraw_flag)
  {   
    redraw_flag=i4_F;
    parent_draw(context);
    undrawn_area.delete_list();
  }


  if (child_need_redraw)
  {
    child_need_redraw=i4_F;
    win_iter c=children.begin();
    for (;c!=children.end();++c)   
    {
      if (c->need_redraw())
      {          
        // copy the clip list, and move it to local coordinates
        i4_rect_list_class child_clip(&context.clip,
                                      -(c->x()-x()),
                                      -(c->y()-y()));         

        // intersect the clip with what our area is supposed to be
        child_clip.intersect_area(0,0,
                                  c->width()-1,
                                  c->height()-1);    

        sw16 old_xoff=context.xoff,old_yoff=context.yoff;   // save the old context xoff and yoff
        context.xoff+=(c->x()-x());                // move the context offset to the child's x,y
        context.yoff+=(c->y()-y());

        child_clip.swap(&context.clip);

        c->draw(context);         // the child is ready to draw                    

        context.xoff=old_xoff;    // restore the context's x & y offsets
        context.yoff=old_yoff;


        child_clip.swap(&context.clip);   // restore the old clip list
      }      
    }
  } 
}
