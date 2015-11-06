/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

//{{{ File & File Manager Classes
//
//$Id: file.cc,v 1.48 1998/07/15 21:16:39 jc Exp $

#include "file/file.hh"
#include "memory/malloc.hh"
#include "error/error.hh"
#include "time/profile.hh"
#include "error/error.hh"
#include "file/file_man.hh"
#include "string/string.hh"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>


static i4_profile_class pf_tell("File tell");
static i4_profile_class pf_open("File Open");
static i4_profile_class pf_sopen("System file Open");
static i4_profile_class pf_close("File Close");
static i4_profile_class pf_read("File Read");
static i4_profile_class pf_con_buf("::buffered file");


i4_bool i4_file_class::async_write(const void *buffer, w32 size, 
                                   async_callback call,
                                   void *context)
{
  (*call)(write(buffer, size), context);

  return i4_T;
}


i4_bool i4_file_class::async_read (void *buffer, w32 size, 
                                   async_callback call,
                                   void *context)
{
  (*call)(read(buffer, size), context);
  return i4_T;
}


class file_string : public i4_str
{
public:
  file_string(w16 size) : i4_str(size) {}

  char *buffer() { return (char *)ptr; }
  void set_len(w16 _len) { len = _len; }
};


i4_str* i4_file_class::read_str(w32 len)
{
  file_string *str = new file_string((w16)len);   
  len = read(str->buffer(),len);
  str->set_len((w16)len);

  return str;
}

i4_str* i4_file_class::read_counted_str()
{
  w16 len = read_16();
  return read_str(len);
}


class file_write_string : public i4_const_str
{
public:
  char *buffer() { return (char *)ptr; }
};


w32 i4_file_class::write_str(const i4_const_str &str)
{
  file_write_string *s = (file_write_string *)&str;

  return write(s->buffer(), s->length());
}

w32 i4_file_class::write_counted_str(const i4_const_str &str)
{
  w32 count;

  count = write_16((w16)str.length());
  if (str.length())      
    return (count + write_str(str));
  else
    return count;
}



static inline int fmt_char(char c)
{
  if ((c>='a' && c<='z') || (c>='A' && c<='Z'))
    return 1;
  return 0;
}

// same as fprintf, but with the addition %S is a i4_const_str *
int i4_file_class::printf(char *fmt, ...)
{
  w32 start=tell();

  va_list ap;
  va_start(ap, fmt);

  while (*fmt)
  {
    if (*fmt=='%')
    {
      char *fmt_end=fmt;
      while (!fmt_char(*fmt_end) && *fmt_end) fmt_end++;
      char f[10], out[500]; 
      memcpy(f, fmt, fmt_end-fmt+1);
      f[fmt_end-fmt+1]=0;
      out[0]=0;

      switch (*fmt_end)
      {
        case 's' : 
        {
          char *str=va_arg(ap,char *);
          write(str,strlen(str));
        } break;          
        case 'S' : 
        {
          i4_const_str *s=va_arg(ap,i4_const_str *); 
          write_str(*s);
        } break;
        case 'd' :
        case 'i' :
        case 'x' :
        case 'X' :
        case 'c' :
        case 'o' :
        {
          ::sprintf(out,f,va_arg(ap,int));
          write(out,strlen(out));
        } break;

        case 'f' :
        case 'g' :
          ::sprintf(out,f,va_arg(ap,double));
          write(out,strlen(out));
          break;

        default :
          ::sprintf(out,f,va_arg(ap,void *));
          write(out,strlen(out));
          break;
      }
      fmt=fmt_end;
      if (*fmt)
        fmt++;
    }
    else
    {
      write_8(*fmt);
      fmt++;
    }
  }
  va_end(ap);

  return tell()-start;
}




int i4_file_class::write_format(char *format, ...)
{
  char *f=format;
  va_list ap;
  va_start(ap, format);

  int start=tell();
  while (*f)
  {
    switch (*f)
    {
      case '1' : write_8(*(va_arg(ap,     w8 *))); break;
      case '2' : write_16(*(va_arg(ap,    w16 *))); break;
      case '4' : write_32(*(va_arg(ap,     w32 *))); break;
      case 'f' : write_float(*(va_arg(ap,  float *))); break;
      case 'S' : write_counted_str(  *(va_arg(ap, i4_const_str *))); break;
    }
    f++;
  }
  va_end(ap);

  return tell()-start;
}


// format is same as write_format, returns number of fields written
int i4_file_class::read_format(char *format, ...)
{
  char *f=format;
  va_list ap;
  va_start(ap, format);

  int start=tell();
  while (*f)
  {
    switch (*f)
    {
      case '1' : *(va_arg(ap,   w8 *))=read_8(); break;
      case '2' : *(va_arg(ap,  w16 *))=read_16(); break;
      case '4' : *(va_arg(ap,  w32 *))=read_32(); break;
      case 'f' : *(va_arg(ap, float*))=read_float(); break;
      case 'S' : *(va_arg(ap, i4_str **))=read_counted_str(); break;
    }
    f++;
  }
  va_end(ap);

  return tell()-start;
}


// returns NULL if unable to open file
i4_file_class *i4_open(const i4_const_str &name, w32 flags)
{
  for (i4_file_manager_class *m=i4_file_manager_class::first; m; m=m->next)
  {    
    i4_file_class *fp=m->open(name,flags);
    if (fp) 
      return fp;
  }

  return 0;
}

