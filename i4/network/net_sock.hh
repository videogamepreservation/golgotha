/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef I4_NET_SOCKET_HH
#define I4_NET_SOCKET_HH

#include "arch.hh"
#include "file/file.hh"

class i4_net_address;

class i4_net_socket : public i4_file_class
{
public:
  virtual i4_bool connect(i4_net_address *addr) { return i4_F; }

  virtual i4_bool accept(i4_net_socket *&sock, i4_net_address *&addr) = 0;

  virtual w32 read_from(void *buffer, w32 size, i4_net_address *&addr) = 0;

  virtual w32 read (void *buffer, w32 size) = 0;
  virtual w32 write(const void *buffer, w32 size) = 0;

  virtual i4_bool ready_to_read() = 0;
  virtual i4_bool ready_to_write() = 0;
  virtual i4_bool error_occurred() = 0;

  virtual w32 seek (w32 offset) { return 0; }
  virtual w32 size () { return 0; }
  virtual w32 tell () { return 0; }

  virtual void set_select_status(i4_bool read, i4_bool write, i4_bool error) = 0;

};


#endif
