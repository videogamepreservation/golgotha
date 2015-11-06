/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_CLIENT_HH
#define G1_CLIENT_HH


#include "time/time.hh"
#include "player_type.hh"
#include "string/string.hh"

class i4_net_address;
class i4_net_socket;
class i4_net_protocol;

class g1_client_class
{
  i4_net_address *server_address;
  i4_net_socket *listen;
  i4_net_socket *send;

  i4_time_class last_responce_time;
  int use_port;

  enum {
    JOIN_START,
    RUNNING
  } state;


  g1_player_type player_num;
  i4_str *map_name;

public:
  g1_client_class(i4_net_address *server_address, int use_port, i4_net_protocol *protocol);

  i4_bool poll();
  ~g1_client_class();
};

extern g1_client_class *g1_client;

#endif
