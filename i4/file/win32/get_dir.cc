/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include <io.h>
#include "string/string.hh"
#include "arch.hh"
#include "file/get_dir.hh"
#include "memory/array.hh"

class i4_dir_string : public i4_str
{
public:
  i4_dir_string(char *st) : i4_str(strlen(st)+1)
  {
    strcpy(ptr,st);
    len=strlen(st);
  }

  char *buffer() { return ptr; }
  void set_len(w16 _len) { len = _len; }
};

i4_bool i4_get_directory(const i4_const_str &path, 
                         i4_str **&files, w32 &tfiles,
                         i4_str **&dirs, w32 &tdirs,
                         i4_file_status_struct **file_status)
{
  files=NULL;
  dirs=NULL;
  tfiles=0;
  tdirs=0;

  _finddata_t fdat;
  char os_str[255],buf[256];
  sprintf(os_str,"%s/*.*",i4_os_string(path,buf,sizeof(buf)));

  long handle=_findfirst(os_str,&fdat),done;
  if (handle==-1)
    return i4_F;
  
  i4_array<i4_file_status_struct> stats(64,64);

  do
  {
    if (fdat.attrib & _A_SUBDIR)
    {
      tdirs++;
      dirs=(i4_str **)i4_realloc(dirs,sizeof(i4_str *)*tdirs,"dir list");
      dirs[tdirs-1]=new i4_dir_string(fdat.name);
    }
    else
    {
      i4_file_status_struct *s=stats.add();
      s->last_accessed=fdat.time_access;
      s->last_modified=fdat.time_write;
      s->created=fdat.time_create;

      tfiles++;
      files=(i4_str **)i4_realloc(files,sizeof(i4_str *)*tfiles,"dir list");
      files[tfiles-1]=new i4_dir_string(fdat.name);
    }

    done=_findnext(handle, &fdat);
  } while (done!=-1);


  if (file_status)
  {    
    if (!tfiles)
      *file_status=0;
    else
    {
      i4_file_status_struct *sa;
      sa=(i4_file_status_struct *)i4_malloc(sizeof (i4_file_status_struct) * tfiles, "stat array");      
      for (int j=0; j<tfiles; j++)
        sa[j]=stats[j];

      *file_status=sa;
    }
  }

  return i4_T;
}
