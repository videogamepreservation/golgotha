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
#include "lisp/li_class.hh"
#include "file/ram_file.hh"

static li_int_class_member x("x"), y("y");

void i4_main(w32 argc, i4_const_str *argv)
{
  i4_init();

  // we are going to print everything to a ram file and then dump it all out at the
  // end with i4_warning
  char output_buf[3001];
  i4_ram_file_class fp(output_buf, 3000);
  
  li_load("class.scm");  // loads & evaluates from current directory

  // find the type that was created by the script
  li_type_number type=li_find_type("test_class");

  // create a new class instance from scratch
  // li_get_type fetch the class with type virtual functions
  // then we call create with null parameters
  // finally we cast the result into a li_class
  li_class *c1=li_class::get(li_get_type(type)->create(0,0),0);

  // go through all the members in the class and print out their values
  int total_members=li_class_total_members(type);
  for (int i=0; i<total_members; i++)
  {
    
    fp.printf("member %s : value = ", li_class_get_symbol(type, i)->name()->value());

    li_object *value=c1->value(i);
    li_get_type(value->type())->print(value, &fp);
    
    fp.printf("\n");
  }

  // now print out the class instance defined in the script file and assigned to "test"
  li_class *c2=li_class::get(li_get_value("test"),0);

  fp.printf("test = ");
  li_get_type(type)->print(c2, &fp);
  fp.printf("\n");


  // now let's change a variable in the class using a slower method
  c2->set_value(c2->member_offset("my_string"), new li_string("Ah ha! A new string."));
  
  // now let's change a member using a faster cahching method
  li_this=c2;     // need to setup this correct before we operate on the object
  fp.printf("x old value was %d\n", x());
  x()=y();
  fp.printf("x new value is %d\n", x());


  fp.printf("test = ");
  li_get_type(type)->print(c2, &fp);
  fp.printf("\n");



  // now flush out the ram file
  output_buf[fp.tell()]=0;
  i4_warning(output_buf);

  i4_uninit();
}

