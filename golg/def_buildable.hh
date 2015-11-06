/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_DEF_BUILDABLE_HH
#define G1_DEF_BUILDABLE_HH

#include "g1_object.hh"
#include "objs/defaults.hh"
//#include "objs/factory.hh"

class g1_factory_class;

template <class T>
class g1_buildable_object_definer : public g1_object_definition_class
{
public:
  g1_object_defaults_struct *defaults;
  g1_object_build_info      build_info;
  
  g1_buildable_object_definer<T>
  (char *name,
   char *factory_name,
   w32 type_flags,
   function_type _init = 0,
   function_type _uninit = 0)
    : g1_object_definition_class(name, type_flags, _init, _uninit)
  { 
    build_info.factory_name = factory_name;
  }

  virtual g1_object_class *create_object(g1_object_type id,
                                         g1_loader_class *fp)
  {
    T *o=new T(id, fp);
    o->defaults=defaults;
    o->init();
    return o;
  }

  virtual void init()
  {
    defaults=g1_get_object_defaults(_name);
    build_info.cost=defaults->cost;

    g1_object_definition_class::init();
  }
  g1_object_build_info *get_build_info() { return &build_info; }
};

#ifndef G1_SELECTABLE_ENUM
enum { UNSELECTABLE = i4_F, SELECTABLE = i4_T };
#endif

#endif
