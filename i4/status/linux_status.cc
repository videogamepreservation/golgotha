/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

enum { I4_STAT_LEN=70 };

#include "status/status.hh"
#include <stdlib.h>

class i4_linux_status_class : public i4_status_class
{
public:
  int last_percent;
  i4_linux_status_class() { last_percent=0; }

  virtual i4_bool update(float per)
  {
    while (per*I4_STAT_LEN > last_percent)
    {
      fprintf(stderr,".");
      last_percent++;
    }
  }

  virtual ~i4_linux_status_class()
  {
    int i;
    while (last_percent<I4_STAT_LEN)
    {
      fprintf(stderr,".");
      last_percent++;
    } 

    for (i=0; i<=I4_STAT_LEN; i++)      // backup
      fprintf(stderr,"%c",'\b');
    for (i=0; i<=I4_STAT_LEN+1; i++)     // erase
      fprintf(stderr,"%c",' ');
    for (i=0; i<=I4_STAT_LEN+1; i++)     // and backup again
      fprintf(stderr,"%c",'\b');    
  }
};

// this is operating system dependant
i4_status_class *i4_create_status(const i4_const_str &description, int allow_cancel)
{
  int i,j=description.length()>I4_STAT_LEN ? I4_STAT_LEN : description.length();
  fprintf(stderr,"[");
  
  i4_const_str::iterator it=description.begin();
  for (i=0; i<j;i++, ++it)
    fprintf(stderr,"%c",it.get().value());

  for (i=0; i<I4_STAT_LEN-j; i++)
    fprintf(stderr,"%c",' ');

  fprintf(stderr,"]");
  for (i=0; i<=I4_STAT_LEN; i++)     // erase
    fprintf(stderr,"%c",'\b');

  return new  i4_linux_status_class;
}
