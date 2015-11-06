/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/


#define PTHREAD_KERNEL  /* Needed for OK and NOTOK defines */



#ifndef _WINDOWS
#define SIZE_TYPE unsigned int
#include <unistd.h>

#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <netdb.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define closesocket ::close
#define OPTION_TYPE int
#else

#include <io.h>
#include <windows.h>
#define OPTION_TYPE BOOL
#define SIZE_TYPE int

#endif

#include "error/error.hh"
#include "network/net_prot.hh"
#include "network/net_find.hh"
#include "network/net_addr.hh"
#include "network/net_sock.hh"

#include "time/time.hh"
#include "memory/array.hh"

class i4_tcpip_protocol;
extern i4_tcpip_protocol i4_tcpip_protocol_instance;

static int i4_get_my_addr(sockaddr_in &host)
{
  char buf[256];
  if (gethostname(buf, sizeof(buf))==0)
  {
    hostent *hp=gethostbyname(buf);
    if (hp)
    {
      memset( (char*) &host,0, sizeof(host));
      memcpy(&host.sin_addr,hp->h_addr,hp->h_length);
      return 1;
    }
  }
  return 0;
}


class i4_tcpip_address : public i4_net_address
{
public:
  sockaddr_in addr;

  virtual i4_net_address *copy()
  {
    i4_tcpip_address *a=new i4_tcpip_address;
    a->addr=addr;
    a->protocol=protocol;
    return a;
  }

  virtual void set_port(int port)
  {
    addr.sin_port = htons(port);
  }

  virtual i4_bool equals(i4_net_address *other)
  {
    i4_tcpip_address *to=(i4_tcpip_address *)other;
    if (protocol==other->protocol && addr.sin_addr.s_addr==to->addr.sin_addr.s_addr)
      return i4_T;
    else return i4_F;
  }
};



class i4_tcpip_sock : public i4_net_socket
{
  int fd;
  i4_socket_type stype;
  i4_net_protocol *protocol;
  i4_tcpip_protocol *prot() { return (i4_tcpip_protocol *)protocol; }
public:
  int er;

  void set_select_status(i4_bool read, i4_bool write, i4_bool error);
  
  int listen(int port)
  {
    OPTION_TYPE opt=1;

    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt))!=0)
    { 
      i4_warning("setsockopt reuse failed"); 
      return 0;
    }
    
    sockaddr_in host;
    memset( (char*) &host,0, sizeof(host));
    host.sin_family = AF_INET;
    host.sin_port = htons(port);
    host.sin_addr.s_addr = htonl (INADDR_ANY);

    if (bind(fd, (sockaddr *) &host,  sizeof(host))==-1)
    { 
      i4_warning("bind failed"); 
      return 0; 
    }

    if (stype==I4_CONTINOUS_STREAM && ::listen(fd, 5)==-1)
    {
      i4_warning("listen failed");
      return 0;
    }
      

    return 1;
  }

  i4_bool connect(i4_net_address *addr)
  {
    i4_tcpip_address *a = (i4_tcpip_address *)addr;

    if ((i4_tcpip_protocol *)a->protocol==&i4_tcpip_protocol_instance)
    {
      if (::connect(fd, (sockaddr *) &(a->addr), sizeof(a->addr))==-1)
        return i4_F;
      return i4_T;
    }
    else
      i4_warning("address from another protocol");
    
    return i4_F;
  }

  i4_tcpip_sock(i4_socket_type stype,
                i4_net_protocol *protocol)
    : protocol(protocol), stype(stype)
  {
    fd=socket(AF_INET, stype==I4_CONTINOUS_STREAM ? SOCK_STREAM : SOCK_DGRAM, 0);

    if (fd<0)
    { 
      i4_warning("out of sockets"); 
      er=1;
    }
    else er=0;
  }

  i4_tcpip_sock(int fd, i4_socket_type stype, i4_net_protocol *protocol) 
    : fd(fd), stype(stype), protocol(protocol) 
  { ; }

  i4_bool accept(i4_net_socket *&sock, i4_net_address *&addr)
  {
    sockaddr_in a;
    SIZE_TYPE size=sizeof(a);
    int new_fd=::accept(fd, (sockaddr *)&a, &size);
    if (new_fd==-1) 
      return i4_F;

    sock=new i4_tcpip_sock(new_fd, stype, protocol);
    addr=new i4_tcpip_address;
    ((i4_tcpip_address *)addr)->addr=a;
    addr->protocol=protocol;
    return i4_T;
  }

  ~i4_tcpip_sock()
  {
    if (fd!=-1) 
      closesocket(fd);   
  }
  
  virtual w32 read_from(void *buffer, w32 size, i4_net_address *&addr)
  {
    sockaddr_in a;
    SIZE_TYPE as=sizeof(a);
    int s=recvfrom(fd, (char *)buffer, size, 0, (sockaddr *)&a, &as);
    addr=new i4_tcpip_address;
    ((i4_tcpip_address *)addr)->addr=a;
    addr->protocol=protocol;
    return s;
  }

  virtual w32 read (void *buffer, w32 size)
  {
    return recv(fd, (char *)buffer, size, 0);
  }

  virtual w32 write(const void *buffer, w32 size)
  {
    return send(fd, (char *)buffer, size, 0);
  }

  virtual i4_bool ready_to_read()
  {
    fd_set s;
    struct timeval tv={0,0};
    FD_ZERO(&s);
    FD_SET(fd, &s);
    
    select(fd+1, &s, NULL, NULL, &tv);    
    return FD_ISSET(fd, &s);
  }

  virtual i4_bool ready_to_write()
  {
    fd_set s;
    struct timeval tv={0,0};
    FD_ZERO(&s);
    FD_SET(fd, &s);
    

    select(fd+1, NULL, &s, NULL, &tv);    
    return FD_ISSET(fd, &s);
  }

  virtual i4_bool error_occurred()
  {
    fd_set s;
    struct timeval tv={0,0};
    FD_ZERO(&s);
    FD_SET(fd, &s);
    
    select(fd+1, NULL, NULL, &s, &tv);    
    return FD_ISSET(fd, &s);
  }
};



