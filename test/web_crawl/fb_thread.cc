/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "win.hh"
#include "network/net_sock.hh"
#include "url.hh"
#include "threads/threads.hh"

void fb_thread(void *context)
{
  fb_thread_window *w=(fb_thread_window *)context;
  w->state=FB_WAITING;


  while (w->state!=FB_THREAD_QUITING)
  {
    if (w->stopping)
    {
      w->stopping=0;
      w->last_error=FB_USER_ABORTED;
      w->state=FB_DONE_READING;
    }
    
    switch (w->state)
    {
      case FB_CONNECTING :
      {
        if (w->sock)
          delete w->sock;

        w->sock=w->url->connect_to_server(w->prot);
        
        if (w->sock)
          w->state=FB_READING;
        else
        {
          w->last_error=FB_NO_CONNECT;
          w->state=FB_DONE_READING;
        }
        
        w->last_read.get();
        
      } break;
        
      case FB_READING :
      {
        if (!w->sock)
          w->state=FB_WAITING;
        else
        {
          if (w->sock->ready_to_read())
            w->read_data();
          else
            w->check_for_timeout();          
        }
      } break;



      case FB_SUSPENDED_READ :
        w->last_read.get();      // intentional no-break
      default:
        i4_thread_yield();
        break;
    }
  }
  
  w->state=FB_THREAD_DONE;
}

