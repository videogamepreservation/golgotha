/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "window/style.hh"
#include "window/window.hh"
#include "device/kernel.hh"
#include "window/dragwin.hh"
#include "window/colorwin.hh"
#include "image/image.hh"
#include "menu/boxmenu.hh"
#include "window/win_evt.hh"
#include "gui/text.hh"
#include "gui/deco_win.hh"
#include "app/app.hh"
#include "gui/gradiant.hh"

class i4_mwm_context_help : public i4_window_class
{
  i4_graphical_style_class *style;
  i4_str *text;
public:
  i4_event_handler_reference_class<i4_window_class> shadow;

  void draw(i4_draw_context_class &context)
  {
    i4_font_class *f=style->font_hint->small_font;
    local_image->clear(style->color_hint->text_background, context);
    local_image->rectangle(0,0, width()-1, height()-1, 0, context);
    
    f->set_color(style->color_hint->text_foreground);
    f->put_string(local_image, 2,2, *text, context);
  }

  i4_mwm_context_help(i4_graphical_style_class *style, const i4_const_str text)
    :  style(style),
       i4_window_class(style->font_hint->small_font->width(text)+4,
                       style->font_hint->small_font->height(text)+4),
       text(new i4_str(text))
  {    
  }

  ~i4_mwm_context_help() 
  { 
    delete text; 
    if (shadow.get())
      i4_kernel.delete_handler(shadow.get());
  }

  char *name() { return "i4_mwm_context_help"; }
};

class i4_mwm_style_class : public i4_graphical_style_class
{
public:
  i4_image_class *close_icon;

  virtual void uninit()
  {
    if (close_icon)
    {
      delete close_icon;
      close_icon=0;
    }
    i4_graphical_style_class::uninit();
  }

  char *name() { return "MWM"; }
  virtual i4_parent_window_class *create_mp_window(i4_coord x, i4_coord y, 
                                                   w16 w, w16 h, 
                                                   const i4_const_str &title,
                                                   i4_event_reaction_class *on_delete=0
                                                   );

  virtual i4_bool close_mp_window(i4_parent_window_class *created_window)
  {
    if (!created_window) return i4_F;
    i4_parent_window_class *to_del=created_window->get_parent();
    if (!to_del) return i4_F;
    i4_parent_window_class *to_del_parent=to_del->get_parent();
    if (!to_del_parent) return i4_F;
    
    i4_kernel.delete_handler(to_del);

    return i4_T;
  }

  virtual i4_bool available_for_display(i4_display_class *whom) { return i4_T; }

  virtual i4_menu_class *create_menu(i4_bool hide_on_pick)
  { 
    return new i4_box_menu_class(this,hide_on_pick);
  }


  virtual void get_in_deco_size(w32 &left, w32 &top, w32 &right, w32 &bottom)
  {
    left=right=top=bottom=2;
  }

  virtual void get_out_deco_size(w32 &left, w32 &top, w32 &right, w32 &bottom)
  {
    left=right=top=bottom=2;
  }

  virtual void deco_neutral_fill(i4_image_class *screen,
                                 sw32 x1, sw32 y1, sw32 x2, sw32 y2, 
                                 i4_draw_context_class &context)
  {
    

    i4_rect_list_class sub_clip(&context.clip,0,0);
    sub_clip.intersect_area(x1,y1,x2,y2);
    sub_clip.swap(&context.clip);

    i4_image_class *im=icon_hint->background_bitmap;
    if (im)
    {
      int iw=im->width(), ih=im->height();
      int dx1=-context.xoff;
      int dy1=-context.yoff;
      while (dx1+iw<x1) dx1+=iw;
      while (dy1+ih<y1) dy1+=ih;

      int x,y;
      for (y=dy1; y<=y2; y+=ih)
        for (x=dx1; x<=x2; x+=iw)
          im->put_image(screen, x,y, context);        
    }
    else
      screen->clear(color_hint->neutral(), context);


    sub_clip.swap(&context.clip);
  }

