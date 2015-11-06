/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "file/file.hh"
#include "memory/malloc.hh"
#include "error/error.hh"
#include "time/profile.hh"
#include "error/error.hh"
#include "file/buf_file.hh"
#include "file/async.hh"
#include "file/file_man.hh"
#include "memory/array.hh"
#include <windows.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <direct.h>
#include "string/string.hh"

i4_profile_class pf_win32_seek("Win32File::Seek");

i4_profile_class pf_win32_read("Win32File::Read");
i4_profile_class pf_win32_write("Win32File::Write");

class i4_win32_async_reader : public i4_async_reader
{
public:
  i4_win32_async_reader(char *name) : i4_async_reader(name) {}
  virtual w32 read(sw32 fd, void *buffer, w32 count) 
  { 
    w32 res = _read(fd,buffer,count);
    return res;
  }
} i4_win32_async_instance("hd_async_reader_2");


////////////////////////////////////////////////////////////////////////
//
//  Normal Win32 File Class
//

class i4_win32_file_class : public i4_file_class
{
protected:
  w32 fd;
public:

  i4_win32_file_class(w32 fd) : fd(fd) {}

  virtual w32 read (void *buffer, w32 size)
  {
    pf_win32_read.start();
    w32 res = _read(fd,buffer,size);
    pf_win32_read.stop();
    return res;
  }

  virtual w32 write(const void *buffer, w32 size)
  {
    pf_win32_write.start();
    w32 res = _write(fd,buffer,size);
    pf_win32_write.stop();
    return res;
  }

  virtual w32 seek (w32 offset)
  {  
    pf_win32_seek.start();
    w32 res = lseek(fd, offset, SEEK_SET);
    pf_win32_seek.stop();
    return res;
  }

  virtual w32 size ()                       
  { 
    w32 len = _filelength(fd);

    /*
    w32 cur = lseek(fd,0,SEEK_CUR);
      ]  len = lseek(fd,0,SEEK_END);
    lseek(fd,cur,SEEK_SET);
    */
    return len;
  }

  virtual w32 tell ()
  {
    return _tell(fd);
  }

  ~i4_win32_file_class()
  { 
    _close(fd); 
  }


  // returns i4_F if an immediate error occured
  virtual i4_bool async_read (void *buffer, w32 size, 
                              async_callback call,
                              void *context=0)
  {
    if (i4_threads_supported())
      return i4_win32_async_instance.start_read(fd, buffer, size, call, context);
    else
      call(read(buffer,size),context);
    return i4_T;
  }

};


////////////////////////////////////////////////////////////////////////
//
//  File Manager Methods
//










// see file/file.hh for a description of what each of these functions do
class i4_win32_file_manager_class : public i4_file_manager_class
{
public:   
  virtual i4_file_class *open(const i4_const_str &name, w32 flags)
  {
    sw32 f=0;
    i4_bool no_buffer=i4_F;
    
    flags &= ~I4_SUPPORT_ASYNC;     // don't have to do anything special for these
    
    if (flags & I4_NO_BUFFER)
    {
      flags=(flags & (~I4_NO_BUFFER));
      no_buffer=i4_T;
    }

    f=O_BINARY;       // open all files in binary mode
    char sbuf[256];

    switch (flags)
    {
      case I4_READ: 
        f|=O_RDONLY;
        break;

      case I4_WRITE: 
        f |= O_WRONLY | O_CREAT;
        _unlink(i4_os_string(name,sbuf,sizeof(sbuf)));
        break;

      case I4_WRITE|I4_READ: 
        f |= O_RDWR | O_CREAT;
        _unlink(i4_os_string(name,sbuf,sizeof(sbuf)));
        break;

      case I4_APPEND:
      case I4_WRITE|I4_APPEND:
        f |= O_WRONLY|O_CREAT|O_APPEND;
        break;

      default: 
        i4_warning("i4_file_class::Bad open flags!");
        return NULL;     
    }

    int fd;


    fd=::_open(i4_os_string(name,sbuf,sizeof(sbuf)),f,_S_IREAD | _S_IWRITE);

    if (fd<0)
    {
      i4_warning("i4_file_class::open failed for %s\n",i4_os_string(name,sbuf,sizeof(sbuf)));
      return NULL;
    }


    i4_file_class *ret_fp;
    if (!no_buffer)
      ret_fp=new i4_buffered_file_class(new i4_win32_file_class(fd));
    else
      ret_fp=new i4_win32_file_class(fd);

    return ret_fp;
  }



