/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "window/window.hh"
#include "window/style.hh"
#include "window/win_evt.hh"
#include "device/event.hh"
#include "time/timedev.hh"
#include "time/profile.hh"
#include "time/profile_stack.hh"

//2345678*012345678*012345678*012345678*012345678*012345678*012345678*012345678*012345678*012345678*
i4_profile_class pf_profile_draw("profile window refresh");

// sort in descending order
static int profile_compare(const void *a, const void *b)
{
  i4_profile_class *ap=*((i4_profile_class **)a);
  i4_profile_class *bp=*((i4_profile_class **)b);

  if (ap->total_time<bp->total_time)
    return 1;
  else if (ap->total_time>bp->total_time)
    return -1;
  else return 0;               
}



enum { CLOSE_WIN, POLL };
class i4_prof_win_class : public i4_parent_window_class
{
  i4_bool update_stats;
  i4_graphical_style_class *style;
  i4_time_device_class::id poll_id;
  i4_event_reaction_class *on_close;
  w64 last_clock;

public:
  i4_prof_win_class(w16 w, w16 h, i4_graphical_style_class *style,
                    i4_event_reaction_class *on_close)
    : i4_parent_window_class(w,h), 
      style(style),
      on_close(on_close)
  {    
    i4_object_message_event_class poll(this, POLL);
    poll_id=i4_time_dev.request_event(this, &poll, 5000);  // update once every 5 secs

    last_clock=i4_get_system_clock();

      
    // count how many profile classes are in memory
    for (i4_profile_class *p=i4_profile_class::list; p; p=p->next)
      p->total_time=0;

    i4_profile_on=1;
  }

  void parent_draw(i4_draw_context_class &context)
  {
    int t=0,i=0;
    double total_time_measured=0.0;
    i4_profile_class *p=i4_profile_class::list;
    // count how many profile classes are in memory
    for (; p; p=p->next)
    {
      total_time_measured+=(double)(sw64)p->total_time;
      t++;
    }

    pf_profile_draw.start();

    local_image->clear(0, context);
    if (update_stats)
    {
      // put them all in a list and sort them by clocks
      i4_profile_class **plist;
      plist=(i4_profile_class **)i4_malloc(sizeof(i4_profile_class *)*t,"pl");
      for (p=i4_profile_class::list; p; p=p->next)
      {        
        plist[i]=p;
        i++;
      }
      qsort(plist, t, sizeof(i4_profile_class *), profile_compare);

      
      char buf[200];

      int y=0, h=height();
      int x=50;

      w64 current_clock = i4_get_system_clock();
      double oo_total = 1.0/(double)((sw64)current_clock - (sw64)last_clock);
      last_clock=i4_get_system_clock();    

      i4_font_class *fnt=style->font_hint->small_font;
      fnt->set_color(0x0000ff);   
      sprintf(buf,"%2.2f Total", ((double)(sw64)total_time_measured * oo_total) * 100.0);
      fnt->put_string(local_image, x+1, y, buf, context);
      y+=fnt->height(buf);


      fnt->set_color(0xffff00);



      for (i=0; i<t; i++)
      {
        double percent = (sw64)plist[i]->total_time * oo_total;

        sprintf(buf, "%2.2f %s", percent * 100.0, plist[i]->name);

        int th=fnt->height(buf);
        local_image->bar(x-(sw32)(percent * x), y, x, y+th-1, 0xffff, context);

        plist[i]->total_time=0;
        fnt->put_string(local_image, x+1, y, buf, context);
        
        y+=th;
        if (y>h)
          break;
      }

      i4_free(plist);
      update_stats=i4_F;
    } 
    else last_clock=i4_get_system_clock();    

    pf_profile_draw.stop();
  }

  void receive_event(i4_event *ev); 

  ~i4_prof_win_class()
  {
    if (on_close)
      delete on_close;
    i4_time_dev.cancel_event(poll_id);

    i4_profile_stack_top=0;
    i4_profile_on=0;
  }

  char *name() { return "profile window"; }
};

i4_parent_window_class *i4_prof_win=0;

void i4_prof_win_class::receive_event(i4_event *ev)
{
  CAST_PTR(oev, i4_object_message_event_class, ev);

  if (ev->type()==i4_event::OBJECT_MESSAGE && oev->object==this)
  {
    if (oev->sub_type==CLOSE_WIN)     // close window
    {
      i4_prof_win=0;
      
      i4_kernel.send(on_close);
    }
    else if (oev->sub_type==1)  // update statics
    {
      update_stats=i4_T;
      request_redraw();
      
      i4_object_message_event_class poll(this, POLL);
      poll_id=i4_time_dev.request_event(this, &poll, 5000);  // update once every 5 secs
    }
  }
  else i4_parent_window_class::receive_event(ev);
}

void i4_profile_watch(i4_graphical_style_class *style,
                      i4_parent_window_class *parent,                      
                      sw32 &win_x, sw32 &win_y,
                      w32 w, w32 h,
                      int open_type,  // 0==close, 1==open, 2==toggle window
                      i4_event_reaction_class *on_close)
{
  if (i4_prof_win && (open_type==0 || open_type==2))
  {
    win_x=i4_prof_win->get_parent()->x();
    win_y=i4_prof_win->get_parent()->y();

    style->close_mp_window(i4_prof_win);
    i4_prof_win=0;
  }
  else if (!i4_prof_win && (open_type==1 || open_type==2))
  {
    i4_prof_win_class *p=new i4_prof_win_class(w,h, style, on_close);

    i4_event_reaction_class *re;
    re=new i4_event_reaction_class(p, new i4_object_message_event_class(p,CLOSE_WIN,i4_event::NOW));

    i4_prof_win=style->create_mp_window(win_x, win_y, w,h, 
                                        i4gets("prof_win_title",i4_F), re);
    i4_prof_win->add_child(0,0,p);
  }

}
