/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "/usr/include/sys/stat.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>


#include "error/error.hh"
#include "file/file.hh"
#include "file/buf_file.hh"
#include "file/file_man.hh"
#include "status/status.hh"
#include "threads/threads.hh"
#include "string/string.hh"
#include "memory/malloc.hh"
#include "file/async.hh"

i4_bool i4_allow_async_reads=i4_T;
//#define EMULATE_CD_SPEEDS 1
//#define EMULATE_SLOW_HARDDRIVE 1


extern "C" {
// libpthreads doesn't have this defined (it's not thread-safe)
extern char *realpath __P ((__const char *__name, char *__resolved));
           };

i4_critical_section_class realpath_lock;

enum { S_IRWUGO=(S_IRUSR|S_IRGRP|S_IROTH| S_IWUSR|S_IWGRP|S_IWOTH) };
enum { S_ALL   =(S_IRUSR|S_IRGRP|S_IROTH| S_IWUSR|S_IWGRP|S_IWOTH | S_IXUSR|S_IXGRP|S_IXOTH) };

class i4_unix_async_reader : public i4_async_reader
{
public:
  i4_unix_async_reader(char *name) : i4_async_reader(name) {}
  virtual w32 read(sw32 fd, void *buffer, w32 count) 
  { 
    return ::read(fd,buffer,count);
  }
} i4_unix_async_instance("hd_async_reader");




char *i4_linux_filename_filter(const i4_const_str &name, char *obuf, int olen)
{
  char buf[256], *s, *d;
  i4_os_string(name, buf, 256);
  int changed=0;

  if (olen) olen--;     // save space for null

  for (s=buf, d=obuf; olen && *s; )
  {
    if (*s=='\\')
    {
      *d='/';
      changed=1;
    }
    else if (s[0]=='c' && s[1]==':' && s[2]=='/')
    {
      *d='/';
      s+=2;
      changed=1;
    }
    else if (s[0]=='x' && s[1]==':')
    {
      d[0]='/';
      d[1]='u';
      d++;
      olen--;
      s++;
      changed=1;
    }
    else *d=*s;

    s++;
    d++;
    olen--;
  }
  *d=0;
  
  return obuf;
}



////////////////////////////////////////////////////////////////////////
//
//  Normal UNIX File Class
//

class i4_normal_file_class : public i4_file_class
{
protected:
  w32 fd;
public:

  i4_normal_file_class(w32 fd) : fd(fd) {}


  virtual i4_bool async_read (void *buffer, w32 size, async_callback call, void *context)
  {
    if (i4_threads_supported())
      return i4_unix_async_instance.start_read(fd, buffer, size, call, context);
    else 
      (*call)(read(buffer, size), context);
    return i4_T;
  }

  virtual w32 read (void *buffer, w32 size) 
  {
    int ret=::read(fd,buffer,size);
    if (ret<0) ret=0;
    return ret;
  }
  
  virtual w32 write(const void *buffer, w32 size) 
  { 
    w32 ret=::write(fd,buffer,size); 
    return ret;
  }
  
  virtual w32 seek (w32 offset)
  { 
    w32 ret=lseek(fd,offset,SEEK_SET); 
    return ret;
  }
  
  virtual w32 size ()                       
  { 
    w32 cur=lseek(fd,0,SEEK_CUR);
    w32 len=lseek(fd,0,SEEK_END);
    lseek(fd,cur,SEEK_SET);
    return len;
  }
  
  virtual w32 tell ()                       
  { 
    w32 ret=lseek(fd,0,SEEK_CUR); 
    return ret;
  }
  
  ~i4_normal_file_class()
  { 
    close(fd); 
  }
};



