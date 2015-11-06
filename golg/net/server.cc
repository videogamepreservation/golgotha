/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "net/server.hh"
#include "net/command.hh"
#include "network/net_prot.hh"
#include "file/ram_file.hh"

g1_server_class *g1_server=0;

void g1_server_class::client::cleanup()
{
  if (addr)
  {
    delete addr;
    delete username;
  }
  if (send)
    delete send;
  addr=0;
  send=0;
}

g1_server_class::g1_server_class(int use_port, i4_net_protocol *protocol)
  : protocol(protocol)
{  
  map_name=new i4_str(i4gets("tmp_savename"));
  udp_port=0;
  memset(clients, 0, sizeof(clients));
  list_changed=i4_F;

  if (protocol)
  {
    udp_port=protocol->listen(use_port, I4_PACKETED_DATA);
    if (!udp_port)
      i4_warning("could not bind to port, server already running?");
    


  }
}

void start_game() { ; }


void g1_server_class::send_player_joined(int client_num)
{
  w8 packet[512];
  i4_ram_file_class r(packet, sizeof(packet));

  r.write_8(G1_PK_YOU_HAVE_JOINED);
  r.write_16(client_num+1);
  r.write_counted_str(*map_name);

  clients[client_num].send->write(packet, r.tell());
}

void g1_server_class::process_client_packet(w8 *packet,
                                            int packet_length, 
                                            int client_num)
{
  clients[client_num].last_data.get();

  i4_ram_file_class r(packet, packet_length);
  if (r.read_8()==G1_PK_I_WANNA_JOIN)
  {
    r.read_16();  // skip use port

    delete clients[client_num].username;
    clients[client_num].username=r.read_counted_str();
    send_player_joined(client_num);
  }    
}


void g1_server_class::poll()
{
  int i;

  while (udp_port && udp_port->ready_to_read())
  {
    w8 packet[512];
    i4_net_address *a;
    int len=udp_port->read_from(packet, sizeof(packet), a);
    
    // see if this was from one of our clients

    int found=0, free_spot=-1;

    for (i=0; i<G1_MAX_PLAYERS; i++)
    {
      if (clients[i].addr)
      {
        if (clients[i].addr->equals(a))
        {
          process_client_packet(packet, len, i);
          found=1;
        }
      }
      else free_spot=i;
    }

    if (!found && free_spot!=-1)
    {
      i4_ram_file_class r(packet, len);
      if (r.read_8()==G1_PK_I_WANNA_JOIN)
      {
        clients[free_spot].addr=a->copy();
        clients[free_spot].addr->set_port(r.read_16());
        clients[free_spot].username=r.read_counted_str();
        
        clients[free_spot].send=protocol->connect(clients[free_spot].addr, I4_PACKETED_DATA);

        if (clients[free_spot].send)
          send_player_joined(free_spot);
        else
          clients[free_spot].cleanup();

        list_changed=i4_T;
      }    
    }

    delete a;
  }

  if (state==WAITING_FOR_PLAYERS)
  {
    i4_time_class now;
    for (i=0; i<G1_MAX_PLAYERS; i++)
    {     
      if (clients[i].addr && now.milli_diff(clients[i].last_data)>1000)
      {
        clients[i].cleanup();
        list_changed=i4_T;
      }

    }
  }

}


g1_server_class::~g1_server_class()
{
  delete map_name;

  for (int i=0; i<G1_MAX_PLAYERS; i++)
    if (clients[i].addr)
      delete clients[i].addr;
  delete udp_port;
}
