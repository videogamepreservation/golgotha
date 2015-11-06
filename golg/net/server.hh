/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_SERVER_HH
#define G1_SERVER_HH


#include "network/net_addr.hh"
#include "network/net_sock.hh"
#include "g1_limits.hh"
#include "time/time.hh"

class g1_server_class
{
  friend class g1_server_start_window;

  struct client
  {
    i4_net_address *addr;
    i4_time_class last_data;
    i4_str *username;
    i4_net_socket *send;
    
    void cleanup();
  };

  client clients[G1_MAX_PLAYERS];   // network address of each client

  i4_net_socket *udp_port;
  enum {
    WAITING_FOR_PLAYERS,
    RUNNING,
    QUITING
  } state;
    
  i4_net_protocol *protocol;
  void send_player_joined(int client_num);
  void process_client_packet(w8 *packet, int packet_length, int client_num);
  i4_str *map_name;
  i4_bool list_changed;

public:
  g1_server_class(int use_port, i4_net_protocol *protocol);
  void start_game();
  void poll();  
  ~g1_server_class();
};

extern class g1_server_class *g1_server;

#endif
