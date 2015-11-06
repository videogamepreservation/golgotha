/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "gui/text_input.hh"
#include "gui/text.hh"
#include "lisp/li_types.hh"
#include "lisp/li_class.hh"
#include "lisp/li_dialog.hh"
#include "file/ram_file.hh"
#include "app/app.hh"
#include "memory/array.hh"
#include "gui/image_win.hh"
#include "gui/button.hh"
#include "gui/list_box.hh"
#include "menu/textitem.hh"
#include "window/wmanager.hh"


static li_symbol_ref li_list_box("list_box");

class li_generic_edit_class : public li_type_edit_class
{
public:
  enum input_type { TEXT_INPUT,
                    LIST_BOX };

  input_type get_type(li_object *property_list, li_environment *env)
  {
    if (property_list && property_list->type()==LI_LIST &&
        li_list_box.get()==li_car(property_list,env))
      return LIST_BOX;
    else 
      return TEXT_INPUT;
  }
  

  virtual int create_edit_controls(const i4_const_str &name,
                                   li_object *o, 
                                   li_object *property_list,
                                   i4_window_class **windows, 
                                   int max_windows,
                                   li_environment *env)
  {
    if (max_windows<2) return 0;
    
    char buf[200];
    i4_ram_file_class rf(buf, 30);
    li_get_type(o->type())->print(o, &rf);
    buf[rf.tell()]=0;

    i4_graphical_style_class *style=i4_current_app->get_style();

    windows[0]=new i4_text_window_class(name, style);

    if (get_type(property_list,env)==LIST_BOX)
    {
      property_list=li_cdr(property_list, env);
    
      i4_list_box_class *lb=new i4_list_box_class(200, style, 
                                                  i4_current_app->get_window_manager());

      int on=0;
      for (;property_list; property_list=li_cdr(property_list,env), on++)
      {
        char buf[100];
        i4_ram_file_class rf(buf, 100);
        li_object *v=li_car(property_list,env);


        li_get_type(v->type())->print(v, &rf);
        buf[rf.tell()]=0;

        lb->add_item(new i4_text_item_class(buf, style)); 

        if (v->type()==o->type() && li_get_type(v->type())->equal(o,v))
          lb->set_current_item(on);
       

      }
      windows[1]=lb;   
    }    
    else
    {
      if (o->type()==LI_STRING)
      {
        buf[strlen(buf)-1]=0;   // chop of end quote
        windows[1]=new i4_text_input_class(style,buf+1, 200,200);
      }
      else
        windows[1]=new i4_text_input_class(style,buf, 200,200);
    }
                                       
    return 2;
  }

  virtual i4_bool can_apply_edit_controls(li_object *o, 
                                          li_object *property_list,
                                          i4_window_class **windows)
  {
    return i4_T;    
  }

  virtual li_object *apply_edit_controls(li_object *o, 
                                         li_object *property_list,
                                         i4_window_class **windows,
                                         li_environment *env)
  {
    if (get_type(property_list, env)==LIST_BOX)
    {
      i4_list_box_class *ib=((i4_list_box_class *)windows[1]);
      return li_nth(property_list, ib->get_current()+1, env);
    } 
    else if (get_type(property_list, env)==TEXT_INPUT)
    {
      i4_text_input_class *w=((i4_text_input_class *)windows[1]);
      
      i4_const_str::iterator i=w->get_edit_string()->begin();
      if (o->type()==LI_INT)
        return new li_int(i.read_number());
      else if (o->type()==LI_FLOAT)
        return new li_float(i.read_float());
      else if (o->type()==LI_SYMBOL)
      {
        char buf[300];
        i4_os_string(*w->get_edit_string(), buf, 100);
        return li_get_symbol(buf);
      }
      else if (o->type()==LI_STRING)
        return new li_string(*w->get_edit_string());
      else
        return o;
    }
    else 
      return o;
  }
      

} li_generic_edit_instance;


class li_class_dialog_item : public li_dialog_item
{
public:
  char *name() { return "li_class_item"; }

  i4_array<li_dialog_item *> items;

