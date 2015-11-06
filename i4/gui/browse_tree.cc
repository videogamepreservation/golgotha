/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifdef _MANGLE_INC
#include "gui/BROWS~B4.HH"
#else
#include "gui/browse_tree.hh"
#endif
#include "window/style.hh"
#include "window/colorwin.hh"
#include "window/win_evt.hh"

void i4_vertical_compact_window_class::compact()
{
  sw32 my=0,largest_width=0;
  i4_parent_window_class::win_iter w;
  for (w=children.begin();
       w!=children.end(); ++w)
    if (w->width()>largest_width)
      largest_width=w->width();

  for (w=children.begin();
       w!=children.end(); ++w)
  {
    if (center)
      w->private_move(x() + largest_width/2-(sw32)w->width()/2 - w->x(), y() + my - w->y());
    else
      w->private_move(0, y() + my - w->y());

    my+=w->height();
  }
  if (width()!=largest_width || my!=height())
    resize(largest_width, my);
}


void i4_horizontal_compact_window_class::compact()
{
  sw32 mx=0,largest_height=0;
  i4_parent_window_class::win_iter w;
  for (w=children.begin();
       w!=children.end(); ++w)
    if (w->height()>largest_height)
      largest_height=w->height();

  for (w=children.begin();
       w!=children.end(); ++w)
  {
    if (center)
      w->private_move(x() + mx - w->x(), y() + largest_height/2-(sw32)w->height()/2 - w->y());
    else
      w->private_move(x() + mx - w->x(), 0);

    mx+=w->width();
  }
  if (mx!=width() || largest_height!=height())
    resize(mx, largest_height);
}

void i4_vertical_compact_window_class::receive_event(i4_event *ev)
{
  if (ev->type()==i4_event::WINDOW_MESSAGE)
  {
    CAST_PTR(mess,i4_window_message_class,ev);

    if (mess->sub_type==i4_window_message_class::NOTIFY_RESIZE)
    {
      CAST_PTR(res,i4_window_notify_resize_class,ev);
      sw32 ow=res->from()->width(), oh=res->from()->height();
      res->from()->private_resize(res->new_width, res->new_height);

      compact();
         
      res->from()->private_resize(ow, oh);

      note_undrawn(0,0, width()-1, height()-1);

    } else i4_parent_window_class::receive_event(ev);
  } else i4_parent_window_class::receive_event(ev);
}

void i4_vertical_compact_window_class::parent_draw(i4_draw_context_class &context)
{
  i4_rect_list_class child_clip(&context.clip,0,0);
  child_clip.intersect_list(&undrawn_area);

  child_clip.swap(&context.clip);

  local_image->clear(color,context);

  child_clip.swap(&context.clip);

}


void i4_horizontal_compact_window_class::receive_event(i4_event *ev)
{
  if (ev->type()==i4_event::WINDOW_MESSAGE)
  {
    CAST_PTR(mess,i4_window_message_class,ev);

    if (mess->sub_type==i4_window_message_class::NOTIFY_RESIZE)
    {
      CAST_PTR(res,i4_window_notify_resize_class,ev);
      sw32 ow=res->from()->width(), oh=res->from()->height();
      res->from()->private_resize(res->new_width, res->new_height);
      compact();         
      res->from()->private_resize(ow, oh);
      note_undrawn(0,0, width()-1, height()-1);
    } else i4_parent_window_class::receive_event(ev);
  } else i4_parent_window_class::receive_event(ev);
}

void i4_horizontal_compact_window_class::parent_draw(i4_draw_context_class &context)
{
  i4_rect_list_class child_clip(&context.clip,0,0);
  child_clip.intersect_list(&undrawn_area);
  child_clip.swap(&context.clip);
  local_image->clear(color,context);
  child_clip.swap(&context.clip);
}


class i4_browse_toggle_class : public i4_window_class
{
public:
  char *name() { return "browse_toggle"; }

  enum { EXPANDED, COMPACTED, NO_CHILDREN } state;

  i4_browse_window_class *browse_parent;
  i4_graphical_style_class *style;

  i4_browse_toggle_class(i4_graphical_style_class *style,
                         i4_browse_window_class *browse_parent,
                         i4_bool show_plus_minus,
                         i4_bool expanded,
                         i4_bool has_a_child)

    : i4_window_class(style->icon_hint->plus_icon->width()+4,
                      style->icon_hint->plus_icon->height()),
      browse_parent(browse_parent),
      style(style)
  {
    if (show_plus_minus)
    {
      if (expanded)
        state=EXPANDED;
      else
        state=COMPACTED;
    }
    else
      state=NO_CHILDREN;
  }

  virtual void draw(i4_draw_context_class &context)
  {
    local_image->clear(style->color_hint->window.passive.medium, context);
    if (state!=NO_CHILDREN)
    {
      if (state==EXPANDED)
        style->icon_hint->plus_icon->put_image_trans(local_image, 0,0, 0, context);
      else
        style->icon_hint->minus_icon->put_image_trans(local_image, 0,0, 0, context);
    }
  }

