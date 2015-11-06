/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef I4_RAM_FILE_MAN_HH
#define I4_RAM_FILE_MAN_HH

#include "arch.hh"

class i4_openable_ram_file_info
{   
public:
  char *filename;
  void *data;
  w32 data_size;
  i4_openable_ram_file_info *next;

  i4_openable_ram_file_info(char *filename, void *data, w32 data_size);
};



#endif