  li_class_dialog_item(li_class *c, li_object *_prop_list, li_environment *env)
    : items(5,5)
  {
    prop_list=_prop_list; 
    o=c;

    int t=li_class_total_members(c->type()), i, max_colums=0;
    int colums[50];
    memset(colums, 0, sizeof(colums));

    for (i=0; i<t; i++)
    {
      li_symbol *sym=li_class_get_symbol(c->type(), i);
      li_object *val=c->value(i);
      li_object *prop_list=li_class_get_property_list(c->type(), sym);
      
      li_dialog_item *item=new li_dialog_item(sym->name()->value(), val, prop_list, env);
       items.add(item);

       int t_win=items[i]->t_windows;
       if (t_win>max_colums)
         max_colums=t_win;
       for (int j=0; j<t_win; j++)
         if (items[i]->windows[j] && items[i]->windows[j]->width()>colums[j])
           colums[j]=items[i]->windows[j]->width();
     }

     int dy=0, maxw=0;
     for (i=0; i<t; i++)
     {
       int dx=0, maxh=0;
       add_child(dx,dy, items[i]);

       for (int j=0; j<items[i]->t_windows; j++)
       {
         if (items[i]->windows[j])
         {
           int xoff=dx-(items[i]->windows[j]->x()-items[i]->x());
           items[i]->windows[j]->move(xoff,0);

           if (items[i]->windows[j]->height()>maxh)
             maxh=items[i]->windows[j]->height();
         }

         dx+=colums[j]+3;
         if (dx>maxw) maxw=dx;
       }
       items[i]->resize_to_fit_children();

       dy+=maxh+1;        
     }      

     private_resize(maxw,dy);
   }

   i4_bool can_apply(li_environment *env)
   {
     for (int i=0; i<items.size(); i++)
       if (!items[i]->can_apply(env))
         return i4_F;
     return i4_T;
   }

   li_object *apply(li_environment *env)
   {
     li_class *c=(li_class *)li_new(o.get()->type());

     for (int i=0; i<items.size(); i++)
       c->set_value(i, items[i]->apply(env));

     return c;
   }
 };

 class li_class_edit_class : public li_type_edit_class
 {
 public:
   int create_edit_controls(const i4_const_str &name,
                            li_object *object, 
                            li_object *property_list,
                            i4_window_class **windows, 
                            int max_windows,
                            li_environment *env)
   {
     if (max_windows)
     {
       windows[0]=new li_class_dialog_item(li_class::get(object,env), property_list, env);
       return 1;
     } else return 0;    
   }

   i4_bool can_apply_edit_controls(li_object *objectw, 
                                   li_object *property_list,
                                   i4_window_class **windows,
                                   li_environment *env)
   {
     return ((li_class_dialog_item *)windows[0])->can_apply(env);
   }

   li_object *apply_edit_controls(li_object *o, 
                                  li_object *property_list,
                                  i4_window_class **windows,
                                  li_environment *env)
   {
     return ((li_class_dialog_item *)windows[0])->apply(env);   
   }

 } li_class_edit_instance;


 class li_generic_edit_initer : public i4_init_class
 {
 public:
   int init_type() { return I4_INIT_TYPE_LISP_FUNCTIONS; }

   void init()
   {
     li_get_type(LI_INT)->editor=&li_generic_edit_instance;
     li_get_type(LI_FLOAT)->editor=&li_generic_edit_instance;
     li_get_type(LI_SYMBOL)->editor=&li_generic_edit_instance;
     li_get_type(LI_STRING)->editor=&li_generic_edit_instance;
     li_set_class_editor(&li_class_edit_instance);
   }

 } li_generic_edit_initer_instance;


 li_dialog_item::li_dialog_item() 
   : i4_color_window_class(0,0, i4_current_app->get_style()->color_hint->neutral(),
                           i4_current_app->get_style())
 { 
   windows=0; 
   t_windows=0; 
   o=0;
   prop_list=0;
 }


 li_dialog_item::li_dialog_item(const i4_const_str &name, 
                                li_object *_o, 
                                li_object *prop_list,
                                li_environment *env)

   : i4_color_window_class(0,0, i4_current_app->get_style()->color_hint->neutral(),
                           i4_current_app->get_style()),
     prop_list(prop_list)
 {
   o=_o;
   windows=0;
   t_windows=0;

   i4_window_class *w[10];

   if (li_get_type(o->type())->editor)
   {
     if (prop_list!=li_get_symbol("no_edit"))
     {
       t_windows=li_get_type(o->type())->editor->create_edit_controls(name,
                                                                      o.get(),
                                                                      prop_list,
                                                                      w, 10, env);
       if (t_windows)
       {       
         windows=(i4_window_class **)i4_malloc(sizeof(i4_window_class *) * t_windows,"");
         int x=0, i, maxh=0;
         for (i=0; i<t_windows; i++)
           if (w[i] && w[i]->height()>maxh)
             maxh=w[i]->height();


         for (i=0; i<t_windows; i++)
         {
           windows[i]=w[i];

           add_child(x, maxh/2 - windows[i]->height()/2 , windows[i]);
           x+=w[i]->width();
         }
         resize_to_fit_children();
       }
     }
   }
 }



 i4_bool li_dialog_item::can_apply(li_environment *env)
 {
   if (!li_get_type(o->type())->editor)  return i4_T;

   return li_get_type(o->type())->editor->can_apply_edit_controls(o.get(), prop_list, windows,env);
 }


 li_object *li_dialog_item::apply(li_environment *env)
 {
   if (li_get_type(o->type())->editor)
     return li_get_type(o->type())->editor->apply_edit_controls(o.get(), prop_list, windows, env);
   return o.get();
 }  

 li_dialog_item::~li_dialog_item()
 {
   if (windows)
     i4_free(windows);
 }


 i4_graphical_style_class *li_dialog_window_class::style()
 {
   return i4_current_app->get_style(); 
 }



 li_dialog_window_class::~li_dialog_window_class()
 {
   if (called_on_close)
   {
     if (new_value.get())
       called_on_close(li_make_list(new_value.get(), o.get(), 0),0);
     called_on_close=0;
   }
 }

 li_dialog_window_class::li_dialog_window_class(const i4_const_str &name,
                                                li_object *_o, 
                                                li_object *_prop_list,
                                                li_function_type called_on_close,
                                                li_environment *env)
   : i4_color_window_class(0,0, style()->color_hint->neutral(), style()),
     called_on_close(called_on_close), enviroment(env)
 {
   o=_o;
   prop_list=_prop_list;


   mp_handle=0;
   int t=li_get_type(o->type())->editor->create_edit_controls(name, o.get(), 
                                                             prop_list.get(), w, 10,
                                                              env);
  int x=0, maxh=0;
  for (int i=0; i<t; i++)
  {
    add_child(x, 0, w[i]);
    if (w[i]->height()>maxh)
      maxh=w[i]->height();
  }

  i4_window_class *ok, *cancel;

  if (style()->icon_hint->ok_icon && style()->icon_hint->cancel_icon)
  {
    ok=new i4_image_window_class(style()->icon_hint->ok_icon);
    cancel=new i4_image_window_class(style()->icon_hint->cancel_icon);
  }
  else
  {
    ok=new i4_text_window_class(i4gets("ok"), style());
    cancel=new i4_text_window_class(i4gets("cancel"), style());
  }
    
  resize_to_fit_children();

  i4_button_class *okb=new i4_button_class(0, ok, style(), 
                                           new i4_event_reaction_class(this, 1));
  i4_button_class *cancelb=new i4_button_class(0, cancel, style(), 
                                               new i4_event_reaction_class(this, 1));
  x=width()/2-okb->width()/2-cancelb->width()/2;
  if (x<0) x=0;

  add_child(x, maxh+1, okb);
  add_child(x+okb->width(), maxh+1, cancelb);

  resize_to_fit_children();

}

