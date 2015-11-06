/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef LI_CLASS_HH
#define LI_CLASS_HH


#include "lisp/lisp.hh"
#include "memory/fixed_array.hh"


class li_class_type;
struct sym_var;


struct li_class_member
{
  char *name;
  int offset;
  li_type_number class_type;
  li_symbol *sym;

  li_class_member(char *name) : name(name), sym(0), class_type(0) {}
};

struct li_string_class_member : public li_class_member
{
  li_string_class_member(char *name) : li_class_member(name) {}
  char *operator()();
};

struct li_int_class_member : public li_class_member
{
  li_int_class_member(char *name) : li_class_member(name) {}
  int &operator()();
};


struct li_float_class_member : public li_class_member
{
  li_float_class_member(char *name) : li_class_member(name) {}
  float &operator()();
};


struct li_symbol_class_member : public li_class_member
{
  li_symbol_class_member(char *name) : li_class_member(name) {}
  li_symbol *&operator()();
};


struct li_object_class_member : public li_class_member
{
  li_object_class_member(char *name) : li_class_member(name) {}
  li_object *&operator()();
};


class li_class : public li_object
{
  void **values;
  li_class_type *get_type() const { return (li_class_type *)li_get_type(type()); }
public:
  void mark(int set); 

  void save(i4_saver_class *fp, li_environment *env);
  void load(i4_loader_class *fp, li_type_number *type_remap, li_environment *env);
  void print(i4_file_class *fp);

  li_class(li_type_number class_type, li_object *params=0, li_environment *env=0);

  void free();

  int member_offset(char *sym) const;    // does not type check
  int member_offset(li_symbol *sym) const;    // does not type check

  int get_offset(li_class_member &m) const;
  int get_offset(li_class_member &m, li_type_number _type) const;

  int member_offset(li_class_member &m) const   // inline version checks to see to see if cached
  {
    if (m.class_type!=type())
      get_offset(m);
    return m.offset;
  }

  int member_offset(li_class_member &m, li_type_number _type) const
  {
    if (m.class_type!=type())
      get_offset(m, _type);
    return m.offset;
  }



  char *string_value(int member) const { return ((li_string *)values[member])->value(); }
  int &int_value(int member) { return *(((int *) (values+member))); }
  float &float_value(int member) { return *(((float *) (values+member))); }
  li_symbol *&symbol_value(int member)  { return *((li_symbol **)(values+member)); }
  li_object *&object_value(int member)  { return *((li_object **)(values+member)); }

  char *get(li_string_class_member &c) const { return string_value(member_offset(c, LI_STRING)); }
  int &get(li_int_class_member &c)  {  return int_value(member_offset(c, LI_INT)); }
  float &get(li_float_class_member &c) { return float_value(member_offset(c, LI_FLOAT)); }
  li_symbol *&get(li_symbol_class_member &c) { return symbol_value(member_offset(c, LI_SYMBOL)); }
  li_object *&get(li_class_member &c) { return object_value(member_offset(c)); }


  li_object *value(char *member_name);
  li_object *value(int member);
  void set_value(int member, li_object *value);
  void set(li_class_member &c, li_object *value) { set_value(member_offset(c), value); }
  li_object *set(char *member_name, li_object *value); // slow, but easy way to access data

#ifdef LI_TYPE_CHECK
  static li_class *get(li_object *o, li_environment *env);    
#else
  static li_class *get(li_object *o, li_environment *env)  { return ((li_class *)o); }
#endif


};

li_object *li_def_class(li_object *fields, li_environment *env);

int li_class_total_members(li_type_number type);
li_symbol *li_class_get_symbol(li_type_number type, int member_number);
li_object *li_class_get_default(li_type_number type, li_symbol *sym);
li_object *li_class_get_property_list(li_type_number type, li_symbol *sym);
void li_set_class_editor(li_type_edit_class *editor);

extern li_class *li_this;

inline char *li_string_class_member::operator()() { return li_this->get(*this); }
inline int &li_int_class_member::operator()() { return li_this->get(*this); }
inline float &li_float_class_member::operator()() { return li_this->get(*this); }
inline li_symbol *&li_symbol_class_member::operator()() { return li_this->get(*this); }
inline li_object *&li_object_class_member::operator()() { return li_this->get(*this); }

class li_class_context
{
  li_class *old_context;
public:
  li_class_context(li_class *current_context) 
  { 
    old_context=li_this;
    li_this=current_context; 
  }

  ~li_class_context() { li_this=old_context; }
};

#endif
