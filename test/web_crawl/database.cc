/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/


#include "isllist.hh"


url::~url() 
{
  if (full_name)
    i4_free(full_name);
}


split_url::split_url(char *url_name)
{
  port=80;
  type=HTTP;
  
  server[0]=0;
  page[0]=0;
  extension[0]=0;

  int read_server=1;
  
  if (fb_strneq(url_name,"file://",7))
    type=FILE;
  else if (fb_strneq(url_name,"http://",7))
    type=HTTP;
  else read_server=0;

  if (read_server)
  {
    // read the server name
    while (url_name[0] && (url_name[0]!='/' || url_name[1]!='/')) url_name++;
    if (*url_name) url_name+=2;
  

    char *s=server;
    while (*url_name && *url_name!='/' && *url_name!=':')
      *(s++)=*(url_name++);
    *s=0;


    // read the port number if there is one
    if (*url_name==':')
    {
      url_name++;
      char num_buf[50],*c, count=0;
      for (c=num_buf; *url && *url!='/' && count<48; count++) 
        *(c++)=*(url_name++);
      *c=0;

      sscanf(num_buf,"%d",&port);
    }

  
    // skip final /
    if (*url_name) url_name++;
  }
  
  // read off the page name
  while (*url)
    *(page++)=*(url++);
  *page=0;

}

