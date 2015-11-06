/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "editor/editor.hh"
#include "lisp/li_dialog.hh"
#include "gui/button.hh"
#include "gui/text.hh"
#include "app/app.hh"
#include "li_objref.hh"
#include "editor/e_res.hh"

class li_objref_change_button_class : public i4_button_class
{
public:
  i4_text_window_class *show_name;
  w32 current_id;

  void update_name()
  {
    char buf[200];

    if (g1_global_id.check_id(current_id))
      sprintf(buf, "%s, %d", g1_global_id.get(current_id)->name(), current_id);
    else
      sprintf(buf, "0");

    if (show_name)
      show_name->set_text(new i4_str(buf));
  }


  li_objref_change_button_class(const i4_const_str &help,
                                i4_window_class *child,
                                i4_graphical_style_class *style,
                                i4_text_window_class *show_name,
                                w32 current_id)
    : i4_button_class(&help, child, style),
      show_name(show_name),
      current_id(current_id)
  {
    set_popup(i4_T);
    update_name();
  }

                      
  void do_press()
  {
    w32 selected_objects[G1_MAX_OBJECTS];
    int t_sel=g1_get_map()->make_selected_objects_list(selected_objects, G1_MAX_OBJECTS);
    if (t_sel)
      current_id=selected_objects[0];
    else
      current_id=g1_global_id.invalid_id();

    i4_button_class::do_press();
    update_name();
  }
};



class li_objref_list_controls : public i4_parent_window_class
{
public:
  i4_text_window_class *show_name;

  enum { ADD, DEL, CLEAR };

  li_object_pointer c;
  
  li_g1_ref_list *get() { return (li_g1_ref_list *)c.get(); }

  i4_graphical_style_class *style() { return i4_current_app->get_style(); }

  i4_button_class *create_but(char *name, int id)
  {
    char help_name[50];
    sprintf(help_name, "%s_help", name);

    i4_button_class *b=0;
    b=new i4_button_class(&g1_ges(help_name), 
                          new i4_text_window_class(g1_ges(name), style()),
                          style(),
                          new i4_event_reaction_class(this, id));
    b->set_popup(i4_T);
    return b;
  }

  li_objref_list_controls(li_g1_ref_list *o,
                          i4_text_window_class *show_name)
    : show_name(show_name),
      i4_parent_window_class(0,0)
  {
    c=new li_g1_ref_list;
    for (int i=0; i<o->size(); i++)
    {
      g1_object_class *obj=o->value(i);
      if (obj)
        get()->add(obj);
    }


    add_child(0,0, create_but("add_links", ADD));
    add_child(0,0, create_but("del_links", DEL));
    add_child(0,0, create_but("clear_links", CLEAR));
    arrange_down_right();
    resize_to_fit_children();
  }

  void receive_event(i4_event *ev)
  {
    if (ev->type()==i4_event::USER_MESSAGE)
    {
      w32 sel[G1_MAX_OBJECTS];
      int t_sel=g1_get_map()->make_selected_objects_list(sel, G1_MAX_OBJECTS);

      switch (((i4_user_message_event_class *)ev)->sub_type)
      {
        case ADD :
        {
          for (int i=0; i<t_sel; i++)
            if (get()->find(sel[i])==-1)
              get()->add(sel[i]);
        } break;

        case DEL:
        {
          for (int i=0; i<t_sel; i++)
            if (get()->find(sel[i])!=-1)
              get()->remove(sel[i]);
        } break;

        case CLEAR :
        {
          while (get()->size())
            get()->remove(get()->get_id(0));
        } break;
      }

      if (show_name)
        show_name->set_text(i4_const_str("%d").sprintf(20, get()->size()));

    }
    else i4_parent_window_class::receive_event(ev);
  }

  char *name() { return "objref_list_controls"; }

};

class li_object_reference_edit_class : public li_type_edit_class
{
public:

  virtual int create_edit_controls(const i4_const_str &name,
                                   li_object *o, 
                                   li_object *property_list,
                                   i4_window_class **windows, 
                                   int max_windows,
                                   li_environment *env)
  {
    if (max_windows<3) return 0;
    
    char buf[200];
    sprintf(buf, "Name of object linked to");


    windows[0]=new i4_text_window_class(name, i4_current_app->get_style());
   
    i4_text_window_class *ti=new i4_text_window_class(buf, i4_current_app->get_style());
    windows[2]=ti;

    i4_graphical_style_class *style=i4_current_app->get_style();

    w32 id=li_g1_ref::get(o,env)->id();
    windows[1]=new li_objref_change_button_class(g1_ges("change_link_help"),
                                                 new i4_text_window_class(g1_ges("change_link"), 
                                                                          style),
                                                 style,
                                                 ti, id);
       
    return 3;
  }

  virtual li_object *apply_edit_controls(li_object *o, 
                                         li_object *property_list,
                                         i4_window_class **windows,
                                         li_environment *env)
  {
    return new li_g1_ref(((li_objref_change_button_class *)windows[1])->current_id);
  }

} li_object_reference_edit_instance;


class li_object_list_reference_edit_class : public li_type_edit_class
{
public:
  virtual int create_edit_controls(const i4_const_str &name,
                                   li_object *o, 
                                   li_object *property_list,
                                   i4_window_class **windows, 
                                   int max_windows,
                                   li_environment *env)
  {
    if (max_windows<3) return 0;

    li_g1_ref_list *r=li_g1_ref_list::get(o,env);
    
    char buf[200];
    sprintf(buf, "%d : %s", r->size(), r->size() && r->value(0) ?
            r->value(0)->name() : 0);

    windows[0]=new i4_text_window_class(name, i4_current_app->get_style());
   
    i4_text_window_class *ti=new i4_text_window_class(buf, i4_current_app->get_style());
    windows[2]=ti;

    i4_graphical_style_class *style=i4_current_app->get_style();

    windows[1]=new li_objref_list_controls(li_g1_ref_list::get(o,env),
                                           ti);
       
    return 3;
  }

  virtual li_object *apply_edit_controls(li_object *o, 
                                         li_object *property_list,
                                         i4_window_class **windows,
                                         li_environment *env)
  {
    return ((li_objref_list_controls *)windows[1])->get();
  }

} li_object_list_reference_edit_instance;


class li_objref_edit_initer : public i4_init_class
{
public:
  void init() 
  {
    li_get_type(li_find_type("object_ref"))->editor=&li_object_reference_edit_instance;
    li_get_type(li_find_type("object_ref_list"))->editor=&li_object_list_reference_edit_instance;
  }

} i_objref_edit_instance;
