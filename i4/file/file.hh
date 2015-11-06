/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef __I4FILE_HPP
#define __I4FILE_HPP

#include "arch.hh"
#include "isllist.hh"


enum { I4_FILE_STATUS_DIRECTORY=1};
struct i4_file_status_struct
{
  w32 last_modified;
  w32 last_accessed;
  w32 created;
  w32 flags;
} ;

// Summary
//   A file (i4_file_class) is a purely virtual file 
//   which should allow much flexability.
//   A i4_file_class is opened through the i4_file_manager_class::open().

//   A file manager defaults going through the operating system's open(), but
//   like the unix model other file managers can be mounted on top of directories.
//   When a file is accessed in this directory, the open call will be transfered
//   to this file manager.

//   This allows for redirection, tar-like-files, memory-files, and other goodies.

//   One particular instance this is useful, might be running game off of a cdrom.
//   the data directory can be linked to the cdrom and the savegame data can
//   be linked to somewhere on the hard-drive.

//   i.e.

//   i4_file_man.mount_dir("/cdrom",new file_redirector("d:\\"));
//   i4_file_class *fp=i4_file_man.open("/cdrom/art/picture.pcx");


//   TODO : test mounting, now that all string are i4_const_str instead of char *

class i4_const_str;
class i4_str;

class i4_file_class
{
public:
  virtual w32 read (void *buffer, w32 size)          = 0;
  virtual w32 write(const void *buffer, w32 size)    = 0;
  virtual w32 seek (w32 offset)                      = 0;
  virtual w32 size ()                                = 0;
  virtual w32 tell ()                                = 0;

  i4_bool eof() { return tell()==size(); }

  typedef void (*async_callback)(w32 count, void *context);
  
  // returns i4_F if an immediate error occured
  virtual i4_bool async_read (void *buffer, w32 size, 
                              async_callback call,
                              void *context=0);

  virtual i4_bool async_write(const void *buffer, w32 size, 
                              async_callback call,
                              void *context=0);

  // abort current operation
  virtual void abort() {}

  // Nice Read Methods
  w8      read_8   ()
  {
    w8 buf;
    if (read(&buf,1)!=1)
      return 0xff;
    else return buf;
  }

  w16     read_16  ()   // loads and converts to LSB (intel-format)
  {
    w16 buf;
    if (read(&buf,2)!=2)
      return 0xffff;
    else return s_to_lsb(buf);
  }

  w32     read_32  ()   // loads and converts to LSB (intel-format)
  {
    w32 buf;
    if (read(&buf,4)!=4)
      return 0xffffffff;
    else return l_to_lsb(buf);
  }

  float   read_float()   // loads and converts to LSB (intel-format)
  {
    w32 buf;
    if (read(&buf,4)!=4)
      return (float)0xffff;

    buf = l_to_lsb(buf);

    return *((float*)&buf);
  }

  i4_str* read_str(w32 len);
  i4_str* read_counted_str();

  // C++ stream operators
  i4_file_class& operator>>(w32& v)   { v = (w32) read_32(); return *this; }
  i4_file_class& operator>>(sw32& v)  { v = (sw32)read_32(); return *this; }
  i4_file_class& operator>>(w16& v)   { v = (w16) read_16(); return *this; }
  i4_file_class& operator>>(sw16& v)  { v = (sw16)read_16(); return *this; }
  i4_file_class& operator>>(w8& v)    { v = (w8)  read_8();  return *this; }
  i4_file_class& operator>>(sw8& v)   { v = (sw8) read_8();  return *this; }
  i4_file_class& operator>>(float& v) { v = read_float();    return *this; }

  // Nice Write Methods
  w32     write_8   (w8 num)
  {
    return (write(&num,1));
  }

  w32     write_16  (w16 num)   // loads and converts to LSB (intel-format)
  {
    num = s_to_lsb(num);
    return (write(&num,2));
  }

  w32     write_32  (w32 num)   // loads and converts to LSB (intel-format)
  {
    num = l_to_lsb(num);
    return (write(&num,4));
  }

  w32     write_float(float num)   // loads and converts to LSB (intel-format)
  {
    w32 tmp = l_to_lsb( *((w32*)&num) );
    return (write(&tmp,4));
  }

  w32 write_str(const i4_const_str &str);
  w32 write_counted_str(const i4_const_str &str);

  // same as fprintf, but with the addition %S is a i4_const_str *
  int printf(char *format, ...);

  // write_format takes a different set of % symbols than the typical printf to elimante
  // confusion with sizes and be easily usuable with readf_binary
  // writef_binary("124fS", &a_w8_var, &a_w16_var, &a_w32_var, 
  //                        &a_float_var, &a_i4_const_str_pointer);
  int write_format(char *format, ...);

  // format is same as write_format
  int read_format(char *format, ...);

  

  // C++ stream operators
  i4_file_class& operator<<(w32 v)   { write_32(v);      return *this; }
  i4_file_class& operator<<(sw32 v)  { write_32((w32)v); return *this; }
  i4_file_class& operator<<(w16 v)   { write_16(v);      return *this; }
  i4_file_class& operator<<(sw16 v)  { write_16((w16)v); return *this; }
  i4_file_class& operator<<(w8 v)    { write_8(v);       return *this; }
  i4_file_class& operator<<(sw8 v)   { write_8((w8)v);   return *this; }
  i4_file_class& operator<<(float v) { write_float(v);   return *this; }

  virtual ~i4_file_class() {}
};


// open flags 
enum { I4_READ=1,
       I4_WRITE=2,
       I4_APPEND=4,
       I4_NO_BUFFER=8,
       I4_SUPPORT_ASYNC=16  // this flag is needed if you intend to call async_read/write
};

// returns NULL if unable to open file
i4_file_class *i4_open(const i4_const_str &name, w32 flags=I4_READ);  

// return i4_F on failure
i4_bool i4_unlink(const i4_const_str &name);                        

// returns i4_F if file does not exsist
i4_bool i4_get_status(const i4_const_str &filename, 
                      i4_file_status_struct &return_stat);

// return i4_F on failure
i4_bool i4_mkdir(const i4_const_str &path);
i4_bool i4_rmdir(const i4_const_str &path);
i4_bool i4_chdir(const i4_const_str &path);

struct i4_directory_struct
{
  i4_str **files;
  w32 tfiles;

  i4_str **dirs;
  w32 tdirs;

  i4_file_status_struct *file_status;  // array of file stats coresponding to above files

  i4_directory_struct() { tfiles=tdirs=0; files=dirs=0; file_status=0; }
  ~i4_directory_struct();
};

// returns i4_F if path is bad (tfiles and tdirs will be 0 as well)
// you are responsible for deleting both the array of strings and each string in the array
// file_status is a pointer to an array of file_status's that will be created, you
// must free these as well.  file_status may be 0 (default), in which case no array is created
class i4_status_class;
i4_bool i4_get_directory(const i4_const_str &path, 
                         i4_directory_struct &dir_struct, 
                         i4_bool get_status=i4_F,
                         i4_status_class *status=0);



struct i4_filename_struct
{
  char path[256];
  char filename[256];
  char extension[256];

  i4_filename_struct() { path[0]=0; filename[0]=0; extension[0]=0; }
};

// returns i4_F if path cannot be split
i4_bool i4_split_path(const i4_const_str &name, i4_filename_struct &fname_struct);

// return 0 if full path cannot be determined
i4_str *i4_full_path(const i4_const_str &relative_name);  

// returns the shortest relative path
i4_str *i4_relative_path(const i4_const_str &path);



#endif

