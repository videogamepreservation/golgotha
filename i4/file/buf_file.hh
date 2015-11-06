/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef __I4BUF_FILE_HPP
#define __I4BUF_FILE_HPP

#include "file/file.hh"


// This file provides buffering for another i4_file_class

class i4_buffered_file_class : public i4_file_class
{
protected:
  friend void i4_async_buf_read_callback(w32 count, void *context);
  i4_file_class *from;
  void *buf;
  w32 buf_size,buf_start,buf_end,offset;
  i4_bool write_file;
public:
  i4_buffered_file_class(i4_file_class *from, 
                         w32 buffer_size=0x1000,
                         w32 current_offset=0);


  virtual w32 read (void *buffer, w32 size);
  virtual w32 write(const void *buffer, w32 size);
  virtual w32 seek (w32 offset);
  virtual w32 size ();
  virtual w32 tell ();

  virtual i4_bool async_read (void *buffer, w32 size, 
                              async_callback call,
                              void *context);

  ~i4_buffered_file_class();
};

#endif
