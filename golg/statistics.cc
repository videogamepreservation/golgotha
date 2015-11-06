/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "statistics.hh"
#include "window/window.hh"
#include "window/style.hh"
#include "font/font.hh"
#include "error/error.hh"
#include "time/timedev.hh"
#include "app/app.hh"

static i4_event_handler_reference_class<i4_parent_window_class> g1_stat_win;

class g1_stat_win_class : public i4_window_class
{
  int t_strings;  
  i4_graphical_style_class *style;
  i4_time_device_class::id poll_id;

public:
  int x2() { return 50; }
  g1_stat_win_class(i4_graphical_style_class *style)
    : i4_window_class(0,0), style(style)
      
  {  
    int min_w=0, min_h=0;
    t_strings=0;
    i4_font_class *fnt=style->font_hint->small_font;

    min_w=fnt->width("Object polygons");
    min_h=fnt->largest_height() * g1_statistics_counter_class::LAST;

    min_h+=fnt->largest_height();   // save room for fps (first line)
    min_w+=x2();                 // add room for numbers

    resize(min_w, min_h);

    i4_user_message_event_class poll(0);
    poll_id=i4_time_dev.request_event(this, &poll, 2000);  // update once every 2 secs
  }

  void draw_float(int x, int y, float v, i4_font_class *fnt, i4_draw_context_class &context)
  {
    char buf[30];
    int ipart, fpart;
    ipart=(int)v;
    fpart=(int)(v * 10) - ipart*10;
    
    sprintf(buf, "%d.%d", ipart, fpart);

    for (char *c=buf; *c; c++)
    {
      i4_char ch(*c);
      fnt->put_character(local_image, x,y, ch, context);
      x+=fnt->width(ch);
    }   
  }

  void draw(i4_draw_context_class &context)
  {
    local_image->clear(0, context);

    if (g1_stat_counter.t_frames)
    {      
      i4_font_class *fnt=style->font_hint->small_font;
      fnt->set_color(0xffff00);

      float oo_tframes = 1.0/(float)g1_stat_counter.t_frames;
      
      i4_time_class now;
      draw_float(0,0, (float)g1_stat_counter.t_frames / 
                 (now.milli_diff(g1_stat_counter.last_update_time)/1000.0),
                 fnt, context);

      fnt->put_string(local_image, x2(), 0, i4gets("fps"), context);
      int y=fnt->largest_height();

      char *strs[]=
      {"Polys",
       "Object polys",
       "Terrain polys",
       "Objects",
       "Terrain",
       "Sprites",
       "Sfx mixed",
       "Demo Frames"};

      for (int i=0; i<g1_statistics_counter_class::LAST; i++)
      {
        draw_float(0, y, g1_stat_counter.counter_array[i] * oo_tframes, fnt, context);
        fnt->put_string(local_image, x2(), y, strs[i], context);
        y+=fnt->height(strs[i]);
      }
    }

    g1_stat_counter.reset();

  }

  void receive_event(i4_event *ev)
  {
    if (ev->type()==i4_event::USER_MESSAGE)  // update statics
    {
      request_redraw();      
      i4_user_message_event_class poll(0);
      poll_id=i4_time_dev.request_event(this, &poll, 2000);  // update once every 2 secs
    }
  }

  ~g1_stat_win_class()
  {
    i4_time_dev.cancel_event(poll_id);
  }

  char *name() { return "stat_win"; }
};

void g1_statistics_counter_class::show()
{
  if (!g1_stat_win.get())
  {
    i4_graphical_style_class *style=i4_current_app->get_style();
    i4_window_class *swin = new g1_stat_win_class(style);

    g1_stat_win = style->create_mp_window(-1, -1, swin->width(), swin->height(),
                                          i4gets("stat_win_title"));

    g1_stat_win->add_child(0, 0, swin);
  }  
}
