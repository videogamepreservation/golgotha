/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef I4_NET_FIND_HH
#define I4_NET_FIND_HH

#include "string/string.hh"

class i4_net_address;

/* 
   This class responds to 'search' packets.  When other games are looking for
   a server they should have some way of finding the notifier socket.   
 */

class i4_notifier_socket
{
public:
  virtual int poll() = 0;     // returns # of clients notified (this number may be inaccurate)

  virtual ~i4_notifier_socket() { ; }
};


class i4_finder_socket
{
public:
  struct server
  {
    i4_str *notification_string;
    i4_net_address *addr;
  };


  virtual i4_bool poll() = 0;   // returns true if list changed
  virtual int total_servers() = 0;
  virtual void get_server(int num, server &s) = 0;
  
  virtual ~i4_finder_socket() { ; }
};

#endif
