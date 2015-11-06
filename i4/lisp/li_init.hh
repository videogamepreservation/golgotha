/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef LI_INIT_HH
#define LI_INIT_HH



#include "init/init.hh"
#include "lisp/lisp.hh"


#define li_automatic_add_function(fun_name, char_name)             \
class li_add_##fun_name##_class : public i4_init_class             \
{                                                                  \
  int init_type() { return I4_INIT_TYPE_LISP_FUNCTIONS; }          \
  void init()  { li_add_function(char_name, fun_name);  }          \
} li_add_function_##fun_name


#define li_automatic_add_type(type, var)                           \
class li_add_##type##_class : public i4_init_class                 \
{                                                                  \
  int init_type() { return I4_INIT_TYPE_LISP_FUNCTIONS; }          \
  void init()  { var=li_add_type(new type);  }                     \
} li_add_type_##type##_instance


#endif
