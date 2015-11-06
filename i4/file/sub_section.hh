/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef I4_SUB_SECTION
#define I4_SUB_SECTION

#include "file/file.hh"

/*
The sub section file maps part of another file, it uses dup() on the other
file so seeks and closes won't cause problems.  The subsection file's offset 0,
begins somewhere inside the other file and reads are only allowed in this area.


 */

class i4_sub_section_file : public i4_file_class
{
  i4_file_class *fp;
  int fstart, fsize, foffset;
public:
  virtual w32 read (void *buffer, w32 size);
  virtual w32 write(const void *buffer, w32 size);
  virtual w32 seek (w32 offset);
  virtual w32 size ();
  virtual w32 tell ();

  virtual i4_bool async_read (void *buffer, w32 size, 
                              async_callback call,
                              void *context=0);

  i4_sub_section_file(i4_file_class *fp, int start, int size);
  ~i4_sub_section_file();
};


#endif
