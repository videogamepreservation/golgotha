/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_OBJECT_DEFINER_HH
#define G1_OBJECT_DEFINER_HH

#include "g1_object.hh"
#include "objs/defaults.hh"

// this can be used to generate a definition class for an object and automatically add it into 
// the game via it's global constructor, I think this will work with dlls as well because
// their global constructors are called when a dll is loaded

template <class T>
class g1_object_definer : public g1_object_definition_class
{
public:
  g1_object_definer<T>(char *name,
                       w32 type_flags=0,
                       function_type _init = 0,
                       function_type _uninit = 0)
    : g1_object_definition_class(name, type_flags ,_init, _uninit) {}

  virtual g1_object_class *create_object(g1_object_type id,
                                         g1_loader_class *fp)
  {
    T *o=new T(id, fp);
    o->init();
    return o;
  }
};

class li_object;
class li_environment;

// called from g1_load_level (level_load.cc) : creates and objects type from lisp token
void g1_define_object(li_object *o, li_environment *env);

#endif
