/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "main/main.hh"
#include "lisp/lisp.hh"
#include "lisp/li_dialog.hh"
#include "app/app.hh"
#include "error/error.hh"


li_object *test_quit(li_object *o, li_environment *env)
{
  i4_debug->printf("the new value is ");
  lip(li_car(o,env));

  i4_debug->printf("\nthe old value is ");
  lip(li_second(o,env));

  return 0;
}

class dialog_app : public i4_application_class
{

public:
  enum { QUIT };

  void init()
  {
    i4_application_class::init();
    li_load("dialog.scm");
    li_create_dialog("Editing Options", li_new("options"), 0, test_quit);
  }  

  char *name() { return "dialog_app"; }
};

void i4_main(w32 argc, i4_const_str *argv)
{
  dialog_app test;
  test.run();
}