  // draw a decoration around an area that looks like it's pressed into the screen
  virtual void draw_in_deco(i4_image_class *screen, 
                            i4_coord x1, i4_coord y1,
                            i4_coord x2, i4_coord y2,
                            i4_bool active,
                            i4_draw_context_class &context)
  {
    i4_color_hint_class::bevel *color;
    if (active)
      color=&color_hint->window.active;
    else 
      color=&color_hint->window.passive;

    screen->add_dirty(x1,y1,x2,y1+1,context);
    screen->add_dirty(x1,y1+2,x1+1,y2,context);
    screen->add_dirty(x1+2,y2-1,x2,y2,context);
    screen->add_dirty(x1-1,y1+2,x2,y2-2,context);

    screen->bar(x1,y1,x2,y1,color->dark,context);
    screen->bar(x1,y1,x1,y2,color->dark,context);

    screen->bar(x1+1,y1+1,x2-1,y1+1,color_hint->black,context);
    screen->bar(x1+1,y1+1,x1+1,y2-1,color_hint->black,context);

    screen->bar(x1+1,y2,x2,y2,color->bright,context);
    screen->bar(x2,y1+1,x2,y2,color->bright,context);

    screen->bar(x1+2,y2-1,x2-1,y2-1,color->medium,context);
    screen->bar(x2-1,y1+1,x2-1,y2-1,color->medium,context);   
  }

  // draw a decoration around an area that looks like it sticks out the screen
  virtual void draw_out_deco(i4_image_class *screen, 
                             i4_coord x1, i4_coord y1,
                             i4_coord x2, i4_coord y2,
                             i4_bool active,
                             i4_draw_context_class &context)
  {
    i4_color_hint_class::bevel *color;
    if (active)
      color=&color_hint->window.active;
    else 
      color=&color_hint->window.passive;


    screen->add_dirty(x1,y1,x2,y1+1,context);
    screen->add_dirty(x1,y1+2,x1+1,y2,context);
    screen->add_dirty(x1+2,y2-1,x2,y2,context);
    screen->add_dirty(x1-1,y1+2,x2,y2-2,context);

    screen->bar(x1,y1,x2,y1,color->bright,context);
    screen->bar(x1,y1,x1,y2,color->bright,context);

    screen->bar(x1+1,y1+1,x2-1,y1+1,color->medium,context);
    screen->bar(x1+1,y1+1,x1+1,y2-1,color->medium,context);

    screen->bar(x1+1,y2,x2,y2,color->dark,context);
    screen->bar(x2,y1+1,x2,y2,color->dark,context);

    screen->bar(x1+2,y2-1,x2-1,y2-1,color_hint->black,context);
    screen->bar(x2-1,y1+1,x2-1,y2-1,color_hint->black,context);
  
  }

  // this will create a temporary (quick) context help window at the mouse cursor
  // you are responsible for deleting the window
  virtual i4_window_class *create_quick_context_help(int mouse_x, int mouse_y,
                                                     const i4_const_str &str)
  {
    i4_parent_window_class *parent=i4_current_app->get_root_window();
    i4_mwm_context_help *w=new i4_mwm_context_help(this, str);

    if (w->width()+mouse_x+3 > parent->width())
      mouse_x=parent->width()-w->width()-3;
    if (w->height()+mouse_y+3 > parent->height())
      mouse_y=parent->height()-w->height()-3;

    i4_window_class *cw=new i4_color_window_class(w->width(), w->height(), 0, this);
    w->shadow=cw;

    parent->add_child(mouse_x+3, mouse_y+3, cw);   
    parent->add_child(mouse_x, mouse_y, w);



    return w;
  }

} mwm_style;


class i4_mwm_event_class : public i4_user_message_event_class
{
public:
  enum {
    CLOSE_YOURSELF,
    MAXIMIZE_YOURSELF
  } ;
  i4_window_class *from;

  i4_mwm_event_class(w8 sub_type, i4_window_class *from) 
    : i4_user_message_event_class(sub_type),from(from) {}

