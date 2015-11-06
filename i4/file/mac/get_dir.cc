/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "memory/malloc.hh"
#include "dirent.hh"
#include "file/get_dir.hh"

class i4_dir_string : public i4_str
//{{{
{
public:
  i4_dir_string(StringPtr st) : i4_str(st[0])
  {
    len=st[0];
    memcpy(ptr,st,len);
  }

  char *buffer() { return ptr; }
  void set_len(w16 _len) { len = _len; }
};
//}}}


extern char *i4_os_string(const i4_const_str &name);


i4_bool i4_get_directory(const i4_const_str &path, 
                      i4_str **&files, w32 &tfiles,
                      i4_str **&dirs, w32 &tdirs)
//{{{
{
  struct dirent *de;
  files=NULL;
  dirs=NULL;
  tfiles=0;
  tdirs=0;
  DIR *d=opendir(i4_os_string(path));
  if (!d) return i4_F;

  i4_dir_string **tlist=NULL;
  sw32 t=0;
  char curdir[256];
  getwd(curdir);
  if (chdir(i4_os_string(path))!=0)
    return i4_F;

  do
  {
    de=readdir(d);
    if (de)
    {
      t++;
      tlist=(i4_dir_string **)i4_realloc(tlist,sizeof(i4_dir_string *)*t,"tmp file list");
      tlist[t-1]=new i4_dir_string(de->d_name);
    }
  } while (de);
  closedir(d);

  for (int i=0;i<t;i++)
  {
    d=opendir(((i4_dir_string *)tlist[i])->buffer() );   // see if the file is a directory
    if (d)
    {
      tdirs++;
      dirs=(i4_str **)i4_realloc(dirs,sizeof(i4_str *)*tdirs,"dir list");
      dirs[tdirs-1]=tlist[i];
      closedir(d);
    } else
    {
      tfiles++;
      files=(i4_str **)i4_realloc(files,sizeof(i4_str *)*tfiles,"dir list");
      files[tfiles-1]=tlist[i];
    }
  }
  if (t)
    i4_free(tlist);
  chdir(curdir);
  return i4_T;
}
//}}}


//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}







