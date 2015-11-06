/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "window/window.hh"
#include "window/style.hh"
#include "gui/button.hh"
#include "editor/dialogs/e_time.hh"
#include "window/win_evt.hh"
#include "gui/image_win.hh"
#include "gui/button.hh"
#include "device/keys.hh"
#include "gui/text_input.hh"
#include "gui/deco_win.hh"
#include "gui/text.hh"
#include "math/num_type.hh"
#include "editor/e_state.hh"
#include "math/spline.hh"
#include "m_flow.hh"
#include "editor/editor.hh"
#include "editor/e_state.hh"
#include "loaders/load.hh"
#include "gui/butbox.hh"
#include "g1_speed.hh"
#include "mess_id.hh"
#include "app/app.hh"
#include "editor/e_res.hh"

class g1_time_show : public i4_window_class
{
  i4_graphical_style_class *style;
  i4_event_handler_reference_class<g1_time_line> tl;
public:
  g1_time_show(i4_graphical_style_class *style,
               g1_time_line *tl) 
    : i4_window_class(style->font_hint->normal_font->largest_width()*6,
                      style->font_hint->normal_font->largest_height()),
      style(style)
  {
    tl=tl;
  }

  void draw(i4_draw_context_class &context)
  {
    style->deco_neutral_fill(local_image, 0,0, width()-1, height()-1, context);

    if (!tl.get()) return;

    i4_font_class *f=style->font_hint->normal_font;
    f->set_color(style->color_hint->text_foreground);

    i4_float t=tl->current_frame()/(i4_float)(G1_MOVIE_HZ);
    sw32 sec=(sw32)t;
    sw32 msec=(sw32)((t-sec)*100);

    i4_str *out=g1_editor_instance.get_editor_string("time_format").sprintf(20, sec, msec);
    f->put_string(local_image, 0,0, *out, context);
    delete out;
  }

  char *name() { return "time show"; }
};

class g1_frame_show : public i4_text_input_class
{
  i4_event_handler_reference_class<g1_time_line> tl;
public:
  g1_frame_show(i4_graphical_style_class *style,
               g1_time_line *tl)
    : i4_text_input_class(style,
                          g1_editor_instance.get_editor_string("0"),
                          50,
                          8)
  {
    tl=tl;
  }

  virtual void become_unactive()
  {
    i4_const_str::iterator i=st->begin();
    w32 v=i.read_number();    
    i4_text_input_class::become_unactive();
  }

  void receive_event(i4_event *ev)
  {
    if (!tl.get()) return;

    CAST_PTR(kev, i4_key_press_event_class, ev);
    if (ev->type()==i4_event::KEY_PRESS && kev->key==I4_ENTER)
    {
      become_unactive();

      i4_const_str::iterator i=st->begin();
      sw32 x=i.read_number();

      if (tl->selected_spline() && x>0)
      {
        g1_editor_instance.add_undo(G1_MAP_MOVIE);

        i4_spline_class::point *p;
        w32 frame=tl->current_frame();
        p=tl->selected_spline()->get_control_point_previous_to_frame(frame);

        sw32 add=x-(p->next->frame-p->frame);
        p=p->next;


        for (;p;p=p->next)
          p->frame+=add;

        g1_editor_instance.changed();

       
      }


    }
    else i4_text_input_class::receive_event(ev);
  }
    
  void update()
  {
    i4_str *st;
    if (!tl.get()) return;
    if (tl->selected_spline())
    {
      i4_spline_class::point *p;
      w32 frame=tl->current_frame();
      p=tl->selected_spline()->get_control_point_previous_to_frame(frame);

      if (p && p->next)
        st=g1_editor_instance.get_editor_string("frame_format").sprintf(20,
                                                                        p->next->frame-p->frame);
      else
        st=new i4_str(g1_editor_instance.get_editor_string("no_next"));
    }
    else
      st=new i4_str(g1_editor_instance.get_editor_string("not_sel"));

    change_text(*st);
    delete st;
  }
    
  char *name() { return "time show"; }
};

