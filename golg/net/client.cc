/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "network/net_addr.hh"
#include "network/net_sock.hh"
#include "network/net_prot.hh"
#include "net/client.hh"
#include "net/command.hh"
#include "file/ram_file.hh"
#include "resources.hh"

g1_client_class *g1_client=0;

g1_client_class::g1_client_class(i4_net_address *server_address, 
                                 int use_port,
                                 i4_net_protocol *protocol)
  : server_address(server_address->copy()),
    use_port(use_port)
{
  listen=0;
  send=0;
  map_name=0;

  if (protocol)
  {
    listen=protocol->listen(use_port, I4_PACKETED_DATA);
    send=protocol->connect(server_address, I4_PACKETED_DATA);
  }
}

i4_bool g1_client_class::poll()  // returns false if server is not responding
{
  if (!listen || !send)
    return 0;

  if (state==JOIN_START)
  {
    i4_time_class now;
    if (now.milli_diff(last_responce_time)>5000)  // if it's been 5 secs assume server dead
      return i4_F;

    
    w8 packet[512];
    i4_ram_file_class r(packet, sizeof(packet));

    r.write_8(G1_PK_I_WANNA_JOIN);
    r.write_16(use_port);
    r.write_counted_str(*g1_resources.username);

    if (!send->write(packet, r.tell()))
      return i4_F;

  }

  if (listen->ready_to_read())
  {
    w8 packet[512];
    i4_net_address *a;
    int s=listen->read_from(packet, sizeof(packet), a);
    if (a->equals(server_address))
    {
      i4_ram_file_class r(packet, sizeof(packet));

      if (r.read_8()==G1_PK_YOU_HAVE_JOINED)
      {
        player_num=r.read_16();
       
        if (map_name)
          delete map_name;
        map_name=r.read_counted_str();
        state=JOIN_START;
        last_responce_time.get();
      }
    }
    delete a;
  }
  return i4_T;
}


g1_client_class::~g1_client_class()
{
  delete server_address;

  if (map_name)
    delete map_name;

  if (listen)
    delete listen;

  if (send)
    delete send;
}
