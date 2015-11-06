/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include <Files.h>
#include <Errors.h>
#include <Types.h>
#include <LowMem.h>

#include "file/file.hh"
#include "threads/threads.hh"
#include "file/buf_file.hh"
#include "file/get_dir.hh"

#include "video/display.hh"

extern i4_critical_section_class i4_file_lock;

// Mac Specific Global Variables
short mac_volume;
long mac_directory;
OSType mac_creator;
OSType mac_filetype;

// Asynchronous File Support Variables
static i4_file_class::async_callback current_read_callback;
static void *current_read_context;
static HParamBlockRec read_pb;
static IOCompletionUPP read_complete_upp;

static i4_file_class::async_callback current_write_callback;
static void *current_write_context;
static HParamBlockRec write_pb;
static IOCompletionUPP write_complete_upp;


////////////////////////////////////////////////////////////////////////
//
//  Mac internal routines
//

long GetCurDirID()   { return LMGetCurDirStore(); }
// gets current id of the current directory

short GetCurVRefNum() { return LMGetSFSaveDisk(); }
// get current reference number of the current mac voume


////////////////////////////////////////////////////////////////////////
//
//  Normal Mac File Class
//

class i4_mac_file_class : public i4_file_class
//{{{
{
protected:
  short ioRefNum;
public:
  i4_mac_file_class(short ref) : ioRefNum(ref) {}

  virtual w32 read (void *buffer, w32 count) 
  //  perform normal read through Synchronous HFS routines
  //{{{
  {
    OSErr err;

    i4_file_lock.lock();

    w32 ret=count;
    err = ::FSRead(ioRefNum,(long*)&ret,buffer); 
    if (err && err != eofErr)
      ret = 0;

    i4_file_lock.unlock();
    return ret;
  }
  //}}}

  virtual w32 write(void *buffer, w32 count) 
  //  perform normal write through Synchronous HFS routines
  //{{{
  { 
    OSErr err;

    i4_file_lock.lock();
    w32 ret=count;
    err = ::FSWrite(ioRefNum,(long*)&ret,buffer); 
    if (err && err != eofErr)
      ret = 0;

    i4_file_lock.unlock();
    return ret;
  }
  //}}}

  virtual w32 seek (w32 offset)
  //  perform normal seek through Synchronous HFS routines
  //{{{
  { 
    OSErr err;

    i4_file_lock.lock();
    w32 ret=offset;
    err = ::SetFPos(ioRefNum,fsFromStart,(long)offset); 
    if (err && err != eofErr)
      ret = 0;

    i4_file_lock.unlock();
    return ret;
  }
  //}}}

  virtual w32 size ()                       
  //  get logical file size through Synchronous HFS routines
  //{{{
  { 
    i4_file_lock.lock();
    w32 len = 0;
    ::GetEOF(ioRefNum, (long*)&len);
    i4_file_lock.unlock();
    return len;
  }
  //}}}

  virtual w32 tell ()                       
  //  get logical file position through Synchronous HFS routines
  //{{{
  { 
    i4_file_lock.lock();
    w32 ret=0;
    ::GetFPos(ioRefNum,(long*)&ret); 
    i4_file_lock.unlock();
    return ret;
  }
  //}}}

  virtual i4_bool async_read (void *buffer, w32 count, async_callback call, void *context=0);
  virtual i4_bool async_write(void *buffer, w32 count, async_callback call, void *context=0);

  ~i4_mac_file_class()
  //{{{
  { 
    i4_file_lock.lock();
    ::FSClose(ioRefNum); 
    //::FlushVol(0,mac_volume);
    i4_file_lock.unlock();
  }
  //}}}
};
//}}}


static void mac_read_complete(HParmBlkPtr blk)
//  callback routine for MAC PB device interface
//{{{
{
  (*current_read_callback)(blk->ioParam.ioActCount,current_read_context);

  current_read_callback = 0;
  current_read_context = 0;
}
//}}}


static void mac_write_complete(HParmBlkPtr blk)
//  callback routine for MAC PB device interface
//{{{
{
  (*current_write_callback)(blk->ioParam.ioActCount,current_write_context);

  current_write_callback = 0;
  current_write_context = 0;
}
//}}}


i4_bool i4_mac_file_class::async_read (void *buffer, w32 count, 
                                       i4_file_class::async_callback call,
                                       void *context)
//{{{
{
#if 0
  // use default read for testing purposes

  i4_bool ret = read(buffer,count);

  (*call)(ret,context);

  return i4_T;
#else
  OSErr err;

  read_pb.ioParam.ioCompletion = read_complete_upp;
  read_pb.ioParam.ioRefNum = ioRefNum;
  read_pb.ioParam.ioBuffer = (Ptr)buffer;
  read_pb.ioParam.ioReqCount = count;
  read_pb.ioParam.ioPosMode = fsAtMark;

  current_read_callback = call;
  current_read_context = context;

  // call asynchronous read through PB device interface
  err = PBReadAsync( (ParmBlkPtr)&read_pb );

  if ( err != noErr)
    return i4_F;

  return i4_T;
#endif
}
//}}}


i4_bool i4_mac_file_class::async_write (void *buffer, w32 count, 
                                        i4_file_class::async_callback call,
                                        void *context)
//{{{
{
#if 1
  // use default write for testing purposes

  i4_bool ret = write(buffer,count);

  (*call)(ret,context);

  return i4_T;
#else
  OSErr err;

  write_pb.ioParam.ioCompletion = write_complete_upp;
  write_pb.ioParam.ioRefNum = ioRefNum;
  write_pb.ioParam.ioBuffer = (Ptr)buffer;
  write_pb.ioParam.ioReqCount = count;
  write_pb.ioParam.ioPosMode = fsAtMark;

  current_write_callback = call;
  current_write_context = context;

  // call asynchronous write through PB device interface
  err = PBWriteAsync( (ParmBlkPtr)&write_pb );

  if ( err != noErr)
    return i4_F;

  return i4_T;
#endif
}
//}}}


