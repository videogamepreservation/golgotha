/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "net/startup.hh"
#include "loaders/load.hh"
#include "device/device.hh"
#include "device/event.hh"
#include "gui/text_input.hh"
#include "gui/text.hh"
#include "gui/button.hh"
#include "resources.hh"
#include "network/login.hh"
#include "mess_id.hh"
#include "network/net_find.hh"
#include "network/net_prot.hh"
#include "time/timedev.hh"
#include "net/client.hh"
#include "net/server.hh"
#include "app/app.hh"

i4_event_reaction_class *g1_net_window_class::create_orec(int mess_id)
{
  i4_object_message_event_class *om=new i4_object_message_event_class(this, mess_id);
  return new i4_event_reaction_class(this, om);
}

g1_net_window_class::g1_net_window_class(w16 w, w16 h, 
                                         i4_graphical_style_class *style,
                                         i4_net_protocol *protocol,
                                         char *bg_res,
                                         int poll_delay,
                                         int poll_id)
  : i4_parent_window_class(w,h),
    style(style),
    protocol(protocol),
    poll_delay(poll_delay),
    poll_id(poll_id)
{
  bg=i4_load_image(i4gets(bg_res));

  i4_object_message_event_class poll(this, poll_id);
  poll_event_id=i4_time_dev.request_event(this, &poll, poll_delay);
}
    
void g1_net_window_class::receive_event(i4_event *ev)
{
  if (ev->type()==i4_event::OBJECT_MESSAGE)
  {
    CAST_PTR(oev, i4_object_message_event_class, ev);
    if (oev->object==this)
    {
      if (oev->sub_type==poll_id)
      {
        poll();

        i4_object_message_event_class poll(this, poll_id);
        poll_event_id=i4_time_dev.request_event(this, &poll, poll_delay);
      }
      else object_message(oev->sub_type);
    }
    else i4_parent_window_class::receive_event(ev);
  }
  else i4_parent_window_class::receive_event(ev);
}


void g1_net_window_class::parent_draw(i4_draw_context_class &context)
{
  if (bg)
  {
    local_image->clear(0, context);
    bg->put_image(local_image, 0, 0, context);
  }
}


g1_net_window_class::~g1_net_window_class()
{
  i4_time_dev.cancel_event(poll_event_id);

  if (bg)
    delete bg;
}

g1_startup_window::g1_startup_window(w16 w, w16 h, 
                                     i4_graphical_style_class *style,
                                     i4_net_protocol *protocol)
  : g1_net_window_class(w,h, style, protocol, "net_bg_image", 500, POLL)
{
  hostname=new i4_text_input_class(style, i4_string_man.get(0), 190, 200, this);
  add_child(g1_resources.net_hostname_x, g1_resources.net_hostname_y, hostname);

  username=new i4_text_input_class(style, *g1_resources.username, 90, 10, this);
  add_child(g1_resources.net_username_x, g1_resources.net_username_y, username);

  i4_text_window_class *start_game_text=new i4_text_window_class(i4gets("start_server"), style);
  i4_button_class *new_game;
  new_game=new i4_button_class(0,start_game_text, style, create_orec(START_SERVER));
  add_child(g1_resources.net_start_x, g1_resources.net_start_y, new_game);

  i4_text_window_class *quit_text=new i4_text_window_class(i4gets("net_main_menu"), style);
  add_child(new_game->x(), new_game->y()+new_game->height()+1, 
            new i4_button_class(0, quit_text, style, create_orec(QUIT_NET_GAME)));

  i4_net_protocol *p=protocol;
  if (p)
    find=p->create_finder_socket(g1_resources.net_find_port, g1_resources.net_find_port);
  else find=0;

  t_buts=0;
  buts=0;
}

void g1_startup_window::free_buts()
{
  for (int i=0; i<t_buts; i++)
  {
    remove_child(buts[i]);
    delete buts[i];
  }
    
  if (buts)
    i4_free(buts);
  buts=0;
  t_buts=0;
}

void g1_startup_window::grab_uname()
{
  delete g1_resources.username;
  i4_query_text_input_class q;
  i4_kernel.send_event(username, &q);
  g1_resources.username=q.copy_of_data;
  q.copy_of_data=0;
}

void g1_startup_window::object_message(int id)
{
  switch (id)
  {
    case START_SERVER :
    {
      grab_uname();

      g1_server=new g1_server_class(g1_resources.net_udp_port, protocol);

      i4_user_message_event_class u(G1_SERVER_MENU);
      i4_kernel.send_event(i4_current_app, &u);
    } break;

    case QUIT_NET_GAME :
    {
      grab_uname();

      i4_user_message_event_class u(G1_MAIN_MENU);
      i4_kernel.send_event(i4_current_app, &u);
    } break;
  }

  if (id>=LAST)
  {
    grab_uname();

    i4_finder_socket::server s;
    find->get_server(id-LAST, s);

    s.addr->set_port(g1_resources.net_udp_port);
    g1_client=new g1_client_class(s.addr, g1_resources.net_udp_port, protocol);

    i4_user_message_event_class u(G1_CLIENT_JOINED_MENU);
    i4_kernel.send_event(i4_current_app, &u);    
  }
}

