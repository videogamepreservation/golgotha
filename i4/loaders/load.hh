/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef __IMAGE_LOAD_HPP_
#define __IMAGE_LOAD_HPP_

#include "arch.hh"
#include "init/init.hh"

class i4_const_str;
class i4_file_class;
class i4_image_class;
class i4_status_class;

class i4_image_loader_class : public i4_init_class
{
  public :
  // returns the maximum size you need to identify your file format
  virtual w16 max_header_size() = 0;                  

  // return true if you recognize your header signature
  virtual i4_bool recognize_header(w8 *buf) = 0;      

  // assume fp is at the start of the file
  // if status is non-null, then it should updated as the load progresses 
  virtual i4_image_class *load(i4_file_class *fp,
                               i4_status_class *status) = 0;            

  virtual void init();

  i4_image_loader_class *next;
} ;


i4_image_class *i4_load_image(const i4_const_str &filename, 
                              i4_status_class *status=0);

i4_image_class *i4_load_image(i4_file_class *fp,
                              i4_status_class *status=0);


#endif