// return i4_F on failure
i4_bool i4_unlink(const i4_const_str &name)
{
  for (i4_file_manager_class *m=i4_file_manager_class::first; m; m=m->next)
    if (m->unlink(name))
      return i4_T;
  return i4_F;
}


// returns i4_F if file does not exsist
i4_bool i4_get_status(const i4_const_str &filename, 
                      i4_file_status_struct &return_stat)
{
  for (i4_file_manager_class *m=i4_file_manager_class::first; m; m=m->next)
    if (m->get_status(filename, return_stat))
      return i4_T;

  return i4_F;
}

// return i4_F on failure
i4_bool i4_mkdir(const i4_const_str &name)
{
  for (i4_file_manager_class *m=i4_file_manager_class::first; m; m=m->next)
    if (m->mkdir(name))
      return i4_T;

  return i4_F;
}

// returns i4_F if path is bad (tfiles and tdirs will be 0 as well)
// you are responsible for deleting both the array of strings and each string in the array
// file_status is a pointer to an array of file_status's that will be created, you
// must free these as well.  file_status may be 0 (default), in which case no array is created

i4_bool i4_get_directory(const i4_const_str &path, 
                         i4_directory_struct &dir_struct,
                         i4_bool get_status,
                         i4_status_class *status)
{
  for (i4_file_manager_class *m=i4_file_manager_class::first; m; m=m->next)
    if (m->get_directory(path, dir_struct, get_status, status)) 
      return i4_T;

  return i4_F;
}

// returns i4_F if path cannot be split
i4_bool i4_split_path(const i4_const_str &name, i4_filename_struct &fname_struct)
{
  for (i4_file_manager_class *m=i4_file_manager_class::first; m; m=m->next)
    if (m->split_path(name, fname_struct))
      return i4_T;

  return i4_F;
}

// return 0 if full path cannot be determined
i4_str *i4_full_path(const i4_const_str &relative_name)
{
  for (i4_file_manager_class *m=i4_file_manager_class::first; m; m=m->next)
  {
    i4_str *s=m->full_path(relative_name);
    if (s) 
        return s;
  }

  return 0;
}


i4_file_manager_class *i4_file_manager_class::first=0;

void i4_add_file_manager(i4_file_manager_class *fman, i4_bool add_front)
{
  if (add_front)
  {
    fman->next=i4_file_manager_class::first;
    i4_file_manager_class::first=fman;
  }
  else
  {
    i4_file_manager_class *last=0, *p=i4_file_manager_class::first;
    while (p)
    {
      last=p;
      p=p->next;
    }

    if (!last) 
      i4_add_file_manager(fman, i4_T);
    else 
    {
      last->next=fman;
      fman->next=0;
    }
  }
}

void i4_remove_file_manger(i4_file_manager_class *fman)
{
  i4_file_manager_class *last=0, *p=i4_file_manager_class::first;
  while (p && p!=fman)
  {
    last=p;
    p=p->next;
  }

  if (p!=fman)
    i4_error("unable to find file manager");
  else if (last)
    last->next=fman->next;
  else
    i4_file_manager_class::first=fman->next;   
}




i4_bool i4_file_manager_class::split_path(const i4_const_str &name, i4_filename_struct &fn)
{
  char buf[512];
  i4_os_string(name, buf, 512);


  char *p=buf, *last_slash=0, *last_dot=0, *q;

  while (*p)
  {
    if (*p=='/' || *p=='\\')
      last_slash=p;
    else if (*p=='.')
      last_dot=p;
    p++;
  }

 
  if (last_dot)
  {
    q=fn.extension;    
    for (p=last_dot+1; *p; )
      *(q++)=*(p++);
    *q=0;
  }
  else last_dot=p;
      
  
  if (last_slash)
  {
    q=fn.path;
    for (p=buf; p!=last_slash; )
      *(q++)=*(p++);
    *q=0;
    last_slash++;
  }
  else last_slash=buf;


  q=fn.filename;
  for (p=last_slash; p!=last_dot;)
    *(q++)=*(p++);
  *q=0;


  return i4_T;
}



i4_directory_struct::~i4_directory_struct()
{
  int i;

  for (i=0; i<tfiles; i++)
    delete files[i];

  if (files) 
    i4_free(files);

  for (i=0; i<tdirs; i++)
    delete dirs[i];

  if (dirs)
    i4_free(dirs);


  if (file_status)
    i4_free(file_status);

}



static char convert_slash(char c)
{
  if (c=='\\') return '/';
  else return c;
}

i4_str *i4_relative_path(const i4_const_str &path)
{
  i4_str *full_path=i4_full_path(path);
  i4_str *full_current=i4_full_path(i4_const_str("."));
  
  char full[512], current[512], ret[512];

  i4_os_string(*full_path, full, 512);
  i4_os_string(*full_current, current, 512);

  delete full_path;
  delete full_current;


  // files are on different drives  
  if (full[1]==':' && current[1]==':' && full[0]!=current[0])
    return new i4_str(full);

  // one files is on a network drive and one is local
  if ((full[1]==':' && current[1]!=':') ||
      (full[1]!=':' && current[1]==':'))
    return new i4_str(full);


  int start=0;
  while (convert_slash(full[start])==convert_slash(current[start]))
    start++;


  char *c=current+start;
  if (*c==0)
    return new i4_str(full+start+1);

  strcpy(ret,"../");
  while (*c)
  {
    if (convert_slash(*c)=='/')
      strcpy(ret+strlen(ret),"../");

    c++;
  }

  strcpy(ret+strlen(ret), full+start);
  return new i4_str(ret);
}