void g1_startup_window::poll()
{
  if (find)
    if (find->poll())
    {
      free_buts();

      t_buts=find->total_servers();
      buts=(i4_button_class **)i4_malloc(sizeof(i4_button_class *)*t_buts,"but array");
  
      int y=g1_resources.net_found_y, x=g1_resources.net_found_x1;

      for (int i=0; i<t_buts; i++)
      {
        i4_finder_socket::server s;
        find->get_server(i, s);

        i4_text_window_class *t=new i4_text_window_class(*s.notification_string, style);
        buts[i]=new i4_button_class(0, t, style, create_orec(LAST + i));
  
        add_child(x,y, buts[i]);
        y+=buts[i]->height()+1;
      }
    }
}

g1_startup_window::~g1_startup_window()
{
  if (find)
    delete find;

  free_buts();
}

g1_server_start_window::g1_server_start_window(w16 w, w16 h, 
                                               i4_graphical_style_class *style,
                                               i4_net_protocol *protocol)
  : g1_net_window_class(w,h, style, protocol, "server_bg_image", 100, POLL)
{
  memset(names,0,sizeof(names));

  i4_text_window_class *start_game_text=new i4_text_window_class(i4gets("start_net_game"), 
                                                                 style);

  i4_object_message_event_class *sng_e=new i4_object_message_event_class(this, START_NET_GAME);
  i4_event_reaction_class *sng_r=new i4_event_reaction_class(this, sng_e);
  i4_button_class *new_game=new i4_button_class(0, start_game_text, style, sng_r);
  add_child(480,20, new_game);

  i4_text_window_class *quit_text=new i4_text_window_class(i4gets("net_main_menu"), style);
  i4_object_message_event_class *q_e=new i4_object_message_event_class(this, QUIT_NET_GAME);
  i4_event_reaction_class *q_r=new i4_event_reaction_class(this, q_e);
  i4_button_class *q=new i4_button_class(0, quit_text, style, q_r);
  add_child(new_game->x(), new_game->y()+new_game->height()+1, q);

  if (protocol)
    note=protocol->create_notifier_socket(g1_resources.net_find_port, *g1_resources.username);
  else note=0;
}


void g1_server_start_window::object_message(int id)
{
  switch (id)
  {
    case START_NET_GAME :
    {
      i4_user_message_event_class u(G1_START_NEW_GAME);
      i4_kernel.send_event(i4_current_app, &u);
    } break;

    case QUIT_NET_GAME :
    {
      if (g1_server)
      {
        delete g1_server;
        g1_server=0;
      }

      i4_user_message_event_class u(G1_MAIN_MENU);
      i4_kernel.send_event(i4_current_app, &u);
    } break;
  }
}

void g1_server_start_window::poll()
{
  if (note)
    note->poll();

  if (g1_server)
  {
    g1_server->poll();
    if (g1_server->list_changed)
    {
      g1_server->list_changed=i4_F;

      int i;
      for (i=0; i<G1_MAX_PLAYERS; i++)
        if (names[i])
        {
          remove_child(names[i]);
          names[i]=0;
        }

      i4_text_window_class *t;
      int x=i4getn("net_joined_x"), y=i4getn("net_joined_y");

      t=new i4_text_window_class(*g1_resources.username, style);
      add_child(x, y, t);
      y+=t->height()+1;
      names[0]=t;

      for (i=1; i<G1_MAX_PLAYERS; i++)
      {
        if (g1_server->clients[i].addr)
        {
          t=new i4_text_window_class(*g1_server->clients[i].username, style);
          add_child(x,y,t);
          y+=t->height()+1;
          names[i]=t;
        }
      }
    }
  }
}

g1_server_start_window::~g1_server_start_window()
{ 
  if (note)
    delete note;
}



g1_client_wait_window::g1_client_wait_window(w16 w, w16 h, 
                                             i4_graphical_style_class *style,
                                             i4_net_protocol *protocol)
  : g1_net_window_class(w,h, style, protocol, "client_wait_image", 100, POLL)
{
  i4_text_window_class *quit_text=new i4_text_window_class(i4gets("net_cancel"), style);
  i4_object_message_event_class *q_e=new i4_object_message_event_class(this, QUIT_NET_GAME);
  i4_event_reaction_class *q_r=new i4_event_reaction_class(this, q_e);
  i4_button_class *q=new i4_button_class(0, quit_text, style, q_r);
  add_child(i4getn("net_cancel_x"), i4getn("net_cancel_y"), q);
  
}

void g1_client_wait_window::object_message(int id)
{
  if (id==QUIT_NET_GAME)
  {
    i4_user_message_event_class u(G1_MAIN_MENU);
    i4_kernel.send_event(i4_current_app, &u);
  }
}

void g1_client_wait_window::poll()
{
  if (!g1_client || !g1_client->poll())
  {    
    i4_user_message_event_class u(G1_MAIN_MENU);
    i4_kernel.send_event(i4_current_app, &u);
  }
}

g1_client_wait_window::~g1_client_wait_window()
{
  if (g1_client)
  {
    delete g1_client;
    g1_client=0;
  }
}