class g1_time_scroller : public i4_parent_window_class
{
  i4_event_handler_reference_class<g1_time_line> tl;
  i4_graphical_style_class *style;
  sw32 mx,my;
  i4_bool mouse_down;

public:
  char *name() { return "time sroller"; }

  g1_time_scroller(w16 w, w16 h, 
                   i4_graphical_style_class *style,
                   g1_time_line *tl) 
    : i4_parent_window_class(w,h),
      style(style)
  {
    tl=tl;
    mouse_down=i4_F;
  }
    
  void parent_draw(i4_draw_context_class &context)
  {
    style->draw_in_deco(local_image, 0,0, width()-1, height()-1, i4_F, context);

    w32 l,r,t,b;
    style->get_in_deco_size(l,r,t,b);
    local_image->bar(l,r,width()-r,height()-t, style->color_hint->window.active.medium, context);

    local_image->line(l,height()/2,width()-r, height()/2, 0xffffffff, context);
    
    if (!tl.get()) return;
    int tf=tl->total_frames();
    if (tf)
    {
      w32 o=tl->current_frame()*(width()-l-r)/tf+l;
      local_image->line(o,t,o,height()-b, 0xffffffff, context);
    }
  }


  void mouse_change_time()
  {
    if (!tl.get()) return;

    w32 l,r,t,b;
    style->get_in_deco_size(l,r,t,b);
    if (mx>=width()-r)
      tl->set_current(tl->total_frames()-1, i4_T);
    else if (mx<l)
      tl->set_current(0, i4_T);
    else
      tl->set_current( (mx-l)*tl->total_frames()/(width()-l-r), i4_T);
    request_redraw();

  }

  void g1_time_scroller::receive_event(i4_event *ev)
  {
   
    switch (ev->type())
    {
      case i4_event::MOUSE_MOVE :
      {       
        CAST_PTR(mev, i4_mouse_move_event_class, ev);
        mx=mev->x;
        my=mev->y;
        if (mouse_down)
          mouse_change_time();
      } break;

      case i4_event::MOUSE_BUTTON_DOWN :
      {
        CAST_PTR(mev, i4_mouse_button_down_event_class, ev);
        if (mev->but==i4_mouse_button_down_event_class::LEFT)
        {
          i4_window_request_mouse_grab_class grab(this);
          i4_kernel.send_event(parent,&grab);
          mouse_down=i4_T;
          mouse_change_time();
        }
      } break;

      case i4_event::MOUSE_BUTTON_UP :
      {
        CAST_PTR(mev, i4_mouse_button_up_event_class, ev);
        if (mev->but==i4_mouse_button_up_event_class::LEFT && mouse_down)
        {
          mouse_down=i4_F;
          i4_window_request_mouse_ungrab_class grab(this);
          i4_kernel.send_event(parent,&grab);
        }
      } break;
    }
  }
   
};



i4_button_class *g1_time_line::create_img_win(char *icon_res_name,
                                              w32 mess_id,
                                              i4_graphical_style_class *style)

{
  i4_image_class *im=i4_load_image(g1_editor_instance.get_editor_string(icon_res_name));
  I4_ASSERT(im,"icon missing");
  
  char help[30];
  sprintf(help,"%s_help",icon_res_name);

  i4_object_message_event_class *omes=new i4_object_message_event_class(this, mess_id);
  i4_event_reaction_class *press=new i4_event_reaction_class(this, omes);
  i4_button_class *b=new i4_button_class(&g1_ges(help),
                                         new i4_image_window_class(im, i4_T),
                                         style,
                                         press);
  return b;
}

void g1_time_line::create_time_win(i4_graphical_style_class *style)
{ 
  frame_show=new g1_frame_show(style, this);
  frame_show->update();

  i4_const_str next_str=g1_editor_instance.get_editor_string("next");
  i4_window_class *frame_text=new i4_text_window_class(next_str, style);

  sec_win=new g1_time_show(style, this);

  w32 w=frame_text->width()+10+frame_show->width();
  w32 h=frame_show->height() + 2 + sec_win->height();

  i4_deco_window_class *d=new i4_deco_window_class(w,h, i4_F, style);  
  time_win=d;

  w32 x1=d->get_x1(), y1=d->get_y1();

  time_win->add_child(x1,y1+2, frame_text);
  time_win->add_child(x1+frame_text->width()+5, y1, frame_show.get());
  time_win->add_child(x1,y1+frame_show->height()+1, sec_win.get());
}

