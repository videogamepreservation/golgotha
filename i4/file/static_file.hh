/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef STATIC_FILE_HH
#define STATIC_FILE_HH

#include "file/file.hh"
#include <stdio.h>

class i4_static_file_class : public i4_file_class
{
public:
  FILE *f;

  i4_static_file_class() : f(0) {}
  i4_static_file_class(const char *name) : f(0) {  }
  ~i4_static_file_class() { if (f) fclose(f); }

  i4_static_file_class* open(const char *name)
  {
    if (f)
      fclose(f);
    f=0;

    if (f = fopen(name, "wt"))
      return this;
    else
      return 0;
  }

  virtual w32 read (void *buffer, w32 size) { return 0; }
  virtual w32 write(const void *buffer, w32 size) { return fwrite(buffer, size, 1, f); }
  virtual w32 seek (w32 offset) { return 0; }
  virtual w32 size () { return 0; }
  virtual w32 tell () { return 0; }
};

#endif
