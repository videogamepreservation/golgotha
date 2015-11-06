/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef I4_BMP_WRITE_HH
#define I4_BMP_WRITE_HH

#include "arch.hh"

class i4_file_class;
class i4_image_class;

i4_bool i4_write_bmp(i4_image_class *im, i4_file_class *fp);

#endif
