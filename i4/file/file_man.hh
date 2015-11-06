/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef I4_FILE_MAN_HH
#define I4_FILE_MAN_HH

#include "file/file.hh"
#include "init/init.hh"

// see file/file.hh for a description of what each of these functions do
class i4_file_manager_class : public i4_init_class
{
public:
  int init_type() { return I4_INIT_TYPE_FILE_MANAGER; }

  static i4_file_manager_class *first;
  i4_file_manager_class *next;

  virtual i4_file_class *open(const i4_const_str &name, w32 flags=I4_READ) { return 0; }

  virtual i4_bool unlink(const i4_const_str &name) { return i4_F; }

  virtual i4_bool mkdir(const i4_const_str &name) { return i4_F; }

  virtual i4_bool get_status(const i4_const_str &filename, i4_file_status_struct &return_stat)
  { return i4_F; }

  virtual i4_bool get_directory(const i4_const_str &path, 
                                i4_directory_struct &dir_struct,
                                i4_bool get_status=i4_F,
                                i4_status_class *status=0) { return i4_F; }

  
  virtual i4_bool split_path(const i4_const_str &name, i4_filename_struct &fn);

  virtual i4_str *full_path(const i4_const_str &relative_name) { return 0; }
};


void i4_add_file_manager(i4_file_manager_class *fman, i4_bool add_front);
void i4_remove_file_manger(i4_file_manager_class *fman);

#endif
