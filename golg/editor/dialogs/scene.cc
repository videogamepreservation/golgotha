/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "editor/dialogs/scene.hh"
#include "gui/text_input.hh"
#include "gui/text.hh"
#include "gui/button.hh"
#include "m_flow.hh"
#include "g1_speed.hh"
#include "editor/editor.hh"
#include "window/colorwin.hh"
#include "editor/edit_id.hh"
#include "window/win_evt.hh"
#include "gui/smp_dial.hh"
#include "gui/image_win.hh"
#include "editor/e_res.hh"

class g1_scene_change_order_event : public i4_object_message_event_class
{
public:
  sw32 o,n;
  g1_scene_change_order_event(void *object, 
                              w32 sub_type, 
                              w32 old_order,
                              w32 new_order)
    : i4_object_message_event_class(object, sub_type),
      o(old_order),
      n(new_order)            {}

  virtual dispatch_time when()  { return LATER; }
  virtual i4_event  *copy() { return new g1_scene_change_order_event(object, sub_type, o,n); }
};



class g1_delete_scene_event_class : public i4_object_message_event_class
{
public:
  sw32 sn;
  g1_delete_scene_event_class(void *object,
                              w32 sub_type,
                              sw32 scene_number)
    : i4_object_message_event_class(object, sub_type),
      sn(scene_number)
  {}
  virtual dispatch_time when()  { return LATER; }
  virtual i4_event  *copy() { return new g1_delete_scene_event_class(object, sub_type, sn); }
};

class g1_cut_scene_editor_class : public i4_parent_window_class
{
  sw32 scene_number, start_time;

  g1_cut_scene_class *cs;
  i4_graphical_style_class *style;
  i4_text_input_class *scene_number_input;
  i4_text_input_class *scene_name_input;
  i4_text_input_class *scene_wav_input;
  i4_event_handler_class *eh;

  i4_window_class *dialog_active;

public:
  enum { DELETE_ME,
         DELETE_YES,
         DELETE_NO };
  
  char *name() { return "cut_scene_editor"; }

  ~g1_cut_scene_editor_class()
  {
    if (dialog_active)
      delete dialog_active;
  }

  sw32 tbox_width() { return 50; }
  sw32 name_width() { return 200; }
  sw32 wav_width() { return 150; }

  void send_delete_me()
  {
    g1_delete_scene_event_class del(eh, g1_scene_editor_class::DELETE_SCENE, scene_number);
    i4_kernel.send_event(eh, &del);
  }

  void create_delete_confirm_window()
  {
    dialog_active=i4_create_yes_no_dialog(root_window(),
                            style,
                            g1_ges("delete_title"),
                            g1_ges("delete_message"),
                            g1_ges("yes"), g1_ges("no"),
                            this,
                            new i4_object_message_event_class(this, DELETE_YES),
                            new i4_object_message_event_class(this, DELETE_NO));    
  }

  void receive_event(i4_event *ev)
  {
    if (ev->type()==i4_event::OBJECT_MESSAGE)
    {
      CAST_PTR(oev, i4_object_message_event_class, ev);
    
      if (oev->object==this)
      {
        switch (oev->sub_type)
        {
          case DELETE_ME :
            create_delete_confirm_window(); break;
          case DELETE_NO :
            dialog_active=0;
            break;

          case DELETE_YES :
            dialog_active=0;
            send_delete_me();
            break;                        
        }
      }
      else if (oev->object==scene_number_input)
      {
        CAST_PTR(tev, i4_text_change_notify_event, ev);
        i4_const_str::iterator i=tev->new_text->begin();
        sw32 sn=i.read_number();
        if (sn!=scene_number)
        {
          g1_scene_change_order_event co(eh, g1_scene_editor_class::CHANGE_SCENE_NUMBER,
                                         scene_number, sn);
          i4_kernel.send_event(eh, &co);
        }
      }
      else if (oev->object==scene_name_input)
      {
        CAST_PTR(tev, i4_text_change_notify_event, ev);
        if (cs->name) delete cs->name;
        cs->name=new i4_str(*tev->new_text);
      }
      else if (oev->object==scene_wav_input)
      {
        CAST_PTR(tev, i4_text_change_notify_event, ev);
        if (cs->wave_file) delete cs->wave_file;
        cs->wave_file=new i4_str(*tev->new_text);
      }


    }
    else if (!dialog_active || ev->type()!=i4_event::MOUSE_BUTTON_DOWN)
      i4_parent_window_class::receive_event(ev);
  }

