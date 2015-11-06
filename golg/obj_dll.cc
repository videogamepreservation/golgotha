/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#error this file is not USED!~

#include "obj_dll.hh"
#include "file/file.hh"
#include "memory/malloc.hh"

g1_dll_manager_class g1_dll_man;

void g1_dll_manager_class::init()
{
  // allocate entries
  dll = (i4_dll_file **)i4_malloc(MAX_DLLS*sizeof(i4_dll_file *), "dll files");
  name = (i4_str **)i4_malloc(MAX_DLLS*sizeof(i4_str *), "dll names");
  dlls = last = 0;
}

void g1_dll_manager_class::uninit()
{
  if (dlls>0)
    i4_warning("%d dlls not freed!",dlls);

  i4_free(dll);
  i4_free(name);
}

w16 g1_dll_manager_class::load(const char *buff)
{
  i4_dll_file *new_dll;
  // find next open entry
  if (dll[last])
  {
    w16 end = last;
    do 
    {
      if (++last==MAX_DLLS)
        last = 0;;
    } while (last!=end && dll[last]);
    if (last == end)
      i4_error("Ran out of dll space");
  }

  // set global reference & load dll
  current = last;

  i4_warning("loading [%s].\n",buff);
  new_dll = i4_open_dll(buff);

  current = INVALID;

  if (!new_dll)
  {
    i4_warning("couldn't load dll [%s].", buff);
    return INVALID;
  }

  dll[last] = new_dll;
  name[last] = 0;
  dlls++;

  return last;
}

w16 g1_dll_manager_class::load(const i4_const_str &dll_name)
{
  char buff[256];
  w16 ret;
  i4_os_string(dll_name,buff,sizeof(buff));
  ret = load(buff);
  name[ret] = new i4_str(dll_name);
  return ret;
}

void g1_dll_manager_class::remove(w16 ref)
{
  // sanity check
  if (ref>=MAX_DLLS || !dll[ref])
  {
    i4_warning("Tried to remove invalid DLL.");
    return;
  }

  delete dll[ref];
  dll[ref] = 0;
  if (name[ref])
    delete name[ref];
  name[ref] = 0;
  last = ref;
}

void g1_dll_manager_class::remove_all()
{
  for (w16 i=0; i<MAX_DLLS; i++)
    if (dll[i])
    {
      delete dll[i];
      dll[i] = 0;
      if (name[i])
        delete name[i];
      name[i] = 0;
    }
  last = 0;
  dlls = 0;
}

void g1_load_object_dlls()
{
  w32 files, dirs, f,d;
  i4_str *fullname;
  i4_str **file, **dir;


#ifdef __linux
  i4_const_str dll_dir=i4gets("linux_dll_dir");
#elif _WINDOWS
  i4_const_str dll_dir=i4gets("win32_dll_dir");
#endif

  i4_file_man.get_directory(dll_dir, file,files, dir,dirs);

  i4_const_str dll_extension(i4gets("dll_extension"));
  for (f=0; f<files; f++)
  {

    int is_debug=name->strstr(i4gets("debug_dll_substr"))!=name->end();

    if (extension && *extension == dll_extension &&
#ifdef DEBUG
        is_debug
#else
        !is_debug
#endif
        )

    {
      fullname = i4gets("dll_fullname").sprintf(256, &i4gets("obj_dll_dir"), file[f]);
      g1_dll_man.load(*fullname);
      delete fullname;
    }

    if (path)
      delete path;
    if (name)
      delete name;
    if (extension)
      delete extension;
  }

  for (f=0; f<files; f++)
    delete file[f];
  if (files)
    i4_free(file);

  for (d=0; d<dirs; d++)
    delete dir[d];
  if (dir)
    i4_free(dir);
}

void g1_remove_object_dlls()
{
  g1_dll_man.remove_all();
}