class i4_reply_str : public i4_str
{
public:
  i4_reply_str(char *buf, int _len)
    : i4_str(_len)
  {
    len=_len;
    memcpy(ptr, buf, len);
  }
  void *data() { return ptr; }
};

class i4_tcpip_finder : public i4_finder_socket
{
  int poll_fd, listen_fd, listen_port;
  int t;

  sockaddr_in my_addr;

  i4_array<server> replies;

  i4_time_class last_ping;

  i4_net_protocol *protocol;
public:
  int er;

  int total_servers()
  {
    return replies.size();
  }

  virtual void get_server(int num, server &s)
  {
    s=replies[num];
  }

  ~i4_tcpip_finder() 
  {
    if (poll_fd!=-1) closesocket(poll_fd);
    if (listen_fd!=-1) closesocket(listen_fd);

    for (int i=0; i<replies.size(); ++i)
    {
      delete replies[i].notification_string;
      delete replies[i].addr;
    }      
  }

  int setup(int poll_port, int listen_port)
  {
    poll_fd=-1; listen_fd=-1;

    if (!i4_get_my_addr(my_addr))
    { i4_warning("couldn't get my ip"); return 0; }

    poll_fd=socket(AF_INET, SOCK_DGRAM, 0);
    if (poll_fd<0)
    { i4_warning("out of sockets"); return 0; }

    sockaddr_in host;
    memset( (char*) &host,0, sizeof(host));
    host.sin_family = AF_INET;
    host.sin_port = htons(poll_port);
    host.sin_addr.s_addr = htonl (INADDR_BROADCAST);
  
    OPTION_TYPE opt=1;
    if (setsockopt(poll_fd, SOL_SOCKET, SO_BROADCAST, (char *)&opt, sizeof(opt))!=0)
    { i4_warning("setsockopt broadcast failed"); return 0; }

    opt=1;
    if (setsockopt(poll_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt))!=0)
    { i4_warning("setsockopt reuse failed"); return 0; }

    if (connect(poll_fd, (sockaddr *) &host, sizeof(host))==-1)
    { i4_warning("connect failed"); return 0; }


    listen_fd=socket(AF_INET, SOCK_DGRAM, 0);
    if (listen_fd<0)
    { i4_warning("out of sockets"); return 0; }

    host.sin_port = htons(listen_port);
    host.sin_addr.s_addr = htonl (INADDR_ANY);

    if (bind(listen_fd, (sockaddr *) &host,  sizeof(host))==-1)
    { i4_warning("bind failed"); return 0; }