  virtual i4_bool unlink(const i4_const_str &name)
  {
    char buf[256];
    return _unlink(i4_os_string(name,buf,sizeof(buf)))==0;
  }


  virtual i4_bool mkdir(const i4_const_str &name)
  {  
    char buf[256];
    return ::_mkdir(i4_os_string(name,buf,sizeof(buf)))==0;
  }

  i4_bool get_status(const i4_const_str &filename, i4_file_status_struct &return_stat)
  {
    i4_bool error=i4_F;
    struct _stat times;
    char buf[256];

    return_stat.flags=0;
    if (_stat(i4_os_string(filename,buf,sizeof(buf)),&times)==0)      
    {
      return_stat.last_modified=times.st_mtime;
      return_stat.last_accessed=times.st_atime;
      return_stat.created=times.st_ctime;

      if (times.st_mode &  _S_IFDIR)
        return_stat.flags=I4_FILE_STATUS_DIRECTORY;     
    }
    else 
      error=i4_T;

    return (i4_bool)(!error);
  }

  virtual i4_bool get_directory(const i4_const_str &path, 
                                i4_directory_struct &dir_struct,
                                i4_bool get_status,
                                i4_status_class *status)

  {
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
        dir_struct.tdirs++;
        dir_struct.dirs=(i4_str **)i4_realloc(dir_struct.dirs,
                                              sizeof(i4_str *)*dir_struct.tdirs,"dir list");
        dir_struct.dirs[dir_struct.tdirs-1]=new i4_str(i4_const_str(fdat.name));
      }
      else
      {
        i4_file_status_struct *s=stats.add();
        s->last_accessed=fdat.time_access;
        s->last_modified=fdat.time_write;
        s->created=fdat.time_create;

        dir_struct.tfiles++;
        dir_struct.files=(i4_str **)i4_realloc(dir_struct.files,
                                               sizeof(i4_str *)*dir_struct.tfiles,"dir list");
        dir_struct.files[dir_struct.tfiles-1]=new i4_str(i4_const_str(fdat.name));
      }

      done=_findnext(handle, &fdat);
    } while (done!=-1);


    if (get_status)
    {    
      if (dir_struct.tfiles)
      {
        i4_file_status_struct *sa;
        sa=(i4_file_status_struct *)i4_malloc(sizeof(i4_file_status_struct)*dir_struct.tfiles,"");
        for (int j=0; j<dir_struct.tfiles; j++)
          sa[j]=stats[j];

        dir_struct.file_status=sa;
      }
    }

    return i4_T;
  }

  virtual i4_str *full_path(const i4_const_str &relative_name)
  {
    char buf[256], buf2[256];
    ::_fullpath(i4_os_string(relative_name, buf, 256), buf2, 256);
    return new i4_str(i4_const_str(buf2));
  }

  i4_win32_file_manager_class()
  {
    i4_add_file_manager(this, i4_F);    
  }

  ~i4_win32_file_manager_class()
  {
    i4_remove_file_manger(this);
  }

} i4_win32_file_man;


i4_bool i4_rmdir(const i4_const_str &path)
{
  char p[256];
  i4_os_string(path, p, 256);
  if (_rmdir(p)==0)
    return i4_T;
  else return i4_F;
}

i4_bool i4_chdir(const i4_const_str &path)
{
  char p[256];
  i4_os_string(path, p, 256);
  if (_chdir(p)==0)
    return i4_T;
  else return i4_F;
}

