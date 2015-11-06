/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "gui/button.hh"
#include "gui/butbox.hh"
#include "editor/editor.hh"
#include "editor/e_state.hh"
#include "gui/text.hh"
#include "device/event.hh"
#include "gui/deco_win.hh"
#include "gui/image_win.hh"
#include "loaders/load.hh"
#include "window/colorwin.hh"
#include "window/win_evt.hh"
#include "editor/dialogs/tile_picker.hh"
#include "editor/dialogs/object_picker.hh"
#include "editor/contedit.hh"
#include "player.hh"
#include "app/app.hh"
#include "map_man.hh"
#include "editor/e_res.hh"
#include "lisp/li_init.hh"

g1_edit_state_class g1_edit_state;

g1_controller_edit_class *g1_edit_state_class::current_focus()
{
  if (g1_editor_instance.in_editor() &&  g1_current_controller.get())
    return ((g1_controller_edit_class *)g1_current_controller.get());
  else
    return 0;
}

void g1_edit_state_class::hide_focus()
{
  if (current_focus())
    current_focus()->hide_focus();
}


void g1_edit_state_class::show_focus()
{
  if (current_focus())
    current_focus()->show_focus();
}

i4_graphical_style_class *g1_edit_state_class::get_style()
{
  return i4_current_app->get_style();
}


i4_button_class *g1_edit_state_class::create_button(char *res_name, 
                                                    w32 evalue, 
                                                    i4_bool popup,
                                                    i4_event_handler_class *send_to,
                                                    i4_event *send_event)
{
  if (send_to==0)
    send_to=this;

  i4_const_str s=g1_editor_instance.get_editor_string(res_name);
  if (s.null())
    i4_error("missing %s from res/editor.res",res_name);

  i4_const_str::iterator i=s.begin();
  sw32 icon_num=i.read_number();

  load_icons();

  if (icon_num<0 || icon_num>=total_icons)
    i4_error("bad icon #, %d",icon_num); 

  // change this to an icon at some point
  i4_image_window_class *t=new i4_image_window_class(icons[icon_num]);

  if (!send_event)
    send_event=new i4_object_message_event_class(send_to, evalue);

  i4_event_reaction_class *r=new i4_event_reaction_class(send_to, send_event);

  char help[30];
  sprintf(help,"%s_help",res_name);
  i4_button_class *b=new i4_button_class(&g1_ges(help), t, get_style(), r);


  if (popup)
    b->set_popup(popup);


  return b;
}

void g1_edit_state_class::add_but(i4_button_box_class *box,                     
                                  char *res_name, 
                                  w32 evalue, 
                                  i4_bool down,
                                  i4_event *send_event)
{
  char help[30];
  sprintf(help,"%s_help",res_name);
  i4_button_class *b=create_button(res_name, evalue, i4_F, 0, send_event);

  box->add_button(0,0, b);
  
  if (down)
    box->push_button(b, i4_F);

}


i4_window_class *g1_edit_state_class::create_buttons(w32 height)
{
  i4_graphical_style_class *style=get_style();

  i4_button_box_class *box=new i4_button_box_class(this);

  int i=0;
  g1_mode_creator *mc, *cur=0;

  for (mc=g1_mode_creator::first; mc; mc=mc->next, i++)
  {
    if (strcmp(major_mode, mc->name())==0)
      cur=mc;
  
    add_but(box, mc->name(), 0, (i4_bool) (cur==mc), new g1_set_major_mode_event(mc->name()));
  }

  if (cur)
  {
    box->arrange_right_down();

    i4_parent_window_class *minor_container=new i4_color_window_class(0, height,
                                                                      style->color_hint->neutral(),
                                                                      style);
    cur->create_buttons(minor_container);
    minor_container->resize_to_fit_children();
    
    i4_deco_window_class *d;
    d=new i4_deco_window_class(box->width() + minor_container->width(), height, i4_F, style);


    d->add_child(d->get_x1(), d->get_y1(), box);
    d->add_child(d->get_x1() + box->width(), d->get_y1(), minor_container);
    
    return d;
  }

  if (box)
    delete box;

  return 0;
}

void g1_edit_state_class::show_tools(i4_parent_window_class *p, 
                                     w32 _win_h)
{
  win_h=_win_h;
  parent=p;

  if (tools)
    delete tools;
  tools=create_buttons(win_h);
  parent->add_child(parent->width()-tools->width(), 0, tools);
  
}

void g1_edit_state_class::hide_tools()
{
  if (tools)
  {
    delete tools;
    tools=0;
    parent=0;
  }

  for (g1_mode_creator *mc=g1_mode_creator::first; mc; mc=mc->next)
    mc->cleanup();
}

