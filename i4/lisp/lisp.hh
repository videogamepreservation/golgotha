/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef LI_LISP_HH
#define LI_LISP_HH

#include "lisp/li_types.hh"
#include "lisp/li_optr.hh"

class i4_status_class;

// this is used for debugging... it doesn't evaluate o
void lip(li_object *o);

// this is the function added into the system, arguments are evaluated
li_object *li_print(li_object *o, li_environment *env=0);

li_object *li_get_expression(char *&s, li_environment *env);

li_object *li_eval(li_object *expression, li_environment *env=0);
li_object *li_call(char *fun_name, li_object *params=0, li_environment *env=0);
li_object *li_call(li_symbol *fun_name, li_object *params=0, li_environment *env=0);

// if env is 0, global function is set
void li_add_function(li_symbol *sym, li_function_type fun, li_environment *env=0); 
void li_add_function(char *sym_name, li_function_type fun, li_environment *env=0);

li_symbol *li_find_symbol(const char *name);    // if symbol doesn't exsist, 0 is returned
li_symbol *li_get_symbol(const char *name);     // if symbol doesn't exsist, it is created

// if cache_to is 0, then the symbol is found and stored there, otherwise cache_to is returned
li_symbol *li_get_symbol(char *name, li_symbol *&cache_to);


inline li_object *li_get_value(li_symbol *sym, li_environment *env=0)
{ 
  if (env)
    return env->value(sym);
  else return sym->value();
}

inline li_object *li_get_value(char *sym, li_environment *env=0) 
{ return li_get_value(li_get_symbol(sym),env); }

inline void li_set_value(li_symbol *sym, li_object *value, li_environment *env=0)
{ 
  if (env)
    env->set_value(sym, value);
  else sym->set_value(value);  
}

inline void li_set_value(char *sym, li_object *value, li_environment *env=0) 
{ li_set_value(li_get_symbol(sym),value,env); }
  

// gets a function for a symbol.  Checks to see if it has a local value in the environment
// first
li_object *li_get_fun(li_symbol *sym, li_environment *env);

li_object *li_get_fun(char *sym, li_environment *env);
  
// return 0 if type doesn't exsist
li_object *li_new(char *type_name, li_object *params=0, li_environment *env=0);  
li_object *li_new(int type, li_object *params=0, li_environment *env=0);  


// CAR comes from old LISP machines which stands for Contents of the Address Register
// though lisp machines aren't used anymore, the term has stuck around. 
inline li_object *li_car(li_object *o, li_environment *env) { return li_list::get(o,env)->data(); }

// CDR = Contents of the Destination Register.  or better know by "C" people as ->next
inline li_object *li_cdr(li_object *o, li_environment *env) { return li_list::get(o,env)->next(); }

i4_bool    li_get_bool(li_object *o, li_environment *env);   // return i4_T if o is 'T,  i4_F if it is 'F or nil
float      li_get_float(li_object *o, li_environment *env);  // will convert int to float
int        li_get_int(li_object *o, li_environment *env);    // will convert float to int
char      *li_get_string(li_object *o, li_environment *env); 
i4_bool    li_is_number(li_object *o, li_environment *env);  // returns true if item is LI_INT or LI_FLOAT

// these function return the 1st, 2nd...  item in a list first=li_cdr(o), second=li_cdr(li_cdr(o))
li_object  *li_first(li_object *o, li_environment *env);
li_object  *li_second(li_object *o, li_environment *env);
li_object  *li_third(li_object *o, li_environment *env);
li_object  *li_fourth(li_object *o, li_environment *env);
li_object  *li_fifth(li_object *o, li_environment *env);
li_object  *li_nth(li_object *o, int x, li_environment *env);

// li_make_list takes a variable @ of arguments (null terminated) and return a list of
// add the members : example li_make_list(new li_int(4), new li_string("hello"), 0);
li_list   *li_make_list(li_object *first, ...);
int        li_length(li_object *o, li_environment *env);     // returns the length of the list

// this version of load take a list of li_strings which are loaded sequentially
// return result of last evaluated expression
li_object *li_load(li_object *name, li_environment *env, i4_status_class *status);
li_object *li_load(li_object *name, li_environment *env=0);

// this loads a single file and returns result of last evaluated expression
li_object *li_load(char *filename, li_environment *env=0, i4_status_class *status=0);

// loads from a file pointer, reads entire buffer
li_object *li_load(i4_file_class *fp, li_environment *env=0, i4_status_class *status=0);

// reads an expression from i4_debug and prints the result of evaluation to i4_debug
li_object *li_read_eval(li_object *o, li_environment *env=0);

// simply add an item to the begining of a list
inline void li_push(li_list *&l, li_object *o) { l=new li_list(o, l); }

// global symbols
extern li_symbol *li_nil, 
  *li_true_sym, 
  *li_quote, 
  *li_backquote,
  *li_comma,
  *li_function_symbol;


class li_symbol_ref
{
  char *name;
  li_symbol *sym;
public:
  li_symbol *get() { if (!sym) sym=li_get_symbol(name); return sym; }
  i4_bool operator==(const li_symbol *&b) { return (get() == b); }

  li_symbol_ref(char *sym_name) : name(sym_name) { sym=0; }
};

#endif