    last_ping.get();
    return 1;
  }

  i4_tcpip_finder(int poll_port, int listen_port, i4_net_protocol *protocol)
    : replies(32, 64),
      listen_port(listen_port),
      protocol(protocol)
  {
    er=!setup(poll_port, listen_port);
  }

  i4_bool poll()
  {
    fd_set read_set;
    FD_ZERO(&read_set);
    FD_SET(listen_fd, &read_set);
    struct timeval tv={0,0};

    // see if there are any responses from servers
    select(FD_SETSIZE,&read_set,0, 0, &tv);
    if (FD_ISSET(listen_fd, &read_set))
    {
      w8 buf[512];
      i4_tcpip_address r;

      SIZE_TYPE from_len=sizeof(sockaddr_in);
      if (recvfrom(listen_fd, (char *)buf, sizeof(buf), 0, (sockaddr *)&r.addr, &from_len)>2)
      {
        w16 s_len=(buf[0]<<8) | buf[1];
        if (s_len<510)  // if this more than 510 then bad data in packet
        {
          int found=0;
          if (r.addr.sin_addr.s_addr==my_addr.sin_addr.s_addr)
            found=1;

          // see if we already found heard from this guy
          for (int i=0; i<replies.size(); i++)
            if (r.addr.sin_addr.s_addr==
                ((i4_tcpip_address *)replies[i].addr)->addr.sin_addr.s_addr)
              found=1;

          if (!found)
          {
            server s;
            s.notification_string=new i4_reply_str((char *)buf+2, s_len);
            s.addr=new i4_tcpip_address(r);
            s.addr->protocol=protocol;

            replies.add(s);
            return i4_T;
          }       
        }
      }
    }
  

    i4_time_class now;

    if (now.milli_diff(last_ping) > 1000)   // ping once a second
    {
      w8 reply_port[2];
      reply_port[0]=(listen_port>>8);
      reply_port[1]=(listen_port&0xff);

      if (send(poll_fd,  (char *)reply_port, 2, 0)!=2)
        i4_warning("ping server error");
      last_ping.get();
    }

    return i4_F;
  }
};

class i4_tcpip_notifier : public i4_notifier_socket
{
  int fd;
  w8 reply_packet[512];
public:
  int er;

  ~i4_tcpip_notifier()
  {
    if (fd==-1) closesocket(fd);
  }

  int setup(int port, const i4_const_str &notification_string) 
  {
    fd=-1;

    fd=socket(AF_INET, SOCK_DGRAM, 0);
    if (fd<0)
    { i4_warning("out of sockets"); return 0; }

    OPTION_TYPE opt=1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt))!=0)
    { i4_warning("setsockopt reuse failed"); return 0; }

    sockaddr_in host;
    memset( (char*) &host,0, sizeof(host));
    host.sin_family = AF_INET;
    host.sin_port = htons(port);
    host.sin_addr.s_addr = htonl (INADDR_ANY);

    if (bind(fd, (sockaddr *) &host,  sizeof(host))==-1)
    { i4_warning("notify bind failed"); return 0; }

    int s_len=notification_string.length();
    reply_packet[0]=s_len>>8;
    reply_packet[1]=s_len&0xff;

    memcpy(reply_packet+2, ((i4_reply_str *)&notification_string)->data(), s_len);
      
    return 1;
  }

  i4_tcpip_notifier(int port,  const i4_const_str &notification_string)  
  {
    er=!setup(port, notification_string);    
  }

  virtual int poll()
  {
    fd_set read_set;
    FD_ZERO(&read_set);
    FD_SET(fd, &read_set);
    struct timeval tv={0,0};

    // see if there are any clients pinging us
    select(FD_SETSIZE,&read_set,0, 0, &tv);
    if (FD_ISSET(fd, &read_set))
    {    
      w8 buf[512];
      sockaddr_in host;
      SIZE_TYPE from_len=sizeof(sockaddr_in);
      if (recvfrom(fd, (char *)buf, sizeof(buf), 0, (sockaddr *)&host, &from_len)==2)
      {
        int reply_port=(buf[0]<<8)|buf[1];
        host.sin_port=htons(reply_port);
        sendto(fd, (char *)reply_packet, sizeof(reply_packet), 0, (sockaddr *)&host, sizeof(host));
        return 1;
      }
    }
    return 0;

  }
};

#ifdef __linux
#include <stdio.h>
#include <string.h>

// linux gethostname doesn't seem to be thread-safe, so I'm making a system call
int linux_name_to_address(char *hostname, unsigned int &ip)
{
  char buf[200];
  sprintf(buf, "nslookup -ret=2 -timeout=2 %s", hostname);

  fprintf(stderr,"\n*** %s **\n",buf);
  
  FILE *fp=popen(buf, "rb");
    fprintf(stderr,"\n*** popen done fp=%p**\n",fp);
  if (!fp) return 0;
  while (!feof(fp))
  {
    fgets(buf,200,fp);
    fprintf(stderr,"\n*** fgets %s**\n",buf);
    if (strstr(buf, hostname))
    {
      fgets(buf,200,fp);
      buf[strlen(buf)-1]=0;

      char *c;
      for (c=buf; *c!=':'; c++); c++;
      for (; *c==' '; c++);

      int i1,i2,i3,i4;
      sscanf(c,"%d.%d.%d.%d", &i1,&i2,&i3,&i4);

      char ret[4];
      ret[0]=i1;  ret[1]=i2;  ret[2]=i3;  ret[3]=i4;

      fclose(fp);
      ip= *((unsigned long *)ret);

      fprintf(stderr,"\n*** found ip **\n");
      return 1;
    }
  }

  fclose(fp);
  return 0;
}

