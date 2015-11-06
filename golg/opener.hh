/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_OPENER_HH
#define G1_OPENER_HH

// this function should be called to open all files in the game
// it will check the cd_image for quick local loading of files before
// trying to load the file externally

#include "file/open.hh"

i4_file_class *g1_opener(const i4_const_str &name, w32 flags = i4_file_manager_class::READ);

#endif