void li_dialog_window_class::receive_event(i4_event *ev)
{
  if (ev->type()==i4_event::USER_MESSAGE)
  {
    if (((i4_user_message_event_class *)ev)->sub_type==1)
    {
      if (!li_get_type(o->type())->editor->can_apply_edit_controls(o.get(), prop_list.get(), w, env()))
        return;
      else
        new_value=li_get_type(o->type())->editor->apply_edit_controls(o.get(),
                                                                      prop_list.get(), w, env());

    }

    if (mp_handle)
      style()->close_mp_window(mp_handle);        

    if (called_on_close)
    {
      if (new_value.get())
        called_on_close(li_make_list( new_value.get(), o.get(),  0),0);
      called_on_close=0;
    }

  }
  else
    i4_color_window_class::receive_event(ev);
}
  


li_dialog_window_class *li_create_dialog(const i4_const_str &name,
                                         li_object *o, 
                                         li_object *prop_list,
                                         char *close_fun,
                                         li_environment *env)
{
  li_function_type fun=0;
  if (close_fun)
    fun=li_function::get(li_get_fun(li_get_symbol(close_fun), env),env)->value();

  li_dialog_window_class *d=new li_dialog_window_class(name, o,prop_list, fun, env);

  i4_parent_window_class *mp;
  mp=i4_current_app->get_style()->create_mp_window(-1,-1, d->width(), d->height(),
                                                   name, 0);
  d->mp_handle=mp;
  mp->add_child(0,0,d);
  return d;
}




li_dialog_window_class *li_create_dialog(const i4_const_str &name,
                                         li_object *o, 
                                         li_object *prop_list,
                                         li_function_type fun,
                                         li_environment *env)
{
  li_dialog_window_class *d=new li_dialog_window_class(name, o,prop_list, fun, env);

  i4_parent_window_class *mp;
  mp=i4_current_app->get_style()->create_mp_window(-1,-1, d->width(), d->height(),
                                                   name, 0);
  d->mp_handle=mp;
  mp->add_child(0,0,d);
  return d;
}



