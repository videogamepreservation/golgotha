/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "main/main.hh"
#include "init/init.hh"
#include "lisp/lisp.hh"
#include "lisp/li_init.hh"

li_object *add(li_object *o, li_environment *env)
{
  int int_sum=0, return_float=0;
  float float_sum=0;

  while (o)     // while there are still more parameters in the list
  {
    // li_car get's the data for this node in the parameter list
    // i.e. o->data()
    // li_eval evaluates the object if it's not already a number
    li_object *item=li_eval(li_car(o, env), env); 



    if (item->type()==LI_FLOAT || return_float)   // adding floats or ints?    
    {
      return_float=1;
      float_sum += li_get_float(item, env);
    }
    else
      int_sum += li_get_int(item,env);
      
    o=li_cdr(o,env);      // go to the next parameter
  }

  if (return_float) 
    return new li_float(float_sum);
  else
    return new li_int(int_sum);
}


li_object *sub(li_object *o, li_environment *env)
{
  float x1=li_get_float(li_eval(li_first(o,env), env),env);
  float x2=li_get_float(li_eval(li_second(o,env), env),env);

  return new li_float(x1-x2);
}


// this method of adding functions is the automatic way.  sub will be added during i4_init
// through c++ magic.
li_automatic_add_function(sub, "sub");

void i4_main(w32 argc, i4_const_str *argv)
{
  i4_init();
  
  // this is the manual way to add functions
  li_add_function("add", add);

  li_load("functions.scm");  // loads & evaluates from current directory

  i4_uninit();
}

