/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef I4_NET_PROTOCOL_HH
#define I4_NET_PROTOCOL_HH

#include "string/string.hh"

// The network protocol is the interface to all the network commands.
// Multiple network interfaces such as TCPIP and IPX can exsist at the same time
// and you don't really have to know which one you are using.


class i4_net_address;             // defined in network/net_addr.hh
class i4_notifier_socket;         // defined in network/net_find.hh
class i4_finder_socket;           // defined in network/net_find.hh
class i4_net_socket;              // defined in network/net_sock.hh
class i4_net_address;             // defined in network/net_addr.hh

enum i4_protocol_type { I4_TCPIP };   // this is the only kind we have support for right now

enum i4_socket_type { I4_PACKETED_DATA,          // data is sent in packets (i.e. udp)
                      I4_CONTINOUS_STREAM };   // data is seen as a continous stream

class i4_net_protocol
{
public:
  static i4_net_protocol *first;  // protocols are kept in a list
  i4_net_protocol *next;

  i4_net_protocol();

  virtual char *name() = 0;               // for debugging
  virtual i4_protocol_type type() = 0;

  // this creates a socket that will notify other clients that this server
  // exsist if the create a finder socket.
  virtual i4_notifier_socket *create_notifier_socket(int port, 
                                             const i4_const_str &notification_string) = 0;

  // this socket will look for other server on the poll_port
  // implemented as spurious broadcast packets.
  virtual i4_finder_socket *create_finder_socket(int poll_port, int listen_port) = 0;

  // for TCPIP this converts an internet name to an IP address
  virtual i4_net_address *name_to_address(const i4_const_str &name) = 0;

  // try to make a connection to a remote server
  virtual i4_net_socket *connect(i4_net_address *addr,
                                 i4_socket_type type=I4_CONTINOUS_STREAM) = 0;

  // creates a socket that listens to a port, see network/net_sock.hh
  virtual i4_net_socket *listen(int port,
                                i4_socket_type type=I4_CONTINOUS_STREAM) = 0;

  // timeout=0 is inifinity, returns number of sockets with events pending
  virtual int select(int milli_sec_timeout) = 0;
};

inline i4_net_protocol *i4_get_first_protocol() { return i4_net_protocol::first; }


i4_net_protocol *i4_get_typed_protocol(i4_protocol_type type=I4_TCPIP); 

#endif
