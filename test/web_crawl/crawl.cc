/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "window/window.hh"
#include "app/app.hh"
#include "main/main.hh"
#include "window/style.hh"
#include "file/file.hh"
#include "loaders/load.hh"
#include "window/wmanager.hh"
#include "math/transform.hh"
#include "gui/text.hh"
#include "gui/button.hh"
#include "gui/image_win.hh"
#include "loaders/load.hh"

#include "window/colorwin.hh"
#include "gui/li_pull_menu.hh"
#include "menu/pull.hh"
#include "font/anti_prop.hh"
#include "lisp/lisp.hh"
#include "win.hh"
#include "url.hh"
#include "memory/array.hh"
#include "checksum/checksum.hh"
#include "string/string.hh"
#include <ctype.h>

enum { FB_MAX_THREADS=100 };
i4_event_handler_reference_class<fb_thread_window> fb_thread_windows[FB_MAX_THREADS];

li_object *fb_add_thread(li_object *o, li_environment *env=0)
{
  for (int i=0; i<FB_MAX_THREADS; i++)
  {
    if (!fb_thread_windows[i].get())
    {      
      i4_graphical_style_class *style=i4_current_app->get_style();
 
      fb_thread_window *fb=new fb_thread_window(300, 35, style);

      i4_parent_window_class *mp;
  
      if (o)  
        mp=style->create_mp_window(li_int::get(li_first(o,0),0)->value(),
                                   li_int::get(li_second(o,0),0)->value(),
                                   fb->width(), fb->height(), "Thread");
      else
        mp=style->create_mp_window(30,30, fb->width(), fb->height(), "Thread");

      mp->add_child(0,0, fb);
      fb_thread_windows[i]=fb;
      
      return li_true_sym;
    }
  }
  return 0;
}

li_object *fb_add_page(li_object *o, li_environment *env);
li_object *fb_visited(li_object *o, li_environment *env);
                       

i4_event_handler_reference_class<i4_text_window_class> fb_counts_window;

class fb_app : public i4_application_class
{
public:
  enum { QUIT };


  void init()
  {        
    i4_application_class::init();
        
    i4_graphical_style_class *style=get_style();

    i4_parent_window_class *cw=i4_add_color_window(wm, 0, style, 0,0, wm->width(), wm->height());

    i4_image_class *logo=i4_load_image("logo.jpg");

    cw->add_child(wm->width()/2-logo->width()/2,
                  wm->height()/2-logo->height()/2,
                  new i4_image_window_class(logo, i4_T, i4_F));

    style->icon_hint->background_bitmap=i4_load_image("background.tga");
    
    fb_counts_window=new i4_text_window_class("Visited 000000, Unvisited 000000",
                                              style, style->font_hint->small_font);
    
    wm->add_child(wm->width()-fb_counts_window->width(), wm->height()-fb_counts_window->height(),
              fb_counts_window.get());

    li_create_pull_menu("menu.scm")->show(cw, 0,0);
        
    li_add_function("add_thread", fb_add_thread);
    li_add_function("add_page", fb_add_page);
    li_add_function("visited", fb_visited);

    i4_file_class *fp=i4_open("filebot.scm");
    if (!fp)
    {
      fp=i4_open("default.scm");
      if (!fp)
        i4_error("no default?");
      int size=fp->size();
      i4_file_class *out=i4_open("filebot.scm",I4_WRITE);
      if (!out) i4_error("couldn't write to filebot.scm");
      char buf[2000];
      fp->read(buf, size);
      out->write(buf,size);
      delete fp;
      delete out;
      
    } else delete fp;
    
    li_load("filebot.scm");
    
    i4_mkdir(li_string::get(li_get_value("save_path",0),0)->value());    
  }  
  
  i4_array<w32> visited_urls;
  i4_array<fb_url *> urls_to_visit;

  fb_app()
    : visited_urls(0, 1024),
      urls_to_visit(0, 1024)
  {
  }

  void uninit()
  {
    visited_urls.uninit();        // free up memory associated with the arrays
    urls_to_visit.uninit();
    i4_application_class::uninit();
  }


  fb_url *get_best_url()
  {
    int i;
    for (i=0; i<urls_to_visit.size(); i++)
      if (urls_to_visit[i])
      {
        char *name=urls_to_visit[i]->full_name;
        if (name[strlen(name)-1]=='3' && (strstr(name, ".mp3") || strstr(name, ".MP3")))
        {
          fb_url *ret=urls_to_visit[i];
          urls_to_visit[i]=0;
          return ret;
        }
      }

        
    for (i=0; i<urls_to_visit.size(); i++)
      if (urls_to_visit[i])
      {
        char *name=urls_to_visit[i]->full_name;
        if (strstr(name, "mp3") || strstr(name, "MP3"))
        {
          fb_url *ret=urls_to_visit[i];
          urls_to_visit[i]=0;
          return ret;
        }
      }


    for (i=0; i<urls_to_visit.size(); i++)
      if (urls_to_visit[i])
      {
        fb_url *ret=urls_to_visit[i];
        urls_to_visit[i]=0;
        return ret;
      }

    return 0;
  }
  
