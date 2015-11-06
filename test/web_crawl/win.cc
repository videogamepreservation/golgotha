/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "network/net_prot.hh"

#include "win.hh"
#include "threads/threads.hh"
#include "fb_thread.hh"
#include "font/font.hh"
#include "gui/button.hh"
#include "gui/text.hh"
#include "url.hh"
#include "network/net_sock.hh"
#include "lisp/lisp.hh"
#include <ctype.h>


char *fb_thread_window::get_error_string()
{
  switch (last_error)
  {
    case FB_NO_ERROR : return "ok"; break;
    case FB_TIMED_OUT : return "timeout"; break;
    case FB_URL_TOO_BIG : return "url too big"; break;
    case FB_END_OF_STREAM : return "end of stream"; break;
    case FB_NOT_IN_INCLUDE_LIST : return "not in include list"; break;
    case FB_NO_EXTENSION : return "no file extension"; break;
    case FB_NO_FILENAME : return "no filename"; break;
    case FB_FILE_EXSIST : return "file exsist"; break;
    case FB_NO_WRITE_OPEN : return "couldn't open write file"; break;
    case FB_IN_EXCLUDE_LIST : return "in exclusion list"; break;
    case FB_NO_CONNECT : return "connect failed"; break;
    case FB_USER_ABORTED : return "user aborted"; break;
    default : return "unknown error"; break;
  }
  last_error=FB_NO_ERROR;
}
    


void fb_thread_window::stop_thread()
{
  if (state!=FB_WAITING && state!=FB_DONE_READING)
    stopping=1;
      
  while (state!=FB_WAITING && state!=FB_DONE_READING)
    i4_thread_yield();
}

void fb_thread_window::add_button(char *name, cmd_type cmd)
{
  i4_text_window_class *tw=new i4_text_window_class(name, style,
                                                    style->font_hint->small_font);

  i4_button_class *b;
  b=new i4_button_class(0, tw, style, new i4_event_reaction_class(this, cmd));
  b->set_popup(i4_T);
  button_x-=(int)b->width();
  add_child(button_x, 0, b);  
}




void fb_thread_window::read_data()
{
  w8 packet[1024];                
  int s=sock->read(packet,1024);
  last_read.get();
     
  if (s>0)    // read 0 or less, assume end of stream
  {    
    if (save_file)
    {
      save_file->write(packet, s);
      save_buffer_size+=s;
    }
    else
    {    
      // we are going to overflow our buffer
      if (save_buffer_size+s > FB_MAX_SAVE_BUFFER_SIZE)
      {
        last_error=FB_URL_TOO_BIG;
        state=FB_DONE_READING;  // tell main program to process what we read of url
      }
      else
      {
        memcpy(save_buffer + save_buffer_size, packet, s);
        save_buffer_size+=s;
      }
    }    
  }
  else
  {
    last_error=FB_END_OF_STREAM;

    if (save_file)
    {
      close_save_file(i4_F);
      state=FB_WAITING;         // ready for another request
      save_buffer_size=0;
    }
    else
    {
      state=FB_DONE_READING;    // tell main program to process this url
      
    }
    
    if (sock)
    {
      delete sock;
      sock=0;
    }
  }
}

char *nocase_strstr(char *hay, char *needle)
{
  while (*hay)
  {
    char *a1, *a2;
    int not_equal=0;
    
    for (a1=hay, a2=needle; *a1 && *a2; )
    {
      if (toupper(*a1)!=toupper(*a2))
        not_equal=1;
      a1++;
      a2++;
    }

    if (*a2) not_equal=1;

    if (!not_equal)
      return hay;
    
    hay++;
  }
  return 0;
}

void fb_thread_window::set_url(fb_url *new_url)
{
  stop_thread();
  save_buffer_size=0;
  

  close_save_file();
    
  if (sock)
  {
    delete sock;
    sock=0;
  }
    
    if (url)
      delete url;
  
  url=new_url;
  
  li_object *o;
  
  for (o=li_get_value("exclude_sub_strings",0); o; o=li_cdr(o,0))
    if (nocase_strstr(url->full_name, li_string::get(li_car(o,0),0)->value()))
    {
      last_error=FB_IN_EXCLUDE_LIST;
      state=FB_DONE_READING;
      return ;
    }

  int save=0;
  char *ext=url->get_extension();      
  if (ext)
  {
  
    o=li_get_value("include_sub_strings");
    if (o)
    {
      int ok=0;
    
      while (o && !ok)
      {
        li_object *i=li_car(o,0);
        if (fb_strneq( li_get_string(li_car(i,0),0), ext, strlen(ext)))
        {
          i=li_cdr(i,0);
          if (!i)
            ok=1;
          else
          {
            for (i=li_cdr(i,0); i; i=li_cdr(i,0))
              if (nocase_strstr(url->full_name, li_string::get(li_car(i,0),0)->value()))
                ok=1;

            if (!ok)
              i4_debug->printf("not listed '%s'\n", url->full_name);

          }
        }
        o=li_cdr(o,0);
      }

      int l=strlen(url->full_name)-1;
      if (!ok && url->full_name[l]!='/' && url->full_name[l]!='\\')
      {
        last_error=FB_NOT_IN_INCLUDE_LIST;
        state=FB_DONE_READING;
        return ;
      }
    }

    for (o=li_get_value("save_extensions",0); o; o=li_cdr(o,0))
      if (strcmp(li_string::get(li_car(o,0),0)->value(), ext)==0)
        save=1;

  }
    


  if (save)
  {
    char *fn=url->get_filename();      
    if (!fn)
    {
      last_error=FB_NO_FILENAME;
      state=FB_DONE_READING;
      return ;
    }

    char filename[200];
    sprintf(filename, "%s%s", li_string::get(li_get_value("save_path",0),0)->value(), fn);

    i4_file_status_struct stat;
    
    if (i4_get_status(filename, stat))
    {
      last_error=FB_FILE_EXSIST;
      state=FB_DONE_READING;
      return ;
    }
      
    save_file=i4_open(filename, I4_WRITE);
    if (!save_file)
    {
      last_error=FB_NO_WRITE_OPEN;
      state=FB_DONE_READING;
      return ;
    }
  }

  state=FB_CONNECTING;        // let the thread do the nameserver lookup
}