  g1_cut_scene_editor_class(g1_cut_scene_class *cs,                             
                            sw32 scene_number,
                            sw32 start_time,          // in frames
                            i4_graphical_style_class *style,
                            i4_event_handler_class *eh)
    : cs(cs),
      scene_number(scene_number),
      start_time(start_time),
      style(style),
      i4_parent_window_class(0,0),
      eh(eh)
  {
    dialog_active=0;

    w32 l,r,t,b,xon=0;
    style->get_in_deco_size(l,t,r,b);

    i4_font_class *f=style->font_hint->normal_font;
    //resize(300, f->largest_height()+t+b+2);

    i4_str *fn=g1_ges("frame_format").sprintf(10,scene_number);

    scene_number_input=new i4_text_input_class(style,*fn,24,4, this);
    add_child(xon,0, scene_number_input); xon+=scene_number_input->width();

    xon+=tbox_width()*2;

    i4_text_window_class *tw=new i4_text_window_class(*cs->name,style);
    

    scene_name_input=new i4_text_input_class(style,*cs->name,name_width(), 100, this);
    add_child(xon, 0, scene_name_input);   xon+=scene_name_input->width();

    i4_str *s=cs->wave_file;
    if (!s)     
      scene_wav_input=new i4_text_input_class(style,g1_ges("null_string"),wav_width(), 100, this);
    else
      scene_wav_input=new i4_text_input_class(style,*s,wav_width(), 100, this);
    add_child(xon, 0, scene_wav_input);  xon+=scene_wav_input->width();

    // delete button
    i4_object_message_event_class *om;
    om=new i4_object_message_event_class(this, DELETE_ME);
    i4_image_class *del_icon=g1_editor_instance.delete_icon;
    i4_button_class *del=new i4_button_class(&g1_ges("delete_scene_help"),
                                             new i4_image_window_class(del_icon),
                                             style,
                                             new i4_event_reaction_class(this, om));
    del->set_popup(i4_T);
    add_child(xon, 0, del); xon+=del->width();

    resize(xon,f->largest_height()+t+b+2);

  }

  void draw_time_box(sw32 x, sw32 y, 
                     sw32 t, sw32 wid,
                     i4_draw_context_class &context)
                     
  {
    sw32 t_sec=t/G1_MOVIE_HZ;
    sw32 t_usec=(t-t_sec*G1_MOVIE_HZ)*60/G1_MOVIE_HZ;
    w32 l,r,top,b;

    style->get_in_deco_size(l,top,r,b);
    style->draw_in_deco(local_image, x,y, x+wid-1, height()-1, i4_F, context);
    local_image->bar(x+l,y+top, x+wid-r, y+height()-b, 
                     style->color_hint->window.passive.medium,
                     context);

    i4_str *s=g1_ges("time_format").sprintf(30, t_sec, t_usec);
    i4_font_class *f=style->font_hint->normal_font;

    f->set_color(style->color_hint->text_foreground);
    f->put_string(local_image, x+l, y+top+1,  *s, context);
    delete s;
  }


  void parent_draw(i4_draw_context_class &context)
  {
    local_image->clear(style->color_hint->window.passive.medium, context);

    draw_time_box(scene_number_input->width(), 0, start_time, tbox_width(), context);
    draw_time_box(scene_number_input->width()+tbox_width(), 0, cs->total_frames(),
                  tbox_width(), context);       
  }

};



void g1_scene_editor_class::show(i4_parent_window_class *parent_window,
                                 g1_movie_flow_class *_movie,
                                 i4_graphical_style_class *_style)
                                 
