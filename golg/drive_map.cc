/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "file/file_man.hh"
#include "string/string.hh"

class g1_drive_map : public i4_file_manager_class
{  
  i4_str *sub, *replace_with;

public:
  i4_str *get(const i4_const_str &s)
  {
    i4_const_str::iterator i=s.strstr(*sub);    
    if (i!=s.end())
    {
      int rlen=s.length() + replace_with->length();
      i4_str *ret= new i4_str(s, rlen);

      i4_str::iterator start=ret->begin();     
      start+=s.ptr_diff(s.begin(), i);
      
      i4_str::iterator end=start;
      end+=sub->length();

      ret->remove(start, end);
      ret->insert(start, *replace_with);
      
      return ret;
    }    
    else return 0;
  }
  

  g1_drive_map(const i4_const_str &sub_path, const i4_const_str &replace_with)
    : sub(new i4_str(sub_path)),
      replace_with(new i4_str(replace_with))
  {

  }

  ~g1_drive_map()
  {
    delete sub;
    delete replace_with;
  }

  virtual i4_bool unlink(const i4_const_str &name)
  {    
    i4_str *s=get(name);
    if (s)
    {
      i4_bool ret=next->unlink(*s);
      delete s;
      return ret;
    }
    else
      return i4_F;
  }

  virtual i4_file_class *open(const i4_const_str &name, w32 flags)
  {
    i4_str *s=get(name);
    if (s)
    {
      i4_file_class *ret=next->open(*s, flags);
      delete s;
      return ret;
    }
    else
      return 0;
  }

  i4_bool get_status(const i4_const_str &filename, 
                             i4_file_status_struct &return_stat)
  {
    i4_str *s=get(filename);
    if (s)
    {
      i4_bool ret=next->get_status(*s, return_stat);
      delete s;
      return ret;
    }
    else
      return i4_F;
  }

  i4_bool mkdir(const i4_const_str &name)
  {
    i4_str *s=get(name);
    if (s)
    {
      i4_bool ret=next->mkdir(*s);
      delete s;
      return ret;
    }
    else
      return i4_F;
  }

  virtual i4_bool get_directory(const i4_const_str &path, 
                                i4_str **&files, w32 &tfiles, 
                                i4_str **&dirs, w32 &tdirs,
                                        i4_file_status_struct **file_status)
  {
    i4_str *s=get(path);
    if (s)
    {
      i4_bool ret=next->get_directory(*s, files, tfiles, dirs, tdirs, file_status);
      delete s;
      return ret;
    }
    else
      return i4_F;
  }
} ;




class g1_drive_map_init : public i4_init_class
{
  g1_drive_map *x, *X, *c, *alpha1;

public:
  int init_type() { return I4_INIT_TYPE_FILE_MANAGER; }

  void init()
  {
    x=new g1_drive_map(i4_const_str("x:/"), i4_const_str("/u/"));
    i4_add_file_manager(x, i4_T);

    X=new g1_drive_map(i4_const_str("X:/"), i4_const_str("/u/"));
    i4_add_file_manager(X, i4_T);

    c=new g1_drive_map(i4_const_str("c:/tmp/"), i4_const_str("/tmp/"));
    i4_add_file_manager(c, i4_T);

    alpha1=new g1_drive_map(i4_const_str("//alpha1/"), i4_const_str("/u/"));
    i4_add_file_manager(alpha1, i4_T);
  }

  void uninit()
  {
    i4_remove_file_manger(x);
    i4_remove_file_manger(X);
    i4_remove_file_manger(c);
    i4_remove_file_manger(alpha1);
  } 

} g1_drive_map_init_instance;















