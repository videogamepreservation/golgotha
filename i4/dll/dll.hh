/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef I4_DLL_HH
#define I4_DLL_HH

#include "string/string.hh"

class i4_dll_file
{
public:
  virtual ~i4_dll_file() { ; }
  virtual void *find_function(const char *name) = 0;
};

i4_dll_file *i4_open_dll(const i4_const_str &name);


#endif
