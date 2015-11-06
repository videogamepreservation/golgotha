/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "window/style.hh"
#include "gui/text_input.hh"
#include "gui/text.hh"
#include "g1_speed.hh"
#include "window/colorwin.hh"
#include "editor/editor.hh"
#include "editor/e_res.hh"

class g1_time_edit_window : public i4_color_window_class
{
  i4_text_input_class *si, *mi;
  w32 t_sec, t_msec;

public:
  char *name() { return "time edit"; }

  g1_time_edit_window(i4_graphical_style_class *style, w32 cur_frame)
    : i4_color_window_class(0,0,style->color_hint->neutral(), style)
  {
    i4_text_window_class *st,*mt;
    i4_str *sec_str, *msec_str;

    st=new i4_text_window_class(g1_ges("sec"),style);
    mt=new i4_text_window_class(g1_ges("msec"),style);

    t_sec=cur_frame/G1_MOVIE_HZ;
    t_msec=(cur_frame-t_sec*G1_MOVIE_HZ)*60/G1_MOVIE_HZ;


    sec_str=g1_ges("sec_fmt").sprintf(10,t_sec);
    si=new i4_text_input_class(style, *sec_str, 40, 8, this);

    msec_str=g1_ges("msec_fmt").sprintf(10,t_msec);
    mi=new i4_text_input_class(style, *msec_str, 40, 8, this);

    delete sec_str;
    delete msec_str;

    add_child(0,3,st);
    add_child(st->width(),0,si);

    add_child(st->width() + si->width(),3,mt);
    add_child(st->width() + si->width() + mt->width(),0,mi);

    resize_to_fit_children();
  }

  void change_time()
  {      
    i4_spline_class *s[g1_controller_edit_class::MAX_SPLINE_EDIT];
    int t=g1_editor_instance.get_current_splines(s,g1_controller_edit_class::MAX_SPLINE_EDIT);
    
    int nf=t_sec*G1_MOVIE_HZ+t_msec*G1_MOVIE_HZ/60;

    for (int i=0; i<t; i++)
    {
      i4_spline_class::point *sp=s[i]->begin(), *last=0;
      for (;sp;sp=sp->next)
      {
        if (sp->selected)
          if (sp->frame<nf || (!last || last->frame<nf))
          {
            int advance=nf-sp->frame;

            for (i4_spline_class::point *q=sp; q; q=q->next)
              q->frame+=advance;
          }            
        last=sp;
      }
    }     
  }

  void receive_event(i4_event *ev)
  {
    CAST_PTR(tev, i4_text_change_notify_event, ev);

    if (tev->type()==i4_event::OBJECT_MESSAGE)
    {
      if (tev->object==si)
      {
        i4_const_str::iterator i=tev->new_text->begin();
        t_sec=i.read_number();
        change_time();
      }

      if (tev->object==mi)
      {
        i4_const_str::iterator i=tev->new_text->begin();
        t_msec=i.read_number();
        change_time();
      }
    }
    i4_color_window_class::receive_event(ev);
  }
};

i4_parent_window_class *g1_create_time_edit_window(i4_graphical_style_class *style,
                                                   w32 cur_frame)
{
  return new g1_time_edit_window(style, cur_frame);
}
