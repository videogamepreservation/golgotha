/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef URL_HH
#define URL_HH


#include "network/net_prot.hh"
#include "network/net_addr.hh"
#include "memory/malloc.hh"
#include "checksum/checksum.hh"

enum fb_url_type { FB_FILE_URL,
                   FB_HTTP_URL };


struct fb_url
{  

  char *full_name;
  i4_net_socket *connect_to_server(i4_net_protocol *prot);

  w32 checksum() { return i4_check_sum32(full_name, strlen(full_name)); }
  
  fb_url(char *name, fb_url *server_from=0);

  ~fb_url()
  {
    if (full_name) 
      i4_free(full_name);
  }

  char *get_extension();
  char *get_filename();
};

void fb_split_url(char *url,
                  fb_url_type &type,
                  char *server,
                  int &port,
                  char *page);

// compares (not case sensitive) the first t bytes of 2 string and return 1 if they
// are equal, and 0 if the are not equal
int fb_strneq(char *n1, char *n2, int t); 

#endif
