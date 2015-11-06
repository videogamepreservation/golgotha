/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef FLI_LOAD_HH
#define FLI_LOAD_HH


class i4_file_class;
class i4_image_class;
#include "arch.hh"

i4_image_class **i4_load_fli(i4_file_class *fp, w32 &tframes);


#endif