  virtual i4_event  *copy() { return new i4_mwm_event_class(sub_type,from); }

};


static void widget(i4_image_class *im,
                   i4_coord x1, i4_coord y1, i4_coord x2, i4_coord y2,
                   i4_color bright, i4_color med, i4_color dark,
                   i4_draw_context_class &context)
{
  // to keep from creating a dirty for each operation below
  im->add_dirty(x1,y1,x2,y2,context);      

  im->bar(x1,y1,x2,y1,bright,context);
  im->bar(x1,y1+1,x1,y2,bright,context);
  im->bar(x2,y1+1,x2,y2,dark,context);
  im->bar(x1+1,y2,x2-1,y2,dark,context);
  im->bar(x1+1,y1+1,x2-1,y2-1,med,context);
}


class i4_mwm_close_button_class : public i4_window_class
{  
private:
  i4_graphical_style_class *hint;
  i4_bool active;

public:
  char *name() { return "close_button"; }

  i4_mwm_close_button_class(w16 w, w16 h,i4_graphical_style_class *hint) 
    : i4_window_class (w,h),hint(hint)
  { 
    active=i4_F;
  }

  void activate(i4_bool yes)
  {
    active=yes;
    request_redraw();
  }
    

  virtual void receive_event(i4_event *ev)
  {
    i4_mwm_event_class c(i4_mwm_event_class::CLOSE_YOURSELF,this);
    if (ev->type()==i4_event::MOUSE_BUTTON_DOWN && parent)
      i4_kernel.send_event(parent,&c);
  }
 
  void draw(i4_draw_context_class &context)
  {
    i4_color_hint_class::bevel *color;
    /*    if (active)
      color=&hint->color_hint->window.active;
    else */
    color=&hint->color_hint->window.passive;

    local_image->add_dirty(0,0,width()-1,height()-1,context);



    widget(local_image, 0,0,width()-2,height()-2,color->bright,color->medium,color->dark,context);
    local_image->bar(0,height()-1,width()-1,height()-1,hint->color_hint->black,context);
    local_image->bar(width()-1,0,width()-1,height()-1,hint->color_hint->black,context);

    mwm_style.icon_hint->close_icon->put_image(local_image,2,2,context);    
  }

  virtual void show_self(w32 indent) 
  {    
    char fmt[50];
    sprintf(fmt,"%%%ds mwm_close_button",indent);
    i4_warning(fmt," ");
  }
} ;


static inline int get_res_num(char *name, int def)
{
  i4_const_str s=i4_string_man.get(name);
  if (s.null()) return def;
  i4_const_str::iterator i=s.begin();
  int r=i.read_number();
  int g=i.read_number();
  int b=i.read_number();
  return (r<<16)|(g<<8)|b;
}

// this is the top draggable part of a window
class i4_mwm_drag_bar_class : public i4_window_class
{
private:
  i4_graphical_style_class *hint;
  i4_bool active,dragging;
public:
  i4_const_str title;

  char *name() { return "drag_bar"; }

  enum 
  {
    MIN_WIDTH=15 
  };

  i4_mwm_drag_bar_class(w16 w, w16 h, const i4_const_str &title, i4_graphical_style_class *hint) : 
    i4_window_class(w,h), hint(hint), title(title)
  { 
    active=i4_F;
    dragging=i4_F;
  }

  void activate(i4_bool yes)
  {
    active=yes;
    request_redraw();
  }

