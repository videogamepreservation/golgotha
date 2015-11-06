/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef __I4_GET_DIR_HH
#define __I4_GET_DIR_HH

#include "arch.hh"
#include "string/string.hh"
#include "file/file.hh"

#error "don't use this file anymore (use file/file.hh)
i4_bool i4_get_directory(const i4_const_str &path,
                         i4_str **&files, w32 &tfiles, 
                         i4_str **&dirs, w32 &tdirs,
                         i4_file_status_struct **file_status);  


#endif