class i4_linux_file_manager_class : public i4_file_manager_class
{

public:
  virtual i4_file_class *open(const i4_const_str &name, w32 flags=I4_READ)
  {
    char buf[256];
    i4_file_class *fp=0;

    sw32 f=0;
    i4_bool no_buffer=i4_F;

    flags &= ~(I4_SUPPORT_ASYNC);
    
    if (flags & I4_NO_BUFFER)
    {
      flags=(flags & (~I4_NO_BUFFER));
      no_buffer=i4_T;
    }


    switch (flags)
    {
      case I4_READ | I4_APPEND :
      case I4_READ | I4_WRITE | I4_APPEND :
        f |= O_RDWR | O_CREAT | O_APPEND;
        break;
        
      case I4_READ: 
        f|=O_RDONLY;
        break;

      case I4_WRITE: 
        f |= O_WRONLY | O_CREAT;
        ::unlink(i4_linux_filename_filter(name, buf, sizeof(buf)));
        break;

      case I4_WRITE|I4_READ: 
        f |= O_RDWR | O_CREAT;
        ::unlink(i4_linux_filename_filter(name, buf, sizeof(buf)));
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


    if (flags & I4_WRITE)
      fd=::open(i4_linux_filename_filter(name, buf, sizeof(buf)),f,S_IRWUGO);
    else
      fd=::open(i4_linux_filename_filter(name, buf, sizeof(buf)),f);

    if (fd<0)
      return 0;

    if (!no_buffer)
      fp=new i4_buffered_file_class(new i4_normal_file_class(fd));
    else
      fp=new i4_normal_file_class(fd);

    return fp;
    
  }

  virtual i4_bool unlink(const i4_const_str &name)
  {
    char buf[256];
    return ::unlink(i4_linux_filename_filter(name, buf, sizeof(buf)))==0;
  }

  virtual i4_bool mkdir(const i4_const_str &name)
  {
    char buf[256];
    return ::mkdir(i4_linux_filename_filter(name,buf,sizeof(buf)),S_ALL)==0;
  }

  i4_bool get_status(const i4_const_str &filename, i4_file_status_struct &return_stat)
  {
    char buf[256];
    i4_bool error=i4_F;
    struct stat times;

    return_stat.flags=0;

    if (stat(i4_linux_filename_filter(filename,buf,sizeof(buf)),&times)==0)     
    {
      return_stat.last_modified=times.st_mtime;
      return_stat.last_accessed=times.st_atime;      
      return_stat.created=times.st_ctime;

      if (times.st_mode &  S_IFDIR)
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
    char buf[256];
    struct dirent *de;

    DIR *d=opendir(i4_linux_filename_filter(path,buf,sizeof(buf)));
    if (!d) return i4_F;

    i4_str **tlist=NULL;
    sw32 t=0;
    char curdir[200];
    getcwd(curdir,200);
    if (chdir(i4_linux_filename_filter(path,buf,sizeof(buf)))!=0)
      return i4_F;

    do
    {
      de=readdir(d);

      if (status) 
        status->update(0.5);

      if (de && de->d_name[0])
      {
        t++;
        tlist=(i4_str **)i4_realloc(tlist,sizeof(i4_str *)*t,"tmp file list");
        tlist[t-1]=new i4_str(de->d_name);
      }
    } while (de);
    closedir(d);

    i4_file_status_struct *sa=0;
    
    if (t)
      sa=(i4_file_status_struct *)i4_malloc(sizeof (i4_file_status_struct) * t, "stat array");
   
    for (int i=0;i<t;i++)
    {
      if (status) 
        status->update(i/(float)t);


      struct stat s;
      stat(i4_linux_filename_filter(*tlist[i],buf,sizeof(buf)), &s);
                                                                                   
      
      if (S_ISDIR(s.st_mode))
      {
        dir_struct.tdirs++;
        dir_struct.dirs=(i4_str **)i4_realloc(dir_struct.dirs,
                                              sizeof(i4_str *)*dir_struct.tdirs,"dir list");
        dir_struct.dirs[dir_struct.tdirs-1]=tlist[i];
      } else
      {
        int on=dir_struct.tfiles++;
        
        dir_struct.files=(i4_str **)i4_realloc(dir_struct.files,
                                               sizeof(i4_str *)*dir_struct.tfiles,"dir list");
        dir_struct.files[on]=tlist[i];

        sa[on].last_accessed=s.st_atime;
        sa[on].last_modified=s.st_mtime;
        sa[on].created=s.st_ctime;
      }
    }
    
    if (t)
      i4_free(tlist);

    if (get_status)
      dir_struct.file_status=sa;
    else if (sa)
      i4_free(sa);

    
    chdir(curdir);
    return i4_T;
  }

  virtual i4_str *full_path(const i4_const_str &relative_name)
  {
    char buf[256], buf2[256];
    realpath_lock.lock();
    ::realpath(i4_linux_filename_filter(relative_name, buf, 256), buf2);
    realpath_lock.unlock();
    return new i4_str(i4_const_str(buf2));
  }


  i4_linux_file_manager_class()
  {
    i4_add_file_manager(this, i4_F);
  }

  ~i4_linux_file_manager_class()
  {
    i4_remove_file_manger(this);
  }

} i4_linux_file_manager;



i4_bool i4_rmdir(const i4_const_str &path)
{
  char p[256];
  i4_os_string(path, p, 256);
  if (rmdir(p)==0)
    return i4_T;
  else return i4_F;
}

i4_bool i4_chdir(const i4_const_str &path)
{
  char p[256];
  i4_os_string(path, p, 256);
  if (chdir(p)==0)
    return i4_T;
  else return i4_F;
}