  void draw(i4_draw_context_class &context)
  {
    i4_color_hint_class::bevel *color;
    if (active)
      color=&hint->color_hint->window.active;
    else 
      color=&hint->color_hint->window.passive;


    local_image->add_dirty(0,0,width()-1,height()-1,context);
    local_image->rectangle(0,0,width()-1,height()-1,hint->color_hint->black,context);

    i4_color sc,ec;

    if (active)
    { 
      sc=get_res_num("drag_bar_gradiant_active_start", 0x80); 
      ec=get_res_num("drag_bar_gradiant_active_end", 0x20); 
    }
    else
    {
      sc=get_res_num("drag_bar_gradiant_start", 0x707070); 
      ec=get_res_num("drag_bar_gradiant_end", 0x202020); 
    }

    i4_gradiant_bar(local_image, 1,1,width()-2,height()-2, sc, ec, context);


//     if (dragging)
//       local_image->widget(1,1,width()-2,height()-2,
//                           color->dark,color->medium,color->bright,context);
//     else
//       local_image->widget(1,1,width()-2,height()-2,
//                           color->bright,color->medium,color->dark,context);

    i4_font_class *font=hint->font_hint->normal_font;
    w16 strw=font->width(title);
    w16 strh=font->height(title);

    font->set_color(0xffffff);

    font->put_string(local_image,width()/2-strw/2,height()/2-strh/2,title,context);
  }
    
  virtual void receive_event(i4_event *ev)
  {
    if (ev->type()==i4_event::MOUSE_BUTTON_DOWN)
    {
      if (!dragging)
      {
	i4_window_request_drag_start_class drag(this);
	i4_kernel.send_event(parent,&drag);
	if (drag.return_result)
	{
	  dragging=i4_T;
	  request_redraw();
	}
      }
    } else if (ev->type()==i4_event::MOUSE_BUTTON_UP)
    {
      if (dragging)
      {
        i4_window_request_drag_end_class end_drag(this);
	i4_kernel.send_event(parent,&end_drag);
	dragging=i4_F;
	request_redraw();
      }
    } 
  }

};


class i4_mwm_window_class : public i4_draggable_window_class
{
private:
  i4_graphical_style_class *hint;
  i4_mwm_drag_bar_class *drag;
  i4_mwm_close_button_class *close_button;
  i4_bool active,draw_frame;
  w16 left,right,top,bottom;
  i4_parent_window_class *user_area;
  i4_event_reaction_class *on_delete;
 
  w32 close_button_width(i4_graphical_style_class *style)   
  { return 2+style->icon_hint->close_icon->width()+3; }

  w32 close_button_height(i4_graphical_style_class *style)   
  { return 2+style->icon_hint->close_icon->height()+3; }

  
  w32 dragbar_height(i4_graphical_style_class *style, const i4_const_str &title)
  {    
    return hint->font_hint->normal_font->height(title)+4+1;
  }

public:
  ~i4_mwm_window_class()
  {
    if (on_delete)
      delete on_delete;
    on_delete = 0;
  }

  i4_parent_window_class *user_window() { return user_area; }

  i4_mwm_window_class(w16 width, w16 height, 
                      const i4_const_str &title, 
                      i4_graphical_style_class *hint,
                      i4_event_reaction_class *on_delete)
    : i4_draggable_window_class(width,height), hint(hint),
      on_delete(on_delete)
  {   
    left=2;
    if (close_button_height(hint)>dragbar_height(hint,title))
      top=close_button_height(hint);
    else
      top=dragbar_height(hint,title);

    top+=2;  // top border

    right=2;
    bottom=2;

    
    active=i4_F;
    draw_frame=i4_T;
    
    resize(w+left+right,h+top+bottom);
    
    drag=new i4_mwm_drag_bar_class(w-left-right-close_button_width(hint),
                                   top-2,
                                   title,hint);
    add_child(2,2,drag);

    close_button=new i4_mwm_close_button_class(close_button_width(hint),
                                               close_button_height(hint),
                                               hint);

    add_child(w-right-close_button_width(hint),2,close_button);

    user_area=i4_add_color_window(this, hint->color_hint->window.passive.medium,
                                  hint,
                                  left, top, 
                                  width, height);
  } 

