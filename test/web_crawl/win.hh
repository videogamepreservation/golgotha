/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef FB_THREAD_WINDOW_HH
#define FB_THREAD_WINDOW_HH


#include "window/window.hh"
#include "time/time.hh"

enum fb_state
{ 
  FB_THREAD_INIT,       // starts in this state until thread goes to WATING state
  FB_WAITING,           // waiting for a url from main program, ready to 
  
  FB_CONNECTING,        // threads sets this state when a new url appears
  FB_READING,           // threads sets this state after connect and reading
  FB_SUSPENDED_READ,    // main program sets this state to pause the thread
  FB_DONE_READING,      // thread sets this state after it has read all of it's data

  FB_THREAD_QUITING,    // state is set when window is closing, thread will set to DONE
  FB_THREAD_DONE        // set by thread when the it sees a QUITING, then thread exits

};

enum fb_error
{
  FB_NO_ERROR,
  FB_TIMED_OUT,
  FB_URL_TOO_BIG,
  FB_END_OF_STREAM,
  FB_NOT_IN_INCLUDE_LIST,
  FB_NO_EXTENSION,
  FB_NO_FILENAME,
  FB_FILE_EXSIST,
  FB_NO_WRITE_OPEN,
  FB_IN_EXCLUDE_LIST,
  FB_NO_CONNECT,
  FB_USER_ABORTED
};

class i4_file_class;              // defined in file/file.hh
class i4_graphical_style_class;   // defined in window/style.hh
class i4_net_protocol;            // defined in network/net_prot.hh
class i4_net_socket;              // defined in network/net_sock.hh
struct fb_url;                    // defined in fb_url.hh

// urls that are bigger than this size are chopped
enum { FB_MAX_SAVE_BUFFER_SIZE=400*1024 };

class fb_thread_window : public i4_parent_window_class
{
  friend void fb_thread(void *context);
  
  void freeup_buffer();
  i4_graphical_style_class *style;

  int last_sec;         // last timeout checked
  int button_x;         // when we add a new button add it to the left of this position

  // commands that a button will send to this window
  enum cmd_type { PAUSE, ABORT, BAN_DIR, BAN_SITE };
  void add_button(char *name, cmd_type cmd);

  i4_net_protocol *prot;    
  fb_url *url;              // url this crawl thread is working on

  int last_error;
  fb_state state, last_state; // what is thread doing currently?
  int stopping;

  i4_time_class last_refresh;  // don't redraw too much, here's the last time we redrew
  i4_time_class last_read;     // time thread last got data, will time-out if too high
  i4_net_socket *sock;
  i4_file_class *save_file;    // if we are downloading a file, save to here

 
  void parent_draw(i4_draw_context_class &context);

  void receive_event(i4_event *ev);    // responds to buttons in window


  void stop_thread();

  void read_data();   // called by thread when ready_to_read reports data available

   // closes save file if need, and deletes it if it's too small
  void close_save_file(int wait_on_thread=0);
  
public:
  char *get_error_string();

  w8 save_buffer[FB_MAX_SAVE_BUFFER_SIZE];  // what the thread has loaded so far
  int last_save_buffer_size;  // used by main program to see if thread read anything
  int save_buffer_size;    // how much data has been stored in the buffer

  fb_state get_state() { return state; }

  void ack_data()    // main program tells us it has looked at our data
  {
    state=FB_WAITING;        
    save_buffer_size=0;
  }
  
  void update();
  void check_for_timeout();
 
  void set_url(fb_url *new_url);  // stops thread if and changes url, then starts thread
  fb_url *get_url() { return url; }

  fb_thread_window(w16 w, w16 h, i4_graphical_style_class *style);
  
  ~fb_thread_window();   // waits for thread to stop before closing the window

  char *name() { return "fb_thread_window"; }
};


#endif

