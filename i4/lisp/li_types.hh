/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef LI_TYPES_HH
#define LI_TYPES_HH



#include "error/error.hh"
#include "lisp/li_alloc.hh"
#include "string/string.hh"
#include "lisp/li_error.hh"

#undef new

#ifdef DEBUG
#define LI_TYPE_CHECK
#endif


// default types available, type numbers for these types shouldn't be changed
enum { 
  LI_INVALID_TYPE,
  LI_SYMBOL,
  LI_STRING,
  LI_INT,
  LI_FLOAT,
  LI_LIST,
  LI_CHARACTER,
  LI_FUNCTION,
  LI_ENVIROMENT,
  LI_TYPE,
  LI_LAST_TYPE
};


class li_environment;
class i4_loader_class;
class i4_saver_class;
typedef w32 li_type_number;

char *li_get_type_name(li_type_number type);

// base object from which all object instances in li_ should be derived
// all objects derived should add 4 bytes to the structure.  li_objects cannot
// contain virtual functions.

class li_object
{  
protected:
  li_type_number _type;
  enum { GC_FLAG=1<<31 };
public:
  i4_bool is_marked() const { if (_type & GC_FLAG) return i4_T; else return i4_F; }
  void mark(int set) { if (set) _type |= GC_FLAG; else _type &= ~GC_FLAG; }
  void mark_free() { _type=LI_INVALID_TYPE; }

  li_object(li_type_number type) { _type=type; }
  li_type_number type() const { return _type; }
  li_type_number unmarked_type() const { return _type & (~GC_FLAG); }

  static void check_type(li_object *o, li_type_number type, li_environment *env)
  {
#ifdef LI_TYPE_CHECK
    if (o->type()!=type)
      li_error(env, "expecting type %s, but got %O",
               li_get_type_name(type),o);
#endif
  }

  void *operator new(size_t size)
  { 
#ifdef DEBUG
    if (size!=8) 
      i4_error(0, "li_objects should be 8 bytes");
#endif
    return li_cell8_alloc(); 
  }

  void *operator new(size_t size, char *file, int line)
  { 
#ifdef DEBUG
    if (size!=8) 
      i4_error(0, "li_objects should be 8 bytes");
#endif
    return li_cell8_alloc(); 
  }

  void operator delete(void *ptr);
};


// the li_ system does not know about gui stuff directly, it goes through this type_edit_class
// which can be added to each type later
class i4_window_class;   
class li_type_edit_class;

// if you want to add a new type into the system, implement one of these
// and call li_add_type
class li_type_function_table
{ 
public:
  li_type_edit_class *editor;

  // mark any data you the type have
  virtual void mark(int set) { ; }

  // mark any data an instance of this type has
  virtual void mark(li_object   *o, int set) { o->mark(set); }

  // free data associated with an instance of this type
  virtual void free(li_object   *o) { ; } // during free, you will not be marked

  virtual int equal(li_object  *o1, li_object *o2) { return o1==o2; }
  virtual void print(li_object  *o, i4_file_class *stream) = 0;
  virtual char *name() = 0;   // name is used to sync types across a network & saves

  virtual li_object *create(li_object *params, li_environment *env) { return 0; }

  // these load and save type information
  virtual void save(i4_saver_class *fp, li_environment *env) { ; }
  virtual void load(i4_loader_class *fp, li_type_number *type_remap, li_environment *env) { ; }
  virtual void load_done() { ; }

  // load & save type instance information
  virtual void save_object(i4_saver_class *fp, li_object *o, li_environment *env) = 0;
  virtual li_object *load_object(i4_loader_class *fp, li_type_number *type_remap,
                                 li_environment *env) = 0;

  li_type_function_table() { editor=0; }
  virtual ~li_type_function_table() { ; }
};

extern li_type_function_table **li_types;

// return type number for type
// if type is not anonymous it's name's symbol value will be set to the type number
int li_add_type(li_type_function_table *type_functions, 
                li_environment *env=0,
                int anonymous=0);

void li_remove_type(int type_num);
void li_cleanup_types();

li_type_function_table *li_get_type(li_type_number type_num);
li_type_number li_find_type(char *name, li_environment *env=0);
li_type_number li_find_type(char *name, li_environment *env, li_type_number &cache_to);
i4_bool li_valid_type(li_type_number type_number);
int li_max_types();    // returns the highest type number currently registered


inline void li_object::operator delete(void *ptr)
{  
  li_object *o = (li_object *)ptr;

  li_get_type(o->type())->free(o);
  li_cell8_free(o);
}

class li_string : public li_object
{
  char *_name;
public:
  li_string(i4_file_class *fp);

  li_string(char *name);
  li_string(int len);
  li_string(const i4_const_str &str);

  char *value() const { return _name; }

  static li_string *get(li_object *o, li_environment *env) 
  { check_type(o, LI_STRING, env); return ((li_string *)o); }
};




class li_symbol : public li_object
{
public:
  struct symbol_data
  {
    li_object *_value;
    li_object *_fun;
    li_string *_name;
    li_symbol *left, *right;
    
