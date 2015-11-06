/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "lisp/li_init.hh"
#include "lisp/lisp.hh"
#include "error/error.hh"
#include "time/profile.hh"


li_object *li_profile(li_object *o, li_environment *env)
{
  li_object *r=li_eval(li_car(o,env),env);
  if (r->type()!=LI_STRING)
    i4_warning("profile expects a string arg");
  else
  {
    i4_profile_clear();
    i4_profile_stack_top=0;

    for (i4_profile_class *c=i4_profile_class::list; c; c=c->next)
    {
      if (strstr(c->name, li_get_string(r,env))!=0)
        c->active=1;
      else
        c->active=0;
    }
  }

  
  return 0;
}

li_object *li_profile_report(li_object *o, li_environment *env)
{
  li_object *r=li_eval(li_car(o,env),env);
  if (r->type()!=LI_STRING)
    i4_warning("profile expects a string arg");
  else
    i4_profile_report(li_get_string(r,env));

  
  return 0;
}


li_automatic_add_function(li_profile,"profile");
li_automatic_add_function(li_profile_report,"profile_report");