g1_time_line::~g1_time_line()
{
  if (g1_frame_change_notify)
  {
    delete g1_frame_change_notify;
    g1_frame_change_notify=0;
  }
  
  if (g1_scene_change_notify)
  {
    delete g1_scene_change_notify;
    g1_scene_change_notify=0;
  }

  if (g1_movie_stop_notify)
  {
    delete g1_movie_stop_notify;
    g1_movie_stop_notify=0;
  }

  if (scroller.get())
      delete scroller.get();
      
  if (time_win.get())
    delete time_win.get();

  if (bbox.get())
    delete bbox.get();

  if (last_scene.get())
    delete last_scene.get();

  if (next_scene.get())
    delete next_scene.get();

  if (scene_number_input.get())
    delete scene_number_input.get();
}

g1_time_line::g1_time_line(i4_parent_window_class *parent, 
                           i4_graphical_style_class *style,
                           g1_edit_state_class *state)
  : state(state)
{
  i4_object_message_event_class *o;
  o=new i4_object_message_event_class(this,FRAME_CHANGED);
  g1_frame_change_notify=new i4_event_reaction_class(this, o);
                                                          
  o=new i4_object_message_event_class(this,SCENE_CHANGED);                            
  g1_scene_change_notify=new i4_event_reaction_class(this, o);

  o=new i4_object_message_event_class(this,MOVIE_STOPPED);
  g1_movie_stop_notify=new i4_event_reaction_class(this, o);

  bbox=new i4_button_box_class(this, i4_F);
  

  i4_button_class *left, *right, *rewind, *fforward;
  left=create_img_win("e_left", LAST_TIME, style);  
  left->set_repeat_down(i4_T);
  left->set_popup(i4_T);

  right=create_img_win("e_right", NEXT_TIME, style);  
  right->set_repeat_down(i4_T);
  right->set_popup(i4_T);

  play=create_img_win("e_play", PLAY, style);
  i4_user_message_event_class *stop=new i4_user_message_event_class(G1_STOP_MOVIE);
  play->send.depress=new i4_event_reaction_class(i4_current_app, stop);
                                                 

  rewind=create_img_win("e_rewind", REWIND, style);
  fforward=create_img_win("e_fforward", FFORWARD, style);

  bbox->add_button(0,0,rewind);
  bbox->add_button(0,0,left);
  bbox->add_button(0,0,play.get());
  bbox->add_button(0,0,right);
  bbox->add_button(0,0,fforward);

  bbox->arrange_down_right();

  h=bbox->height();
  

  create_time_win(style);
  if (time_win->height()>h)
    h=time_win->height();

  last_scene=create_img_win("e_left", LAST_SCENE, style);  
  last_scene->set_repeat_down(i4_T);
  last_scene->set_popup(i4_T);

  next_scene=create_img_win("e_right", NEXT_SCENE, style);  
  next_scene->set_repeat_down(i4_T);
  next_scene->set_popup(i4_T);
  
  i4_const_str fmt=g1_editor_instance.get_editor_string("frame_format");
  w32 scene_d=current_movie() ? current_movie()->get_scene()+1 : 1;
  i4_str *scene_number=fmt.sprintf(10, scene_d);
  scene_number_input=new i4_text_input_class(style,*scene_number, 40, 8, this);
  delete scene_number;

  sw32 xon=0;
  parent->add_child(xon, parent->height()-last_scene->height()-1, last_scene.get());
  xon+=last_scene->width();

  parent->add_child(xon, parent->height()-scene_number_input->height()-1, 
                    scene_number_input.get());
  xon+=scene_number_input->width();

  parent->add_child(xon, parent->height()-next_scene->height()-1, next_scene.get());
  xon+=next_scene->width();


  scroller=new g1_time_scroller(parent->width() - xon -
                                bbox->width() -
                                time_win->width(),
                                h, style, this);
  

  parent->add_child(xon, parent->height()-scroller->height()-1, scroller.get());
  xon+=scroller->width();

  parent->add_child(xon, parent->height()-bbox->height()-1, bbox.get());
  xon+=bbox->width();

  parent->add_child(xon, parent->height()-time_win->height()-1, time_win.get());
}