  virtual void parent_draw(i4_draw_context_class &context)
  {    
    if (draw_frame ||
	!undrawn_area.clipped_away(0,0,0+width()-1,0+1)     ||
	!undrawn_area.clipped_away(0,0,0+1,0+height()-1)  ||
	!undrawn_area.clipped_away(0+1,0+height()-2,0+width()-1,0+height()-1)  ||
	!undrawn_area.clipped_away(0+width()-2,0,0+width()-1,0+height()-1))
	
    {
      i4_color_hint_class::bevel *color;
      if (active)
        color=&hint->color_hint->window.active;
      else 
        color=&hint->color_hint->window.passive;


      local_image->add_dirty(0,0,width()-1,1,context);
      local_image->add_dirty(0,2,1,height()-1,context);
      local_image->add_dirty(2,height()-2,width()-1,height()-1,context);
      local_image->add_dirty(width()-2,2,width()-1,height()-3,context);

      local_image->bar(0,0,width()-1,0,color->medium,context);
      local_image->bar(0,1,0,height()-1,color->medium,context);

      local_image->bar(1,1,width()-2,1,color->bright,context);
      local_image->bar(1,2,1,height()-2,color->bright,context);

      local_image->bar(width()-2,2,width()-2,height()-2,color->medium,context);
      local_image->bar(2,height()-2,width()-3,height()-2,color->medium,context);

      local_image->bar(width()-1,0,width()-1,height()-1,hint->color_hint->black,context);
      local_image->bar(0,height()-1,width()-2,height()-1,hint->color_hint->black,context);
      draw_frame=i4_F;
    }


    if (!undrawn_area.empty())
      local_image->bar(2,2,width()-3,height()-3,hint->color_hint->window.passive.medium,context);

  }

  virtual i4_bool need_redraw() 
  { 
    return (i4_bool)(i4_parent_window_class::need_redraw()|draw_frame); 
  }

  void receive_event(i4_event *ev)
  {
    if (ev->type()==i4_event::WINDOW_MESSAGE)
    {
      CAST_PTR(f,i4_window_message_class,ev);
      if (f->sub_type==i4_window_message_class::GOT_MOUSE_FOCUS)
      {
	active=i4_T;
        drag->activate(i4_T);
	close_button->activate(i4_T);
	draw_frame=i4_T;

      }
      else if (f->sub_type==i4_window_message_class::LOST_MOUSE_FOCUS)
      {
        drag->activate(i4_F);
	close_button->activate(i4_F);
	active=i4_F;
	draw_frame=i4_T;

      } 
      else if (f->sub_type==i4_window_message_class::NOTIFY_RESIZE)
      {
        CAST_PTR(res,i4_window_notify_resize_class,ev);

        i4_mwm_window_class *new_parent=new i4_mwm_window_class(res->new_width,
                                                                res->new_height,
                                                                drag->title,
                                                                hint,
                                                                on_delete ? 
                                                                on_delete->copy() : 0);

        user_area->transfer_children(new_parent->user_area,0,0);

        // this will be post-poned
        i4_kernel.delete_handler(this);

        parent->add_child(x(),y(),new_parent);

      }

      i4_draggable_window_class::receive_event(ev);
    } else if (ev->type()==i4_event::USER_MESSAGE)
    {
      CAST_PTR(f,i4_mwm_event_class,ev);
      if (f->sub_type==i4_mwm_event_class::CLOSE_YOURSELF)
      {
        if (on_delete)
          i4_kernel.send(on_delete);

        i4_kernel.delete_handler(this);
      }
    } else i4_draggable_window_class::receive_event(ev);    
  }

} ;


i4_parent_window_class *i4_mwm_style_class::create_mp_window(i4_coord x, i4_coord y, 
                                                             w16 w, w16 h, 
                                                             const i4_const_str &title,
                                                             i4_event_reaction_class *on_delete)
{
    i4_parent_window_class *parent=i4_current_app->get_root_window();

  i4_mwm_window_class *win=new i4_mwm_window_class(w,h,title,this,on_delete);
  if (x==-1) 
    x=parent->width()/2-w/2;
  if (y==-1)
    y=parent->height()/2-h/2;

  parent->add_child(x,y,win);
  return win->user_window();
}
