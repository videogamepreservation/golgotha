/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "file/ram_file_man.hh"
#include "file/ram_file.hh"
#include "file/file_man.hh"
#include "string/string.hh"

i4_openable_ram_file_info *i4_flist=0;

class i4_ram_file_manager_class : public i4_file_manager_class
{  
public: 

  virtual i4_file_class *open(const i4_const_str &name, w32 flags)
  {
    if (flags==I4_READ)
    {
      char buf[256];
      i4_os_string(name, buf, 256);

      for (i4_openable_ram_file_info *f=i4_flist; f; f=f->next)
        if (strcmp(buf, f->filename)==0)
          return new i4_ram_file_class(f->data, f->data_size);         
    }

    return 0;
  }

  i4_ram_file_manager_class()
  {
    i4_add_file_manager(this, i4_F);
  }

  ~i4_ram_file_manager_class()
  {
    i4_remove_file_manger(this);
  }
} i4_ram_file_manager_instance;

i4_openable_ram_file_info::i4_openable_ram_file_info(char *filename, void *data, w32 data_size)
  : filename(filename), data(data), data_size(data_size)
{ 
  next=i4_flist;
  i4_flist=this;
}