    symbol_data(li_string *name) { _value=0; _fun=0; _name=name; left=0; right=0; }
  };

private:
  symbol_data *data;

public:  
  void free();
  li_symbol *left() { return data->left; }
  li_symbol *right() { return data->right; }
  li_symbol *set_left(li_symbol *left) { return data->left=left; }
  li_symbol *set_right(li_symbol *right) { return data->right=right; }

  li_symbol(li_string *name) : li_object(LI_SYMBOL), data(new symbol_data(name)) {}

  li_object *fun() const { return data->_fun; }
  li_object *value() const { return data->_value; }
  li_string *name() const { return data->_name; }
  void set_value(li_object *value) { data->_value=value; }
  void set_fun(li_object *fun) { data->_fun=fun; }

  static li_symbol *get(li_object *o, li_environment *env) 
  { check_type(o, LI_SYMBOL, env); return ((li_symbol *)o); }

  int compare(const li_symbol *a) const { return strcmp(name()->value(), a->name()->value()); }

};


typedef li_object *(*li_function_type)(li_object *o, li_environment *env);

class li_function : public li_object
{
  li_function_type _fun;
public:
  li_function(li_function_type fun)  : li_object(LI_FUNCTION), _fun(fun) {}
  li_function_type value() { return _fun; }
  static li_function *get(li_object *o, li_environment *env) 
  { check_type(o, LI_FUNCTION, env); return (li_function *)o; }
};

class li_int : public li_object
{
  int _x;
public:
  li_int(int x) : li_object(LI_INT) { _x=x; }
  int value() { return _x; }  
  static li_int *get(li_object *o, li_environment *env) 
  { check_type(o, LI_INT, env); return ((li_int *)o); }
} ;


class li_type : public li_object
{
  li_type_number _x;
public:
  li_type(li_type_number x) : li_object(LI_TYPE) { _x=x; }
  li_type_number value() { return _x; }  
  static li_type *get(li_object *o, li_environment *env) 
  { check_type(o, LI_TYPE, env); return ((li_type *)o); }
} ;


class li_float : public li_object
{
  float _x;
public:
  li_float(float x) : li_object(LI_FLOAT) { _x=x; }
  float value() { return _x; }  
  static li_float *get(li_object *o, li_environment *env) 
  { check_type(o, LI_FLOAT, env); return ((li_float *)o); }
} ;

class li_character : public li_object
{
  w32 _ch;
public:
  li_character(w8 ch) : li_object(LI_CHARACTER) {  _ch=ch; }
  w8 value() { return _ch; }  
  static li_character *get(li_object *o, li_environment *env)
  { 
    check_type(o, LI_CHARACTER, env); 
    return ((li_character *)o); 
  }
} ;


class li_list : public li_object
{
  friend class li_memory_manager_class;

  struct list_data
  {
    li_object *_data, *_next;
    list_data(li_object *data, li_object *next) : _data(data), _next(next) {}
  } *_data;

  li_list *get_next_free() { return (li_list *)_data; }
  void set_next_free(li_list *next_free) { _data=(list_data *)next_free; }

public: 
  void cleanup() { delete _data; }

 

  li_list(li_object *data, li_object *next=0) 
    : li_object(LI_LIST), _data(new list_data(data,next)) {}

  li_object *data() { return _data->_data; }
  li_object *next() { return _data->_next; }

  void set_next(li_object *next) { _data->_next=next; }
  void set_data(li_object *data) { _data->_data=data; }

  static li_list *get(li_object *o, li_environment *env) 
  { check_type(o, LI_LIST,env); return (li_list *)o; }
};


class li_environment : public li_object
{ 
  struct value_data
  {
    li_symbol *symbol;
    li_object *value;
    value_data *next;
  };

  struct fun_data
  {
    li_symbol *symbol;
    li_object *fun;
    fun_data  *next;
  };

  struct env_data
  {
    value_data     *value_list;
    fun_data       *fun_list;
    li_environment *next;
    li_symbol      *current_function;   // set by li_call
    li_object      *current_args;
    i4_bool        local_namespace;


    env_data(li_environment *top_env, i4_bool local_namespace) 
      : next(top_env), local_namespace(local_namespace)
    
    { value_list=0; fun_list=0; current_function=0; current_args=0; }

  } *data;


public:
  li_symbol *&current_function();
  li_object *&current_arguments();


  li_environment(li_environment *top_env, i4_bool local_namespace)
    : li_object(LI_ENVIROMENT), data(new env_data(top_env, local_namespace)) {}

  li_object *value(li_symbol *s);
  li_object *fun(li_symbol *s);

  void set_value(li_symbol *s, li_object *value);
  void set_fun(li_symbol *s, li_object *fun);


  // don't call these functions directly!
  void mark(int set);
  void print(i4_file_class *stream);
  void free();

  static li_environment *get(li_object *o, li_environment *env)
  { check_type(o, LI_ENVIROMENT, env); return (li_environment *)o; }

  void print_call_stack(i4_file_class *fp);
};


#include "memory/new.hh"      // make sure old definition of new is redeclared

#endif