void g1_time_line::update()
{
  scroller->request_redraw(i4_F);
  frame_show->update();
  sec_win->request_redraw(i4_F);
}

i4_spline_class *g1_time_line::selected_spline()
{
  return 0;
}


g1_movie_flow_class *g1_time_line::current_movie()
{
  g1_get_current_movie_event gm;
  i4_kernel.send_event(i4_current_app, &gm);
  return gm.mflow;
}

void g1_time_line::set_current(w32 frame, i4_bool stop_play)
{  
  g1_movie_flow_class *m=current_movie();
  if (m)
  {
    g1_editor_instance.add_undo(G1_MAP_MOVIE);

    m->set_frame(frame);

    i4_user_message_event_class c(G1_MAP_CHANGED);
    i4_kernel.send_event(i4_current_app, &c);
    update();   
    
    if (stop_play)
      play->do_depress();
  }
}


w32 g1_time_line::current_frame()
{
  g1_movie_flow_class *m=current_movie();
  if (m)
    return m->get_frame();
  else
    return 0;
}

w32 g1_time_line::total_frames()
{
  g1_movie_flow_class *m=current_movie();
  if (m && m->current())
    return m->current()->total_frames();
  else return 0;
}

void g1_time_line::update_scene()
{
  g1_movie_flow_class *m=current_movie();

  i4_const_str fmt=g1_editor_instance.get_editor_string("frame_format");
  i4_str *scene_number=fmt.sprintf(10, m->get_scene()+1);
  scene_number_input->change_text(*scene_number);
  delete scene_number;
}

void g1_time_line::set_current_scene(sw32 scene)
{
  g1_editor_instance.add_undo(G1_MAP_MOVIE);

  g1_movie_flow_class *m=current_movie();
  m->set_scene(scene);
  m->set_frame(0);

  update_scene();

  i4_user_message_event_class ch(G1_MAP_CHANGED);
  i4_kernel.send_event(i4_current_app, &ch);
}

void g1_time_line::unpress_play()
{
  if (play.get())
    play->do_depress();
}

void g1_time_line::receive_event(i4_event *ev)
{
  g1_movie_flow_class *m=current_movie();

  CAST_PTR(oev, i4_object_message_event_class, ev);

  if (!m || oev->type()!=i4_event::OBJECT_MESSAGE)
    return ;

  if (oev->object==this)
  {
    switch (oev->sub_type)
    {
      case LAST_TIME :
        if (m->get_frame())
          set_current(m->get_frame()-1, i4_T);
        break;

      case NEXT_TIME :
        if (m->current() && m->get_frame()+1<m->current()->total_frames())          
          set_current(m->get_frame()+1, i4_T);          
        break;

      case FFORWARD :
        if (m->current())
          set_current(m->current()->total_frames()-1, i4_T);
        break;

      case REWIND :
        set_current(0, i4_T);
        break;

      case PLAY :
      {
        i4_user_message_event_class m(G1_PLAY_MOVIE);
        i4_kernel.send_event(i4_current_app, &m);
      } break;

      case LAST_SCENE :      
        if (m->current() && m->get_scene())
         set_current_scene(m->get_scene()-1);
        break;

      case NEXT_SCENE :
        if (m->current() && m->get_scene()<m->t_cut_scenes-1)
         set_current_scene(m->get_scene()+1);
        break;        

      case SCENE_CHANGED :
        update_scene();
        break;

      case FRAME_CHANGED :
        update();
        break;

      case MOVIE_STOPPED :
        unpress_play();
        break;
    }
  } else if (oev->object==scene_number_input.get())
  {
    CAST_PTR(tc, i4_text_change_notify_event, ev);
    i4_const_str::iterator i=tc->new_text->begin();
    sw32 n=i.read_number()-1;
    if (n>=0 && n<m->t_cut_scenes)
      set_current_scene(n);
  }
}
