/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_NET_STARTUP_HH
#define G1_NET_STARTUP_HH


#include "window/window.hh"
#include "time/timedev.hh"
#include "g1_limits.hh"

class i4_text_input_class;
class i4_graphical_style_class;
class i4_event;
class i4_event_handler_class;
class i4_finder_socket;
class i4_notifier_socket;
class i4_button_class;
class i4_event_reaction_class;
class i4_net_protocol;

class g1_net_window_class : public i4_parent_window_class
{
protected:
  i4_image_class *bg;
  i4_time_device_class::id poll_event_id;
  i4_graphical_style_class *style;
  i4_net_protocol *protocol;
  int poll_delay, poll_id;

  i4_event_reaction_class *create_orec(int mess_id);
public:
  virtual void object_message(int id) { ; }
  virtual void poll() { ; }

  g1_net_window_class(w16 w, w16 h, 
                      i4_graphical_style_class *style,
                      i4_net_protocol *protocol,
                      char *bg_res,
                      int poll_delay, int poll_event_id);
    
  virtual void receive_event(i4_event *ev);
  virtual void parent_draw(i4_draw_context_class &context);

  virtual ~g1_net_window_class();
};

class g1_startup_window : public g1_net_window_class
{
  i4_button_class **buts;
  int t_buts;
  i4_text_input_class *hostname, *username;
  i4_finder_socket *find;

  enum { START_SERVER, QUIT_NET_GAME, POLL, LAST };

  void free_buts();
  void grab_uname();
public:
  virtual void object_message(int id);
  virtual void poll();

  g1_startup_window(w16 w, w16 h, 
                    i4_graphical_style_class *style,
                    i4_net_protocol *protocol);

  char *name() { return "net_startup"; }
  ~g1_startup_window();
};

class g1_server_start_window : public g1_net_window_class
{
  i4_notifier_socket *note;
  i4_window_class *names[G1_MAX_PLAYERS];

  enum { START_NET_GAME, QUIT_NET_GAME, POLL };
public:
  g1_server_start_window(w16 w, w16 h, 
                         i4_graphical_style_class *style,
                         i4_net_protocol *protocol);

  virtual void object_message(int id);
  virtual void poll();

  char *name() { return "server_menu"; }
  ~g1_server_start_window();
};


class g1_client_wait_window : public g1_net_window_class
{
  enum { QUIT_NET_GAME, POLL };

public:
  g1_client_wait_window(w16 w, w16 h, 
                         i4_graphical_style_class *style,
                         i4_net_protocol *protocol);

  virtual void object_message(int id);
  virtual void poll();

  char *name() { return "client_wait"; }
  ~g1_client_wait_window();
};


#endif


