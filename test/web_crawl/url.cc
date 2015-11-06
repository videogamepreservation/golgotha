/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "url.hh"
#include "network/net_sock.hh"
#include <ctype.h>
#include <stdio.h>

int fb_strneq(char *n1, char *n2, int t)
{
  for (int i=0; i<t; i++)
    if (toupper(*(n1++))!=toupper(*(n2++)))
      return 0;
  
  return 1;
}

void fb_split_url(char *url,
                  fb_url_type &type,
                  char *server,
                  int &port,
                  char *page)
{
  // initalize defaults in case an error occurs
  port=80;
  type=FB_HTTP_URL;
  
  server[0]=0;
  int dont_slip_server=0;
  
  if (fb_strneq(url,"file://",7))
    type=FB_FILE_URL;
  else if (fb_strneq(url,"http://",7))
  {
    if (toupper(url[strlen(url)-1])=='l' ||    // should end in .html or .htm
        toupper(url[strlen(url)-1])=='m')
      type=FB_HTTP_URL;
    else type=FB_FILE_URL;
  }
  else dont_slip_server=1;

  if (!dont_slip_server)
  {
    // read the server name
    while (url[0] && (url[0]!='/' || url[1]!='/')) url++;
    if (*url) url+=2;
  
    while (*url && *url!='/' && *url!=':')
      *(server++)=*(url++);
    *server=0;


    // read the port number if there is one
    if (*url==':')
    {
      url++;
      char num_buf[50],*c, count=0;
      for (c=num_buf; *url && *url!='/' && count<48; count++) 
        *(c++)=*(url++);
      *c=0;

      sscanf(num_buf,"%d",&port);
    }

    // skip final /
    if (*url) url++;
  }
  
  // read off the page name
  while (*url)
    *(page++)=*(url++);
  *page=0;
}


char *end_name(char *name)
{
  char *start=name;
  while (*name)
  {
    if (*name=='/' || *name=='\\') 
      start=name;
    name++;
  }
  return start+1;
}


i4_net_socket *fb_url::connect_to_server(i4_net_protocol *prot)
{
  fb_url_type type;
  char server[256], page[256];
  int port;
  
  fb_split_url(full_name, type, server, port, page);

  i4_net_address *addr=prot->name_to_address(server);
  
  if (!addr)
    return 0;
  
  addr->set_port(port);

  i4_net_socket *sock=prot->connect(addr, I4_CONTINOUS_STREAM);
  delete addr;

  if (sock)
    sock->printf("GET /%s HTTP/1.0\n\n",page);
  
  return sock;
}



fb_url::fb_url(char *name, fb_url *server_from)
{
  char server[256], page[256], buf[256];
  int port;
  fb_url_type type;

  // split the name up
  fb_split_url(name, type, server, port, page);

  // if no server name in url try to use the current server
  if (!server[0] && server_from)
  {
    fb_url_type type2;
    fb_split_url(server_from->full_name, type2, server, port, page);

    if (name[0]!='/')
    {
      if (page[strlen(page)-1]!='/')
        strcat(page,"/");
      
      sprintf(buf,"http://%s:%d/%s%s",server, port, page, name);
    }
    else if (name[0]!='/')
      sprintf(buf,"http://%s:%d/%s",server, port, name);
    else
      sprintf(buf,"http://%s:%d/%s",server, port, name);

    name=buf;    
  }  
  
  int l=strlen(name)+1;
  full_name=(char *)i4_malloc(l,"");
  strcpy(full_name, name);    
}


char *fb_url::get_extension()
{
  char *ext=0;
  for (char *c=full_name; *c; c++)
    if (*c=='.')   
      ext=c;
    else if (*c=='/' || *c=='\\')
      ext=0;
  
  return ext;
}

char *fb_url::get_filename()
{
  // extract the filename
  char *c=full_name, *start=0;
  for (; *c; c++)
    if (*c=='/') start=c;
  if (*start=='/') start++;
  return start;
}
