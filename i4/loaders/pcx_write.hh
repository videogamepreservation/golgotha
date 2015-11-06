/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef __WRITE_PCX_HH
#define __WRITE_PCX_HH


#include "palette/pal.hh"

class i4_file_class;
class i4_image_class;
void i4_write_pcx(i4_image_class *im,
                  const i4_pal_handle_class &pal,
                  i4_file_class *fp);

#endif
