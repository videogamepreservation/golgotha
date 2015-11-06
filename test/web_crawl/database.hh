/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/





struct server_info
{
  int html_read;       // number of pages we've read from this server
  int html_waiting;    // number of pages waiting to be read fromt his server

  int files_read;      
  int files_failed;
  int files_waiting;  

  server_info *next;
};



struct split_url
{
  enum { FILE, HTTP } type;
  char server[100];
  int  port;
  char page[100];
  char extension[100];

  split_url(char *url_name);
};


struct url
{  
  char *full_name;
  server_info *server_for;

  w32 checksum();  
  url(char *name,            // text name of url "http://crack.com:80/test.html"
      url *server_from=0);   // in case url is a relative path we need to know where it came from

  ~url();                    // frees memory use allocated by constructor

  void split(split_url &s); // splits the url into type, server name, page, and extension
  
};


