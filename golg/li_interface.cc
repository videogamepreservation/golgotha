/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "lisp/li_init.hh"
#include "g1_object.hh"

static li_object *li_get_object_type(li_object *o, li_environment *env)
{
  return new li_int(g1_get_object_type(li_string::get(li_eval(li_car(o,env), env),env)->value()));
}

li_automatic_add_function(li_get_object_type, "get_object_type");

