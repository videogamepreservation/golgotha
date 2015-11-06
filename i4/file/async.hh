/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef I4_ASYNC_READ_HH
#define I4_ASYNC_READ_HH

// This class should probably only be used by files in "i4/files/*"

// This class is a portable implemented of async file reading
// A thread gets created during init() which runs as long as
// there is stuff to read and then blocks for a signal from the main
// program so if nothing needs reading, it runs efficiently.  Request
// to be read are qued up (max of 16).  The request are processed
// serially.  If you want to read from multiple devices in parallel,
// you should create two i4_async_reads, one for each device.

#include "memory/que.hh"
#include "threads/threads.hh"
#include "init/init.hh"
#include "file/file.hh"

class i4_async_reader : public i4_init_class
{
  volatile i4_bool stop;
  i4_signal_object sig;

  i4_bool emulation;

  struct read_request
  {
    sw32 fd;
    w32 size;
    void *buffer;
    i4_file_class::async_callback callback;
    void *context;
    read_request(sw32 fd, void *buffer,
                 w32 size, i4_file_class::async_callback callback, 
                 void *context)
      : fd(fd), buffer(buffer), size(size), callback(callback), context(context) {}
    read_request() { ; }
  };

  enum { MAX_REQUEST=16 };
  enum { STACK_SIZE=8096 };
  i4_critical_section_class que_lock;
  i4_fixed_que<read_request, MAX_REQUEST> request_que;
  void emulate_speeds(read_request &r);

protected:

  virtual w32 read(sw32 fd, void *buffer, w32 count) = 0;

public:
  // name is just some unique name.  Windows requires this for Semaphores
  i4_async_reader(char *name);

  void init();   // creates thread (called by i4_init()
  void uninit(); // waits for thread to die (called by i4_uninit()

  // ques up a request
  i4_bool start_read(int fd, void *buffer, w32 size, 
                     i4_file_class::async_callback call,
                     void *context);

  void PRIVATE_thread();  // don't call this!
};


#endif