  void add_new_url(char *uname, fb_url *current_url)
  {
    fb_url *url_copy=new fb_url(uname, current_url);
    
    // see if we have visited this site already.  Instead of storing the whole string
    // for places we've been we store the checksum of the name
    w32 csum=url_copy->checksum();

    int i;
    for (i=0; i<visited_urls.size(); i++)
      if (visited_urls[i]==csum)
        return;                             // we've already been there

    for (i=0; i<urls_to_visit.size(); i++)
      if (!urls_to_visit[i])
      {
        urls_to_visit[i]=url_copy;
        return ;
      }
    
    urls_to_visit.add(url_copy);

  }

  
  void scan_url_data(char *s, int len, fb_url *current_url)
  {
    len-=10;            // need this much space to have a href
    char uname[400];   // store temporary hrefs here
        
    for (int i=0; i<len;)
    {
      // is this a href?
      if (fb_strneq(s+i, "<a href=", 8)==1)
      {
        i+=8;
        while (s[i] && s[i]!='"') i++;      // find quote
        i++;                                // skip it


        char *up=uname;                     // save the rest of the href
        int t=0;
        while (s[i] && s[i]!='"')
        {
          t++;
          if (t==398) s[i]=0;
          else
            *(up++)=s[i++];
            
        }
        *up=0;
        i++;


        add_new_url(uname, current_url);
      }
      else i++;
    }
  }

  
  void calc_model()
  {
    for (int i=0; i<FB_MAX_THREADS; i++)
    {
      if (fb_thread_windows[i].get())
      {
        int state=fb_thread_windows[i]->get_state();
        if (state==FB_DONE_READING)       // has url data it wan't us to process
        {
          w8 *buf=fb_thread_windows[i]->save_buffer;
          int size=fb_thread_windows[i]->save_buffer_size;
          fb_url *url=fb_thread_windows[i]->get_url();


          scan_url_data((char *)buf, size, url);   // scan buffer for new url's to follow

          i4_file_class *fp=i4_open("visited_urls.scm", I4_APPEND);
          if (fp)
          {
            fp->printf("(visited \"%s\") ; %d %s \n", url->full_name, size,
                       fb_thread_windows[i]->get_error_string());
            delete fp;
          }
            
          
          // reset this thread
          fb_thread_windows[i]->save_buffer_size=0;
          state=FB_WAITING;                  // ready to accept new commands
          fb_thread_windows[i]->ack_data();  // tell window we are done with it's data

        }

        if (state==FB_WAITING)     // is the thread ready for a new request?
        {
          fb_url *best=get_best_url();
          if (best)
          {
            
            visited_urls.add(best->checksum());
            fb_thread_windows[i]->set_url(best);
          }
        }

        
        fb_thread_windows[i]->update();
        
      }

      
    }

    
    if (fb_counts_window.get())
    {
      int t=0;
      for (int i=0; i<urls_to_visit.size(); i++)
        if (urls_to_visit[i]) t++;
      
      char buf[100];
      sprintf(buf, "Visited %d, Unvisited %d", visited_urls.size(), t);
      fb_counts_window->set_text(new i4_str(buf));
    }

  }

  void receive_event(i4_event *ev)   
  {
  
    if (ev->type()==i4_event::DO_COMMAND)
    {
      char *cmd=((i4_do_command_event_class *)ev)->command;
      if (cmd)
      {
        if (strcmp(cmd, "exit")==0)
          quit();
        else
          li_call(cmd);
      }
    }        
  }

  char *name() { return "fb_app"; }
} *file_bot;

li_object *fb_visited(li_object *o, li_environment *env)
{
  char *s=li_string::get(li_first(o,0),0)->value();
  w32 checksum=i4_check_sum32(s,strlen(s));   
  file_bot->visited_urls.add(checksum);

  return 0;
}

li_object *fb_add_page(li_object *o, li_environment *env)
{
  file_bot->add_new_url(li_string::get(li_first(o,0),0)->value(), 0);
  return 0;
}

void i4_main(w32 argc, i4_const_str *argv)
{
  fb_app f;
  file_bot=&f;
  file_bot->run();
}


