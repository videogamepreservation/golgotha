/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef MAXCOMM_HH
#define MAXCOMM_HH

#include <windows.h>
#include "string/string.hh"

class m1_mail_slot_class
{
public:
  HANDLE slot;

  m1_mail_slot_class() : slot(INVALID_HANDLE_VALUE) {}

  int initialized() { return (slot != INVALID_HANDLE_VALUE); }

  int create(const i4_const_str &name);
  int open(const i4_const_str &name);

  w32 read(char *buff, w32 max_size);
  w32 write(char *buff, w32 size);

  int read_ready();
};

#endif
