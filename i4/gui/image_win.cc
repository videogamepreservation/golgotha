/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "gui/image_win.hh"

#include "window/window.hh"
#include "window/win_evt.hh"

#include "image/image.hh"


void i4_image_window_class::reparent(i4_image_class *draw_area, i4_parent_window_class *parent)
{
  if (draw_area)
  {
    if (darken_im)
      delete darken_im;

    if (darken && im)
    {
      int x,y, w=im->width(), h=im->height();
      
      i4_image_class *d=i4_create_image(w, h, draw_area->get_pal());

      for (y=0; y<h; y++)
        for (x=0; x<w; x++)
        {
          i4_color c=im->get_pixel(x,y);
          int r= (((c>>16)&0xff)*3/4)<<16;
          int g= (((c>>8)&0xff)*3/4)<<8;
          int b= (((c>>0)&0xff)*3/4)<<0;

          d->put_pixel(x,y, r|g|b);
        }

      darken_im=d;

      
    }
    else darken_im=0;
  }
  i4_parent_window_class::reparent(draw_area, parent);
}


void i4_image_window_class::change_image(i4_image_class *new_im)
{
  if (del && im)
    delete im;

  im = new_im;
  reparent(local_image, parent);
}


i4_image_window_class::i4_image_window_class(i4_image_class *im,
                                             i4_bool delete_on_destructor,
                                             i4_bool dark_when_not_active)
  : i4_parent_window_class(im->width(),im->height()),
    im(im),
    del(delete_on_destructor),
    darken(dark_when_not_active)
{
  active=i4_F;
  darken_im=0;
}

void i4_image_window_class::parent_draw(i4_draw_context_class &context)
{
  if (darken && !active && darken_im)
    darken_im->put_image(local_image, 0,0, context);
  else
  {
    i4_rect_list_class child_clip(&context.clip,0,0);
    child_clip.intersect_list(&undrawn_area);

    child_clip.swap(&context.clip);

  
    for (i4_rect_list_class::area_iter c=context.clip.list.begin();c!=context.clip.list.end();++c)
      im->put_part(local_image, c->x1, c->y1, c->x1, c->y1, c->x2, c->y2, context);

    
    child_clip.swap(&context.clip);
    i4_parent_window_class::parent_draw(context);

  }

}

void i4_image_window_class::receive_event(i4_event *ev)
{
  CAST_PTR(wev, i4_window_message_class, ev);
  if (ev->type()==i4_event::WINDOW_MESSAGE && 
      wev->sub_type==i4_window_message_class::GOT_MOUSE_FOCUS)
  {
    active=i4_T;
    if (darken && darken_im)
      request_redraw(i4_F);
  }
  else if (ev->type()==i4_event::WINDOW_MESSAGE && 
           wev->sub_type==i4_window_message_class::LOST_MOUSE_FOCUS)
  {
    active=i4_F;
    if (darken && darken_im)
      request_redraw(i4_F);

  }
  
  i4_parent_window_class::receive_event(ev);
}

i4_image_window_class::~i4_image_window_class()
{
  if (del && im)
    delete im;
  if (darken_im)
    delete darken_im;
}

