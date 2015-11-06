/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "file/sub_section.hh"
#include "error/error.hh"


w32 i4_sub_section_file::read (void *buffer, w32 rsize)
{
  if (foffset-fstart+rsize>fsize)  // don't allow reads past the sub section
    rsize=fstart+fsize-foffset;

  int rs=fp->read(buffer, rsize);
  foffset+=rs;
  return rs;
}

w32 i4_sub_section_file::write(const void *buffer, w32 size)
{
  i4_error("don't write here");
  return 0;
}


w32 i4_sub_section_file::seek(w32 offset)
{
  if (offset>=fsize)
    offset=fsize-1;
  
  foffset=fstart+offset;
  fp->seek(foffset);
  return foffset;
}

w32 i4_sub_section_file::size()
{
  return fsize;
}

w32 i4_sub_section_file::tell()
{
  return foffset-fstart;
}


i4_sub_section_file::i4_sub_section_file(i4_file_class *_fp, int start, int size)
{
  fp=_fp;
  if (!fp)
    i4_error("no file");

  fp->seek(start);
  fsize=size;
  fstart=start;
  foffset=start;
}

i4_sub_section_file::~i4_sub_section_file()
{
  if (fp)
    delete fp;
}


i4_bool i4_sub_section_file::async_read (void *buffer, w32 size, 
                                         async_callback call,
                                         void *context)
{
  if (foffset-fstart+size>fsize)  // don't allow reads past the sub section
    size=fstart+fsize-foffset;


  foffset+=size;  
  return fp->async_read(buffer, size, call, context);
}
