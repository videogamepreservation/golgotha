/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/


#include "loaders/load.hh"
#include "menu.hh"
#include "image/image.hh"
#include "window/style.hh"
#include "device/event.hh"
#include "menu/textitem.hh"
#include "window/win_evt.hh"
#include "error/alert.hh"

class g1_menu_item_class : public i4_window_class
{
public:
  i4_bool active;
  i4_image_class *act_im;
  i4_event_reaction_class *press;
  g1_menu_item_class(i4_image_class *act_im,
                     i4_event_reaction_class *press)
    : i4_window_class(act_im->width(), act_im->height()),
      act_im(act_im),
      press(press)
  {
    active=i4_F;
  }

  char *name() { return "g1_menu_item"; }
  
  virtual i4_bool transparent() { return !active; }
  virtual void draw(i4_draw_context_class &context)
  {
    if (active)
      act_im->put_image(local_image, 0,0, context);
    i4_window_class::draw(context);
  }

  virtual void receive_event(i4_event *ev)
  {
    if (ev->type()==i4_event::WINDOW_MESSAGE)
    {
      CAST_PTR(wev, i4_window_message_class, ev);
      if (wev->sub_type==i4_window_message_class::GOT_MOUSE_FOCUS)
      {
        active=i4_T;
        request_redraw();
      }
      else if (wev->sub_type==i4_window_message_class::LOST_MOUSE_FOCUS)
      {
        active=i4_F;
        request_redraw();
      }          
    }
    else if (ev->type()==i4_event::MOUSE_BUTTON_DOWN)
    {
      if (active)
        i4_kernel.send(press);
    }
    else i4_window_class::receive_event(ev);
  }
  
  ~g1_menu_item_class()
  {
    delete act_im;
    delete press;
  }
};

g1_main_menu_class::g1_main_menu_class(w16 w, w16 h, 
                                       i4_event_handler_class *notify,
                                       sw32 *ids,
                                       i4_graphical_style_class *style)
  : i4_parent_window_class(w,h),
    notify(notify),
    style(style)
{
  deco=i4_load_image(i4gets("title_screen"));




  i4_const_str *tb=i4_string_man.get_array("title_buts");
  int on=0;
  for (int i=0; !tb[i].null();)
  {
    i4_image_class *im=i4_load_image(tb[i]);
    if (!im)
    {
      i4_alert(i4gets("file_missing"),100,&tb[i]);
      i4_error("could load image");
    }
    i++;
    i4_const_str::iterator it=tb[i].begin();
    int x=it.read_number();
    i++;
    it=tb[i].begin();
    int y=it.read_number();
    i++;

    i4_event_reaction_class *re;
    re=new i4_event_reaction_class(notify, new i4_user_message_event_class(ids[on]));
    on++;

    g1_menu_item_class *gi=new g1_menu_item_class(im,re);
    add_child(x,y, gi);

  }

  i4_free(tb);
}

void g1_main_menu_class::parent_draw(i4_draw_context_class &context)
{
  i4_rect_list_class child_clip(&context.clip,0,0);
  child_clip.intersect_list(&undrawn_area);
  child_clip.swap(&context.clip);

  if (deco)
  {
    i4_coord xp,yp;

    xp=width()/2-deco->width()/2;
    yp=height()/2-deco->height()/2;
    if (!undrawn_area.empty())
      local_image->clear(0,context);


    deco->put_image(local_image,xp,yp,context);
  } 
  else 
  {
    if (!undrawn_area.empty())
      local_image->clear(0,context);
    request_redraw();
  }

  child_clip.swap(&context.clip);
  i4_parent_window_class::parent_draw(context);

}


g1_main_menu_class::~g1_main_menu_class()
{

  delete deco;
}
