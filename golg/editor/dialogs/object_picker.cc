/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "editor/mode/e_object.hh"
#include "editor/dialogs/obj_win.hh"
#include "gui/scroll_bar.hh"
#include "menu/textitem.hh"
#include "app/app.hh"
#include "gui/deco_win.hh"
#include "g1_render.hh"
#include "r1_win.hh"
#include "device/device.hh"
#include "device/event.hh"
#include "device/keys.hh"
#include "lisp/li_init.hh"
#include "editor/editor.hh"
#include "editor/e_res.hh"

static int to_upper(int ch) { return ch>='a' && ch<='z' ? ch-'a' : ch; }

int obj_name_compare(const g1_object_type *a, const g1_object_type *b)
{
  return strcmp(g1_object_type_array[*a]->name(), g1_object_type_array[*b]->name());
}

class g1_object_picker_class : public i4_color_window_class
{
  g1_3d_object_window *obj_view;

  i4_array<g1_object_type> selectable;
  i4_array<i4_text_item_class *> text;
  int offset;
  w32 bg_color;


public:
  enum { W=128,H=400 };

  void recolor_text()
  {
    for (int i=0; i<text.size(); i++)
    {     
      w32 new_color;
      if (i+offset>=selectable.size() ||
          selectable[i+offset]==g1_e_object.get_object_type())
        new_color=0xffff00;
      else
        new_color=bg_color;

      if (text[i]->bg_color!=new_color)
      {
        text[i]->bg_color=new_color;
        text[i]->request_redraw(i4_F);
      }        
    }
  }

  void reorient_text()
  {
    for (int i=0; i<text.size(); i++)
    {
      if (i+offset<selectable.size())
        text[i]->change_text(g1_object_type_array[selectable[i+offset]]->name());
      else
        text[i]->change_text("");

    }

    recolor_text();
  }

  void refresh()
  {
    obj_view->set_object_type(g1_e_object.get_object_type(), 0);

    if (obj_view->object.valid())
      obj_view->camera.view_dist=obj_view->object->occupancy_radius()*1.25;

    request_redraw(i4_T);
  }


  g1_object_picker_class()
    : i4_color_window_class(W, H, i4_current_app->get_style()->color_hint->neutral(),
                            i4_current_app->get_style()),
      selectable(0,32),
      text(0,32)
  {

    offset=0;

    g1_3d_pick_window::camera_struct camera;
    camera.init();

    i4_graphical_style_class *style=i4_current_app->get_style();


    w32 l,t,r,b, ow, oh;
    style->get_in_deco_size(l,t,r,b);

    // put the render window in a deco window to make it stand out and look nice
    ow=W-(l+r)-2;
    oh=W-(t+b);

    i4_deco_window_class *deco=new i4_deco_window_class(ow, oh, i4_T, style);
    

    // object view must reside in a render window
    r1_render_window_class *rwin;
    rwin=g1_render.r_api->create_render_window(ow, oh, R1_COPY_1x1);

    obj_view=new g1_3d_object_window(ow, oh,
                                     g1_e_object.get_object_type(),
                                     0,
                                     camera,
                                     0,0, 0);
    
    rwin->add_child(0,0, obj_view);
    deco->add_child(deco->get_x1(), deco->get_y1(), rwin);   
    add_child(l,t, deco);



    // find out which objects are visible to the editor user
    int i=0, y;
    for (i=0; i<=g1_last_object_type; i++)
      if (g1_object_type_array[i] && 
          g1_object_type_array[i]->flags & g1_object_definition_class::EDITOR_SELECTABLE)
        selectable.add(i);

    selectable.sort(obj_name_compare);


    bg_color=style->color_hint->neutral();

    // create text fields so we know how many items fit on the screen
    for (t=0, y=deco->height()+2; y<height() && t<selectable.size(); y++)
    {
      text.add(new i4_text_item_class(g1_object_type_array[selectable[t]]->name(),
                                      style, style->color_hint, style->font_hint->small_font,
                                      new i4_event_reaction_class(this, t+1)));

      if (selectable[t]==g1_e_object.get_object_type())
        text[t]->bg_color=0xffff00;
      else
        text[t]->bg_color=bg_color;

        

      add_child(0,y, text[t]);
      y+=text[t]->height();
      t++;
    }
      

    // now add a scroll bar
    i4_scroll_bar *sb=new i4_scroll_bar(i4_T, height()-deco->height(), 
                                        t, selectable.size(), 0, this, style);

    add_child(width()-sb->width(), deco->height()+2, sb);

    // new resize the text fields to fit with the scroll bar
    for (i=0; i<text.size(); i++)
      text[i]->resize(W-sb->width(), text[i]->height());
  }


  void select_type(int type)
  {
    g1_edit_state.hide_focus();

    g1_e_object.set_object_type(type);

    if (strcmp(g1_edit_state.major_mode, "OBJECT"))
      g1_edit_state.set_major_mode("OBJECT");

    if (g1_e_object.get_minor_mode() != g1_object_params::ADD)
      g1_edit_state.set_minor_mode("OBJECT", g1_object_params::ADD);

    recolor_text();

    g1_edit_state.show_focus();
  }

  void receive_event(i4_event *ev)
  {
    if (ev->type()==i4_event::USER_MESSAGE)
    {
      CAST_PTR(uev, i4_user_message_event_class, ev);
      
      if (uev->sub_type==0)  // SCROLLBAR message
      {      
        CAST_PTR(sbm, i4_scroll_message, ev);
        offset=sbm->amount;
        reorient_text();
      }
      else                  // selected one of the text fields
        select_type(selectable[uev->sub_type-1+offset]);
    }
    else if (ev->type()==i4_event::KEY_PRESS)
    {
      CAST_PTR(kev, i4_key_press_event_class, ev);
      if (kev->key_code==I4_UP)
      {
        if (offset) { offset--; reorient_text(); }       
      }
      else if (kev->key_code==I4_DOWN)
      {
        if (offset<selectable.size()-1)
        {
          offset++;
          reorient_text();
        }
      }

      for (int i=0; i<selectable.size(); i++)
        if (to_upper(*g1_object_type_array[selectable[i]]->name())==kev->key_code)
        {
          offset=i;
          select_type(selectable[i]);
          return ;
        }
    }
    else 
      i4_color_window_class::receive_event(ev);
  }

};


i4_event_handler_reference_class<i4_parent_window_class> g1_object_picker_mp;
i4_event_handler_reference_class<g1_object_picker_class> g1_object_picker;

void g1_refresh_object_picker()
{
  if (g1_object_picker.get())
    g1_object_picker->refresh();
}


li_object *g1_toggle_object_picker(li_object *o, li_environment *env)
{
  i4_graphical_style_class *style=i4_current_app->get_style();
  if (g1_object_picker.get())
    style->close_mp_window(g1_object_picker.get());
  else
  {
    g1_object_picker=new g1_object_picker_class;
    g1_object_picker_mp=style->create_mp_window(-1,-1, 
                                                g1_object_picker->width(),
                                                g1_object_picker->height(),
                                                g1_ges("object_pick_title"));

    g1_object_picker_mp->add_child(0,0,g1_object_picker.get());
  }

  return 0;
}

li_automatic_add_function(g1_toggle_object_picker, "toggle_object_picker");
