/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

class i4_socket_class
{
public:
  int fd;
  bind
  connect 
  accept
  gethostbyname
  socket( AF_INET,  SOCK_STREAM, 0); 
};



/* 
   This class responds to 'search' packets.  When other games are looking for
   a server they should have some way of finding the notifier socket.   
 */

class i4_notifier_socket
{
  i4_str *note_string;
protected:
  i4_notifier_socket(int port, const i4_const_str &notification_string);
public:
  virtual void set_notification_string(const i4_const_str &str);
  virtual void poll() = 0;
  virtual ~i4_notifier_socket();
};


class i4_finder_socket
{
protected:
  i4_finder_socket(int port);


}