#endif


class i4_tcpip_protocol : public i4_net_protocol
{
  int err;
public:
  fd_set rfdset, wfdset, efdset;
  int max_fd;

  int select(int milli_sec_timeout)
  {
    if (!milli_sec_timeout)
      return ::select(max_fd, &rfdset, &wfdset, &efdset, 0);
    else
    {
      int secs=milli_sec_timeout/1000;
      int usecs=(milli_sec_timeout-(secs*1000)) *1000;
      
      struct timeval tv={secs, usecs};    
      return ::select(max_fd, &rfdset, &wfdset, &efdset, &tv);
    }
  }
  
  i4_tcpip_protocol()
  {
    max_fd=0;
    FD_ZERO(&rfdset);
    FD_ZERO(&wfdset);
    FD_ZERO(&efdset);
    
#ifdef _WINDOWS
    WSADATA wsaData;
    WORD ver=MAKEWORD(1,1);
    err=WSAStartup(ver, &wsaData);
#else
    err=0;
#endif
   
  }

  ~i4_tcpip_protocol()
  {
#ifdef _WINDOWS
    if (!err)
      WSACleanup();
#endif
  }

  i4_notifier_socket *create_notifier_socket(int port, const i4_const_str &notification_string) 
  {
    if (err) return 0;

    i4_tcpip_notifier *n=new i4_tcpip_notifier(port, notification_string);
    if (n->er)
    {
      delete n;
      return 0;
    }
    else 
      return n;
  }

  i4_finder_socket *create_finder_socket(int poll_port, int listen_port)
  {
    if (err) return 0;

    i4_tcpip_finder *f=new i4_tcpip_finder(poll_port, listen_port, this);
    if (f->er) 
    { 
      delete f; 
      return 0; 
    }
    else 
      return f;
  }

  virtual i4_net_address *name_to_address(const i4_const_str &name)
  {
    if (err) return 0;

    sockaddr_in host;
    memset( (char*) &host,0, sizeof(host));
          
    char buf[256];
    i4_os_string(name, buf, 256);
        
    //    if (linux_name_to_address(buf, host.sin_addr.s_addr))
    // {

    hostent *hp=gethostbyname(buf);
    if (hp)
    {      
      memcpy(&host.sin_addr,hp->h_addr,hp->h_length);

      host.sin_family = AF_INET;

      i4_tcpip_address *a=new i4_tcpip_address;
      a->addr=host;
      a->protocol=this;
      return a;
    }
    return 0;

  }

  i4_net_socket *connect(i4_net_address *addr, i4_socket_type stype)
  {
    if (err) return 0;
    if (addr->protocol==this)
    {
      i4_tcpip_sock *s=new i4_tcpip_sock(stype, this);
      if (s->er || !s->connect((i4_tcpip_address *)addr))
      {
        delete s;
        return 0;
      } else return s;
    }
    else i4_warning("address from another protocol");
    return 0;
  }

  i4_net_socket *listen(int port, i4_socket_type stype)
  {
    if (err) return 0;

    i4_tcpip_sock *s=new i4_tcpip_sock(stype, this);
    if (s->er || !s->listen(port))
    {
      delete s;
      return 0;
    } else return s;
  }

  char *name() { return "TCP/IP"; }
  i4_protocol_type type() { return I4_TCPIP; }
};

i4_tcpip_protocol i4_tcpip_protocol_instance;


void i4_tcpip_sock::set_select_status(i4_bool read, i4_bool write, i4_bool error) 
{
  i4_tcpip_protocol *p=prot();
  if (read)
    FD_SET(fd, &p->rfdset);
  else
    FD_CLR(fd, &p->rfdset);

  if (write)
    FD_SET(fd, &p->wfdset);
  else
    FD_CLR(fd, &p->wfdset);

  if (error)
    FD_SET(fd, &p->efdset);
  else
    FD_CLR(fd, &p->efdset);

  if (fd>=p->max_fd)
    p->max_fd=fd+1;
}