void fb_thread_window::receive_event(i4_event *ev)
{
  if (ev->type()==i4_event::USER_MESSAGE)
  {
    switch (((i4_user_message_event_class *)ev)->sub_type)
    {
      case PAUSE :
        if (state==FB_READING)  
          state=FB_SUSPENDED_READ;
        else if (state==FB_SUSPENDED_READ)
          state=FB_READING;
        break;
        
      case ABORT :
        stopping=1;
        break;
        
      case BAN_DIR :
      case BAN_SITE :
        break;
    }
  }
  
  i4_parent_window_class::receive_event(ev);
}

fb_thread_window::fb_thread_window(w16 w, w16 h, i4_graphical_style_class *style)
  : i4_parent_window_class(w,h), style(style)
{
  stopping=0;
  prot=i4_get_typed_protocol(I4_TCPIP);
  if (!prot) i4_error("no TCPIP in program");
  
  save_buffer_size=0;
  sock=0;
  save_file=0;
  url=0;

  last_error=FB_NO_ERROR;
  state=FB_THREAD_INIT;
  i4_add_thread(fb_thread, 100*1024, this);
  
  button_x=width();
  
  add_button("Pause", PAUSE);
  add_button("Abort", ABORT);
  add_button("Ban Dir", BAN_DIR);
  add_button("Ban Site", BAN_SITE);  
}

// called from main program to see if window needs to be redrawn
void fb_thread_window::update()  // check to see if we need to update the window
{
  i4_time_class now;
  
  if (now.milli_diff(last_refresh)>200)    // refresh every .2 secs
  {
    if (last_state!=state ||
        last_save_buffer_size!=save_buffer_size)      
      request_redraw(i4_F);
  } 
}


// called by main program when window some time after window calls request_redraw
void fb_thread_window::parent_draw(i4_draw_context_class &context)
{
  style->deco_neutral_fill(local_image, 0,0, width()-1, height()-1, context);

  i4_font_class *font=style->font_hint->small_font;
  i4_font_class *bfont=style->font_hint->normal_font;
  
  char *state_names[]={"Thread Init",
                       "Idle",
                       "Connecting",
                       "Reading",
                       "Paused",
                       "Done Reading",
                       "Stopping",
                       "Quiting",
                       "Done"};

  bfont->set_color(0xffff00);

  int x=0, y=0;
  bfont->put_string(local_image, x,y, state_names[state], context);
  y+=bfont->height(state_names[state])+4;

  if (url && state==FB_CONNECTING || state==FB_READING ||
      state==FB_DONE_READING || state==FB_SUSPENDED_READ)
  {
    font->set_color(0x7f);
    font->put_string(local_image, x,y, url->full_name, context);
  }
  y+=font->largest_height();


  if (state==FB_READING || state==FB_DONE_READING || state==FB_SUSPENDED_READ)
  {
    char buf[100];
    font->set_color(0x7f0000);
    sprintf(buf, "%d bytes read", save_buffer_size);
    font->put_string(local_image, x,y, buf, context);
  }

  last_state=state;
  last_save_buffer_size=save_buffer_size;
  
  last_refresh.get();
  
}

// waits for thread to stop before closing the window
fb_thread_window::~fb_thread_window()
{
  state=FB_THREAD_QUITING;
  while (state!=FB_THREAD_DONE)
    i4_thread_yield();
}


void fb_thread_window::check_for_timeout()
{ 
  i4_time_class now;
  int secs=now.milli_diff(last_read)/1000;  // time in secs
  
  if (secs!=last_sec)
  {
    last_sec=secs;
        
    for (li_object *o=li_get_value("timeouts",0); o; o=li_cdr(o,0))
    {
      int size=li_int::get(li_first(li_car(o,0),0),0)->value();
      int tout=li_int::get(li_second(li_car(o,0),0),0)->value();

      
      if (save_buffer_size<size && secs>tout)
      {
        last_error=FB_TIMED_OUT;
        state=FB_DONE_READING;
        return ;
      }
    }
  }
}

 // closes save file if need, and deletes it if it's too small
void fb_thread_window::close_save_file(int wait_on_thread)
{
  if (save_file)
  {
    if (wait_on_thread)
      stop_thread();
    
    int need_del = save_file->tell() < li_int::get(li_get_value("save_min_size",0),0)->value() ? 1 : 0;
      
    delete save_file;
    save_file=0;

    if (url && need_del)
    {
      char filename[200];
      sprintf(filename, "%s%s", li_string::get(li_get_value("save_path",0),0)->value(),
              url->get_filename());
      i4_unlink(filename);
    }
  }
}
