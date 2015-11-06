/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef DRIVE_MAP_HH
#define DRIVE_MAP_HH

#include "file/file.hh"
#include "string/string.hh"

class i4_drive_map : public i4_file_manager_class
{  
protected:
  i4_str *drive_path;

public:

  i4_drive_map(const i4_const_str &path) 
  //{{{
  {
    drive_path=new i4_str(path,path.length()+1);
  }
  //}}}


  virtual i4_file_class *default_open(const i4_const_str &name, w32 flags)
  //{{{
  {
    i4_str *cat=new i4_str(*drive_path,drive_path->length()+name.length()+1);
    cat->insert(cat->end(),name);
    i4_file_class *fp=i4_file_manager_class::default_open(*cat,flags);
    delete cat;
    return fp;
  }
  //}}}


  virtual i4_bool get_status(const i4_const_str &filename, 
                             i4_file_status_struct &return_stat)
  //{{{
  {
    i4_str *cat=new i4_str(*drive_path,drive_path->length()+filename.length()+1);
    cat->insert(cat->end(),filename);
    i4_bool ret=i4_file_manager_class::get_status(*cat,return_stat);
    delete cat;
    return ret;
  }
  //}}}


  virtual i4_bool mkdir(const i4_const_str &name)
  //{{{
  {
    i4_str *cat=new i4_str(*drive_path,drive_path->length()+name.length()+1);
    cat->insert(cat->end(),name);
    i4_bool ret=i4_file_manager_class::mkdir(*cat);
    delete cat;
    return ret;
  }
  //}}}
};

#endif

//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}
