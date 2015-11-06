/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef LI_OBJREF_HH
#define LI_OBJREF_HH


#include "lisp/li_types.hh"
#include "global_id.hh"
#include "lisp/li_class.hh"

class g1_object_class;
class g1_draw_context_class;

extern li_type_number li_g1_ref_type_number;
class li_g1_ref : public li_object
{
  friend class li_g1_ref_function;
  w32 _global_id;
public:
  li_g1_ref(w32 global_id) 
    : li_object(li_g1_ref_type_number) 
  { 
    _global_id=global_id;
  }

  li_g1_ref(g1_object_class *o);
  
  w32 id() { return _global_id; }
  g1_object_class *value() 
  { 
    if (g1_global_id.check_id(_global_id))
      return g1_global_id.get(_global_id);
    else
      return 0; 
  }  

  static li_g1_ref *get(li_object *o, li_environment *env)
  { 
    check_type(o, li_g1_ref_type_number, env); 
    return ((li_g1_ref *)o); 
  }

  void draw(g1_object_class *start, w32 color, g1_draw_context_class *context);
} ;

li_g1_ref *li_g1_null_ref();

// this li_object stores a list of references to g1_object
// the storage is an array of w32, the first being how many are in list 
// the rest are actual references

extern li_type_number li_g1_ref_list_type_number;
class li_g1_ref_list : public li_object
{
  friend class li_g1_ref_list_function;
  w32 *list;
  void free();
public:
  li_g1_ref_list *clone();
  li_g1_ref_list() 
    : li_object(li_g1_ref_list_type_number) 
  { 
    list=0;
  }


  int size() { if (!list) return 0; else return list[0]; }
  
  w32 get_id(int list_num);
  g1_object_class *value(int list_num);

  void add(w32 id);
  void add(g1_object_class *obj);

  void remove(w32 id);
  void remove(g1_object_class *o);

  // gets rid of invalid object id's that might be in the list
  void compact();

  int find(w32 id);             // return -1 if not found in list, else returns position
  int find(g1_object_class *o); // return -1 if not found in list, else returns position

  static li_g1_ref_list *get(li_object *o, li_environment *env)
  { 
    check_type(o, li_g1_ref_list_type_number, env); 
    return ((li_g1_ref_list *)o); 
  }

  void draw(g1_object_class *start, w32 color, g1_draw_context_class *context);
} ;

struct li_g1_ref_class_member : public li_class_member
{
  li_g1_ref_class_member(char *name) : li_class_member(name) {}
  li_g1_ref *operator()() { return li_g1_ref::get(li_this->get(*this),0); }
};

struct li_g1_ref_list_class_member : public li_class_member
{
  li_g1_ref_list_class_member(char *name) : li_class_member(name) {}
  li_g1_ref_list *operator()() { return li_g1_ref_list::get(li_this->get(*this),0); }
};


#endif
