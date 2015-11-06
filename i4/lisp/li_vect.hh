/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef LI_VECT_HH
#define LI_VECT_HH


#include "lisp/li_types.hh"
#include "math/vector.hh"
#include "lisp/li_class.hh"

extern li_type_number li_vect_type;
class li_vect : public li_object
{
  friend class li_vect_type_function_table;
  i4_3d_vector *v;
public:
  li_vect(i4_3d_vector _v) : v(new i4_3d_vector), li_object(li_vect_type) 
  { 
    *v=_v;
  }

  li_vect(i4_3d_vector *v) : v(v), li_object(li_vect_type) {}

  i4_3d_vector &value() { return *v; }  
  static li_vect *get(li_object *o, li_environment *env)
  { check_type(o, li_vect_type, env); return ((li_vect *)o); }
} ;



struct li_vect_class_member : public li_class_member
{
  li_vect_class_member(char *name) : li_class_member(name) {}
  i4_3d_vector &operator()() { return li_vect::get(li_this->get(*this),0)->value(); }
};


#endif