  virtual void receive_event(i4_event *ev)
  {
    if (ev->type()==i4_event::MOUSE_BUTTON_DOWN && state!=NO_CHILDREN)
    {
      if (state==EXPANDED)
      {
        state=COMPACTED;
        browse_parent->compress();
      }
      else
      {
        state=EXPANDED;
        browse_parent->expand();

      }
      request_redraw();
    }
  }
} ;


void i4_browse_window_class::add_arranged_child(i4_window_class *child)
{
  title_area->add_child(0,0,child);
  ((i4_horizontal_compact_window_class *)title_area)->compact();


  private_resize(0,0);
  arrange_right_down();
  if (child_object)
    child_object->move(x_start(), 0);
  resize_to_fit_children();
}

void i4_browse_window_class::compress()
{
  if (child_object && expanded)
  {
    expanded=i4_F;
    i4_parent_window_class::remove_child(child_object);
    resize_to_fit_children();
  }
}

void i4_browse_window_class::expand()
{
  if (child_object && !expanded)
  {
    expanded=i4_T;
    i4_parent_window_class::add_child(0,0,child_object);
    private_resize(0,0);
    arrange_right_down();
    if (child_object)
      child_object->move(x_start(), 0);
    resize_to_fit_children();  
  }
}

void i4_browse_window_class::replace_object(i4_window_class *object)
{
  if (child_object)
  {
    if (expanded)
      remove_child(child_object);    

    delete child_object;
    child_object=0;

    /*
    if (((i4_browse_toggle_class *)toggle_button)->state == i4_browse_toggle_class::EXPANDED)
      ((i4_browse_toggle_class *)toggle_button)->state=i4_browse_toggle_class::COMPACTED;
    else if (((i4_browse_toggle_class *)toggle_button)->state == i4_browse_toggle_class::COMPACTED)
      ((i4_browse_toggle_class *)toggle_button)->state=i4_browse_toggle_class::EXPANDED;
      */

    toggle_button->request_redraw();

    private_resize(0,0);
    resize_to_fit_children();
  }

  if (object)
  {
    child_object=object;
    if (expanded)   
      i4_parent_window_class::add_child(0,0,object);

    private_resize(0,0);
    arrange_right_down();
    object->move(x_start(), 0);

    resize_to_fit_children();
  }
}

class i4_browse_title_container_class : public i4_parent_window_class
{
public:
  i4_browse_title_container_class() : i4_parent_window_class(0,0) {}
  char *name() { return "browse title container"; }
};

i4_browse_window_class::i4_browse_window_class(i4_graphical_style_class *style,
                                               i4_window_class *title_object,
                                               i4_window_class *child_obj,
                                               i4_bool show_plus_minus,
                                               i4_bool expanded)
  : style(style),
    expanded(expanded),
    i4_parent_window_class(0,0)
{
  toggle_button=0;
  child_object=0;

  title_area=new i4_horizontal_compact_window_class(style->color_hint->window.passive.medium,
                                                    i4_T);
  ((i4_horizontal_compact_window_class *)title_area)->compact();
  i4_parent_window_class::add_child(0,0,title_area);


  toggle_button=new i4_browse_toggle_class(style, this, 
                                           show_plus_minus,
                                           expanded, 
                                           (i4_bool)(child_obj!=0));
  title_area->add_child(0,0,toggle_button);

  if (title_object)
    add_arranged_child(title_object);

  if (child_obj)
    replace_object(child_obj);

}


sw32 i4_browse_window_class::x_start()
{
  return style->icon_hint->plus_icon->width() + 10;
}



void i4_browse_window_class::receive_event(i4_event *ev)
{
  if (ev->type()==i4_event::WINDOW_MESSAGE)
  {
    CAST_PTR(mess,i4_window_message_class,ev);

    if (mess->sub_type==i4_window_message_class::NOTIFY_RESIZE)
    {
      CAST_PTR(resize,i4_window_notify_resize_class,ev);

      note_undrawn(0,0, width()-1, height()-1);

      w16 ow=resize->from()->width(),
        oh=resize->from()->height();

      resize->from()->private_resize(resize->new_width,resize->new_height);

      private_resize(0,0);
      arrange_right_down();
      if (child_object)
        child_object->move(x_start(), 0);
      resize_to_fit_children();

      note_undrawn(0,0, width()-1, height()-1);

      resize->from()->private_resize(ow, oh);
    }
    else
      i4_parent_window_class::receive_event(ev);
  }
  else i4_parent_window_class::receive_event(ev);
}



void i4_browse_window_class::parent_draw(i4_draw_context_class &context)
{
  i4_rect_list_class child_clip(&context.clip,0,0);
  child_clip.intersect_list(&undrawn_area);

  child_clip.swap(&context.clip);

  local_image->clear(style->color_hint->window.passive.medium,context);

  child_clip.swap(&context.clip);
}
