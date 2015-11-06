/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_OBJ_DLL_HH
#define G1_OBJ_DLL_HH

#error "don't use this anymore"

#include "init/init.hh"
#include "dll/dll.hh"
#include "string/string.hh"

class g1_dll_manager_class : public i4_init_class
{
protected:
  enum {MAX_DLLS=512};

  i4_str **name;
  i4_dll_file **dll;
  w16 dlls;
  w16 current;
  w16 last;
public:
  enum { INVALID=0xffff };

  g1_dll_manager_class() : dlls(0), dll(0), name(0) {}

  w16 current_ref() const { return current; }

  virtual void init();
  virtual void uninit();

  w16 load(const char *s);
  w16 load(const i4_const_str &dll_name);
  void remove(w16 ref);
  void remove_all();
};

extern g1_dll_manager_class g1_dll_man;

void g1_load_object_dlls();
void g1_remove_object_dlls();

#endif
