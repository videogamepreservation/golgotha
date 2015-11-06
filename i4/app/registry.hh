/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef I4_REGISTRY_HH
#define I4_REGISTRY_HH

#include "arch.hh"

enum i4_registry_type
{
  I4_REGISTRY_MACHINE,  // associated with machine
  I4_REGISTRY_USER      // associated with user
};


i4_bool i4_get_registry(i4_registry_type type, 
                        char *path, 
                        char *key_name,
                        char *buffer, int buf_length);

i4_bool i4_set_registry(i4_registry_type type, 
                        char *path, 
                        char *key_name,
                        char *buffer);



#endif