////////////////////////////////////////////////////////////////////////
//
//  OS Name functions
//

Str255 MacName;

StringPtr mac_os_string(const i4_const_str &name)
// convert C string i4 name into Pascal string to use in Macintosh interfaces
//{{{
{
  i4_const_str::iterator p=name.begin();
  int s=1, s2;
  char ch;

  if (p!=name.end())
  {
    if (p.get().value() == '/')
      ++p;
    else
      MacName[s++] = ':';
  }
  while (p!=name.end())
  {
    if (p.get().value()=='/')
    {
      MacName[s++]=':';
      ++p;
    }

    s2 = s;
    ch = 0;
    while (p!=name.end())
    {
      ch = p.get().value();
      if (ch == '/')
        break;
      MacName[s++]= ch;
      ++p;
    }
    MacName[s] = 0;

    if (MacName[s2] == '.')
      if (MacName[s2+1] == '.')
      {
        MacName[s2] = ':';
        s = s2+1;
        if (ch=='/')
          ++p;
      }
      else if (MacName[s2+1] == 0)
        if (ch=='/')
          ++p;
  }
  MacName[s]=0;
  MacName[0] = s-1;

  return MacName;
}
//}}}


char *i4_os_string(const i4_const_str &name)
// convert C string i4 name into C string Mac style filename, mainly for debugging
//{{{
{
  return ((char*)mac_os_string(name))+1;
}
//}}}


////////////////////////////////////////////////////////////////////////
//
//  File Manager Methods
//


i4_file_manager_class::i4_file_manager_class()
//{{{
{
  mac_volume = 0; // GetCurVRefNum();
  mac_directory = GetCurDirID();
  read_complete_upp = NewIOCompletionProc(mac_read_complete);
  write_complete_upp = NewIOCompletionProc(mac_write_complete);
}
//}}}


i4_file_class *i4_file_manager_class::default_open(const i4_const_str &name, w32 flags)
//{{{
{
  OSErr err;

  i4_file_lock.lock();

  i4_bool no_buffer=i4_F;

  if (flags & NO_BUFFER)
  {
    flags=(file_flags)(flags & (~NO_BUFFER));
    no_buffer=i4_T;
  }

  SignedByte mac_flags;
  switch (flags)
  {
    case READ: 
      mac_flags = fsRdPerm;
      break;

    case WRITE: 
    case APPEND:
    case WRITE|APPEND:
      mac_flags = fsWrPerm;
      break;

    case WRITE|READ: 
      mac_flags = fsRdWrPerm;
      break;

    default: 
      i4_warning("i4_file_class::Bad open flags!");
      i4_file_lock.unlock();
      return NULL;     
  }

  if ((flags & WRITE) && !(flags & APPEND))
  {
    // If rewriting file, delete & create file

    ::HDelete( mac_volume, mac_directory, mac_os_string(name) );
    if (::HCreate(mac_volume, mac_directory, mac_os_string(name), 
                  mac_creator, mac_filetype ) != noErr)
    {
      i4_warning("i4_file_class::create failed for %s\n",i4_os_string(name));
      i4_file_lock.unlock();
      return NULL;
    }
  }

  short FRef;

  // Attempt to open file, synchronously, for now
  err = ::HOpen(mac_volume, mac_directory, mac_os_string(name), mac_flags, &FRef);
  if ( err != noErr)
  {
    i4_warning("i4_file_class::open failed for %s\n",i4_os_string(name));
    i4_file_lock.unlock();
    return NULL;
  }

  i4_file_class *ret_fp;

  if (!no_buffer)
    ret_fp=new i4_buffered_file_class(new i4_mac_file_class(FRef), 0x1000, 0);
  else
    ret_fp=new i4_mac_file_class(FRef);

  // if append, seek to end
  if (flags & APPEND)
    ret_fp->seek(ret_fp->size());

  i4_file_lock.unlock();
  return ret_fp;
}
//}}}


i4_bool i4_file_manager_class::default_unlink(const i4_const_str &name)
//{{{
{
  return ::HDelete(mac_volume, mac_directory, mac_os_string(name))==noErr;
}
//}}}


i4_bool i4_file_manager_class::default_get_status(const i4_const_str &name, 
                                                  i4_file_status_struct &return_stat)
//{{{
{
  HParamBlockRec stat;
  OSErr err;

  // search from current directory and volume
  stat.fileParam.ioVRefNum = mac_volume;
  stat.fileParam.ioFVersNum = 0;
  stat.fileParam.ioFDirIndex = 0;
  stat.fileParam.ioNamePtr = mac_os_string(name);
  stat.fileParam.ioDirID = mac_directory;

  err = PBHGetFInfoSync(&stat);

  if (err)
    return i4_F;

  return_stat.last_modified = stat.fileParam.ioFlMdDat;

  return i4_T;
}
//}}}


i4_bool i4_file_manager_class::default_mkdir(const i4_const_str &name)
//{{{
{
  long id;

  return ::DirCreate(mac_volume, mac_directory, mac_os_string(name), &id)==noErr;
}
//}}}


i4_bool i4_file_manager_class::default_get_directory(const i4_const_str &path, 
                                             i4_str **&files, w32 &tfiles, 
                                             i4_str **&dirs, w32 &tdirs)
// returns i4_F if path is bad (tfiles and tdirs will be 0 as well)
//{{{
{
  return ::i4_get_directory(path, files,tfiles, dirs,tdirs);
}
//}}}


//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}
