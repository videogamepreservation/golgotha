/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef I4_DLL_MAN_HH
#define I4_DLL_MAN_HH

#include "init/init.hh"
#include "dll/dll.hh"
#include "string/string.hh"
#include "isllist.hh"

class i4_dll_manager_class : public i4_init_class
{
protected:
  struct dll_node
  {
    dll_node      *next;
    i4_str        *name;
    i4_dll_file   *dll;
    i4_init_class *init_list;  // list of all i4_init_class'es in the dll
    ~dll_node();
  };

  virtual int init_type() { return I4_INIT_TYPE_DLLS; }
  i4_isl_list<dll_node> dll_list;

  virtual void init();     // called by i4_init
  virtual void uninit();

public:
  i4_bool load(const i4_const_str &dll_name, i4_bool prepend_dll_dir=0);
  i4_bool is_loaded(const i4_const_str &dll_name);
  i4_bool unload(const i4_const_str &dll_name);  
};

extern i4_dll_manager_class i4_dll_man;

extern char *i4_dll_dir;

#endif
