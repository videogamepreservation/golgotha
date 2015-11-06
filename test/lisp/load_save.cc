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
#include "lisp/li_load.hh"
#include "loaders/dir_load.hh"
#include "loaders/dir_save.hh"

char *data_file="load_save.data";

void save(li_object *o)
{
  i4_file_class *fp=i4_open(data_file, I4_WRITE);
  i4_saver_class *saver_fp=new i4_saver_class(fp);

  // save type info about objects
  li_save_type_info(saver_fp,0);

  // save the actual object
  li_save_object(saver_fp, o,0);
  
  // first pass just marks sizes, call save again to actually write out code
  saver_fp->begin_data_write();  

  li_save_type_info(saver_fp,0);
  li_save_object(saver_fp, o,0);

  delete saver_fp;
}


li_object *load()
{
  i4_file_class *fp=i4_open(data_file);
  if (fp)
  {
    i4_loader_class *loader=new i4_loader_class(fp);

    // remap remaps li_ type numbers to the current types available
    li_type_number *remap=li_load_type_info(loader,0);

    // load the object
    li_object *ret=li_load_object(loader,remap,0);

    delete loader;
    return ret;
  }
  return 0;
}


void i4_main(w32 argc, i4_const_str *argv)
{
  i4_init();

  // allocate a new li_string
  li_string *li_str=new li_string("Testing,  1... 2... 3..,  Testing");

  // save the object to disk
  save(li_str);

  // load an object up
  li_object *loaded_object=load();

  // print out the object
  lip(loaded_object);


  i4_uninit();
}

