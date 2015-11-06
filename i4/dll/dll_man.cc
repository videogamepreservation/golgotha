/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "dll/dll_man.hh"
#include "file/file.hh"
#include "memory/malloc.hh"
#include "error/alert.hh"
#include <stdio.h>

i4_dll_manager_class i4_dll_man;

#ifdef __linux
char *i4_dll_dir="linux_dlls";
#elif _WINDOWS
char *i4_dll_dir="win32_dlls";
#endif

void i4_dll_manager_class::init()
{
  i4_directory_struct ds;
  if (i4_get_directory(i4_const_str(i4_dll_dir), ds))
  {

    i4_const_str dll_extension(i4_const_str("dll"));
    i4_const_str debug_substr(i4_const_str("_debug"));

    for (int f=0; f<ds.tfiles; f++)
    {
      i4_filename_struct fn;
      i4_split_path(*ds.files[f], fn);

      int is_debug=(strstr(fn.filename, "_debug")==0) ? 0 : 1;

      if (fn.extension[0] && strcmp(fn.extension, "dll")==0 &&
#ifdef DEBUG
          is_debug
#else
          !is_debug
#endif
          )
      {
        i4_str *fullname = i4_const_str("%s/%S").sprintf(512, i4_dll_dir, ds.files[f]);
        load(*fullname);
        delete fullname;
      }
    }
  }
}

void i4_dll_manager_class::uninit()
{
  while (dll_list.begin()!=dll_list.end())
  {
    dll_node *i=&*dll_list.begin();

    i4_init_class *old_first_init=i4_init_class::first_init;
    i4_init_class::first_init=i->init_list;

    for (i4_init_class *f=i->init_list; f; f=f->next_init)
      f->uninit();

    dll_list.erase();
    delete i;

    i4_init_class::first_init=old_first_init;
  }

  dll_list.destroy_all();
}

i4_bool i4_dll_manager_class::load(const i4_const_str &filename, i4_bool prepend_dll_dir)
{
  i4_const_str s=filename;
  char tmp1[512],tmp2[512];
  if (prepend_dll_dir)
  {
    sprintf(tmp2, "%s/%s",i4_dll_dir,i4_os_string(filename, tmp1, 512));
    return load(i4_const_str(tmp2));
  }

  i4_str *full=i4_full_path(filename);

  i4_alert(i4_const_str("Loading DLL [%S]"),512, full);

  if (is_loaded(*full))  
    unload(*full);

  // add this to the dll list
  dll_node *node=new dll_node;
  node->name=full;

  i4_init_class *old_first_init=i4_init_class::first_init;
  i4_init_class::first_init=0;

  node->dll=i4_open_dll(*full);

  node->init_list=i4_init_class::first_init;
  i4_init_class::first_init=old_first_init;

  if (!node->dll)
  {
    delete node;
    return i4_F;
  }

  for (i4_init_class *f=node->init_list; f; f=f->next_init)
    f->init();

  dll_list.insert(*node);
  return i4_T;
}

i4_bool i4_dll_manager_class::is_loaded(const i4_const_str &filename)
{
  for (i4_isl_list<dll_node>::iterator i=dll_list.begin(); i!=dll_list.end(); ++i)
    if (*i->name == filename)
      return i4_T;
  return i4_F;
}


i4_bool i4_dll_manager_class::unload(const i4_const_str &filename)
{
  for (i4_isl_list<dll_node>::iterator i=dll_list.begin(); i!=dll_list.end(); ++i)
    if (*i->name == filename)
    {
      for (i4_init_class *f=i->init_list; f; f=f->next_init)
        f->uninit();
      i->init_list=0;

      delete dll_list.find_and_unlink(&*i);
      return i4_T;
    }

  return i4_F;
}

i4_dll_manager_class::dll_node::~dll_node()
{
  if (name) delete name;
  if (dll) delete dll;
}