{
  movie=_movie;
  style=_style;

  if (parent.get())
    hide();

  i4_color_window_class *holder=new i4_color_window_class(0,0,0, style);

  w32 t=0;
  i4_window_class *addt=new i4_text_window_class(g1_ges("new"), style);
  i4_event_reaction_class *add_reaction;
  i4_event *add_event=new i4_object_message_event_class(this, ADD_SCENE);
  add_reaction=new i4_event_reaction_class(this, add_event);
  i4_button_class *addb=new i4_button_class(0, addt, style, add_reaction);
  addb->set_popup(i4_T);
  holder->add_child(0,0, addb);


  for (w32 i=0; i<movie->t_cut_scenes; i++)
  {    
    g1_cut_scene_editor_class *ce=new g1_cut_scene_editor_class(movie->set[i], i+1,t, style, this);
    t+=movie->set[i]->total_frames();

    holder->add_child(0,0, ce);
  }

  holder->arrange_right_down();
  holder->resize_to_fit_children();

  w32 w=holder->width(), h=holder->height();

  i4_event_reaction_class *re;
  re=new i4_event_reaction_class(this, new i4_object_message_event_class(this, WINDOW_CLOSED));
  parent=style->create_mp_window(wx, wy, w,h, g1_ges("scene_win_title"), re);

  holder->transfer_children(parent.get(), 0,0);
  delete holder;
                            
}

void g1_scene_editor_class::hide()
{
  if (parent.get())
  {
    if (parent->get_parent())
    {
      wx=parent->get_parent()->x();
      wy=parent->get_parent()->y();
    }

    style->close_mp_window(parent.get());

    parent=0;
  }
}


void g1_scene_editor_class::last_scene()
{
 
  if (movie->t_cut_scenes==1)
    movie->set_scene(0);
  else
  {
    if (movie->get_scene()!=0)
      movie->set_scene(movie->get_scene()-1);
  }
}

void g1_scene_editor_class::receive_event(i4_event *ev)
{
  CAST_PTR(oev, i4_object_message_event_class, ev);
  if (ev->type()==i4_event::OBJECT_MESSAGE)
  {
    if (oev->object==this)
    {
      switch (oev->sub_type)
      {
        case ADD_SCENE :
        {
          g1_editor_instance.add_undo(G1_MAP_MOVIE);

          i4_parent_window_class *parent_window=0;

          if (parent.get() && parent->get_parent() && parent->get_parent()->get_parent())
            parent_window=parent->get_parent()->get_parent();

          hide();
          g1_cut_scene_class *cs=movie->add_cut_scene(g1_ges("new_scene"));
          show(parent_window, movie, style);

          g1_editor_instance.changed();
        } break;

        case WINDOW_CLOSED :
        {
          parent=0;
        } break;

        case CHANGE_SCENE_NUMBER :
        {
          g1_editor_instance.add_undo(G1_MAP_MOVIE);

          i4_parent_window_class *parent_window=0;
          if (parent.get() && parent->get_parent() && parent->get_parent()->get_parent())
            parent_window=parent->get_parent()->get_parent();

          hide();

          CAST_PTR(co, g1_scene_change_order_event, ev);
          co->n--;
          co->o--;

          if (co->n>=movie->t_cut_scenes)
            co->n=movie->t_cut_scenes-1;
          else if (co->n<0)
            co->n=0; 

          sw32 i;
          g1_cut_scene_class *c=movie->set[co->o];
          for (i=co->o; i<movie->t_cut_scenes-1; i++)
            movie->set[i]=movie->set[i+1];

          for (i=movie->t_cut_scenes-1; i>co->n; i--)
            movie->set[i]=movie->set[i-1];
          
          movie->set[co->n]=c;

          show(parent_window, movie, style);

          g1_editor_instance.changed();
        } break;

        case DELETE_SCENE :
        {
          g1_editor_instance.add_undo(G1_MAP_MOVIE);
    
          i4_parent_window_class *parent_window=0;
          if (parent.get() && parent->get_parent() && parent->get_parent()->get_parent())
            parent_window=parent->get_parent()->get_parent();

          hide();

          CAST_PTR(dev, g1_delete_scene_event_class, ev);
          dev->sn--;

          last_scene();
          g1_cut_scene_class *c=movie->set[dev->sn];
          
          for (w32 i=dev->sn; i<movie->t_cut_scenes-1; i++)
            movie->set[i]=movie->set[i+1];

          movie->t_cut_scenes--;

          delete c;

          show(parent_window, movie, style);

          g1_editor_instance.changed();
        } break;

      }
    }
  }
}