void g1_edit_state_class::snap_point(i4_3d_point_class &p, int do_z)
{
  switch (snap)
  {
    case SNAP_CENTER : 
    {            
      p.x=((int)(p.x-0.5))+0.5; 
      p.y=((int)(p.y-0.5))+0.5; 
      if (do_z)
        p.z=((int)(p.z-0.5))+0.5; 
    } break;

    case SNAP_ORIGIN :
    {            
      p.x=((int)p.x); 
      p.y=((int)p.y); 
      if (do_z)
        p.y=((int)p.y); 
    } break;
  }
}

void g1_edit_state_class::set_current_team(int team_num)
{
  current_team=team_num;
}

g1_edit_state_class::g1_edit_state_class()
{
  current_team=0;

  snap=NO_SNAP;
  icons=0;
  strcpy(major_mode, "LIGHT");
  tools=0;
  parent=0;
 }

g1_mode_creator *g1_edit_state_class::get_major_mode()
{
  for (g1_mode_creator *mc=g1_mode_creator::first; mc; mc=mc->next)
  {
    if (strcmp(major_mode, mc->name())==0)
      return mc;
  }

  return 0;
}

i4_bool g1_edit_state_class::set_major_mode(char *mode_name)
{ 
  hide_focus();

  for (g1_mode_creator *mc=g1_mode_creator::first; mc; mc=mc->next)
  {
    if (strcmp(mode_name, mc->name())==0)
    {
      strcpy(major_mode, mc->name());

      if (tools)
        delete tools;

      tools=create_buttons(win_h);
      parent->add_child(parent->width()-tools->width(), 0, tools);

      show_focus();      
  
      g1_editor_instance.major_mode_change();

      return i4_T;
    }
  }     
  return i4_F;
}

i4_bool g1_edit_state_class::set_minor_mode(char *major_mode_name, w32 minor_mode)
{
  i4_bool ret;
  hide_focus();


  if (strcmp(major_mode, major_mode_name)==0 || set_major_mode(major_mode_name))
  {
    ret=get_major_mode()->set_minor_mode(minor_mode);
   
    if (tools)
    {
      delete tools;
      tools=create_buttons(win_h);
      parent->add_child(parent->width()-tools->width(), 0, tools);
    }
  }
  else
    ret=i4_F;
  
  show_focus();

  return ret;
}

void g1_edit_state_class::receive_event(i4_event *ev)
{
  CAST_PTR(oev, i4_object_message_event_class, ev);

  if (ev->type()==i4_event::USER_MESSAGE)
  {
    CAST_PTR(uev, i4_user_message_event_class, ev);
    if (uev->sub_type==G1_SET_MAJOR_MODE)
    {
      CAST_PTR(sm, g1_set_major_mode_event, ev);
      set_major_mode(sm->name);
    }
    else if (uev->sub_type==G1_SET_MINOR_MODE)
    {      
      CAST_PTR(sm, g1_set_minor_mode_event, ev);
      set_minor_mode(sm->name, sm->minor_mode);
    }
  }
}


void g1_edit_state_class::load_icons()
{
  if (icons) return;

  i4_const_str *e_cons=g1_editor_instance.get_editor_array("e_icons"), *e;
  e=e_cons;

  for (total_icons=0; !e->null(); total_icons++, e++);
  e=e_cons;

  icons=(i4_image_class **)i4_malloc(sizeof(i4_image_class *) * total_icons, "icons");

  for (w32 i=0; i<total_icons; i++, e++)
  {
    if (e->null())
      i4_error("not enough icons in e_icons array");

    icons[i]=i4_load_image(*e);
  }

  i4_free(e_cons);
}


void g1_edit_state_class::uninit()
{
  hide_tools();
  int i;
  

  for (i=0; i<total_icons; i++)
    if (icons[i])
    {
      delete icons[i];
      icons[i]=0;
    }

  if (icons)
    i4_free(icons);
  icons=0;
  total_icons=0;
}

w32 g1_edit_state_class::tools_width()
{
  if (tools)
    return tools->width();
  else return 0;
}




void g1_edit_state_class::context_help_struct::show(const i4_const_str &help, 
                                                    int _mx, int _my)
{
  hide();
  mx=_mx; my=_my;
  window=i4_current_app->get_style()->create_quick_context_help(_mx, _my+15, help);
}

void g1_edit_state_class::context_help_struct::hide()
{
  if (window.get())
  {
    i4_kernel.delete_handler(window.get());
    window=0;
  }
}

li_object *g1_edit_selected(li_object *o, li_environment *env)
{
  if (g1_edit_state.current_focus())
  {
    if (g1_edit_state.current_focus()->get_mode())
      g1_edit_state.current_focus()->get_mode()->edit_selected();
  }
  
  return 0;
  
}

li_automatic_add_function (g1_edit_selected, "edit_selected");
