/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef I4_RAM_FILE_HH
#define I4_RAM_FILE_HH

#include "file/file.hh"
#include <memory.h>

class i4_ram_file_class : public i4_file_class
{
  w8 *buf;
  w32 bs, offset;

public:
  virtual i4_file_class *dup() 
  { 
    i4_file_class *f=new i4_ram_file_class(buf, bs);
    f->seek(tell());
    return f;
  }

  i4_ram_file_class(void *buffer, int buffer_size)
    : buf((w8 *)buffer), bs(buffer_size), offset(0) { ; }
  
  virtual w32 read (void *buffer, w32 size)
  {
    if (size>bs-offset)
      size=bs-offset;
    memcpy(buffer, buf+offset, size);
    offset+=size;
    return size;
  }
    
  virtual w32 write(const void *buffer, w32 size)
  {
    if (size>bs-offset)
      size=bs-offset;
    memcpy(buf+offset, buffer, size);
    offset+=size;
    return size;
  }

  virtual w32 seek (w32 _offset) { offset=_offset; return offset; }
  virtual w32 size ()            { return bs;      }
  virtual w32 tell ()            { return offset;  }
};


#endif
