/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef I4_STREAM_MUSIC_HH
#define I4_STREAM_MUSIC_HH

#include "arch.hh"
#include "sound/sound.hh"

class i4_voice_class;
class i4_file_class;

// This class should work on all system that support async read's though
// The console versions will probably need a different music system because
// their only disk access is cdrom and that is needed for texture grabbing

class i4_stream_wav_player
{
  i4_voice_class *voice;

  void *locked_buffer_start; // used by lock
  w32   locked_buffer_size;  // used by lock

  void *unused1;
  w32   unused2;

  enum { FIRST_HALF, SECOND_HALF } last_read;
  
  volatile i4_bool wait_read;  // if an async_read has not completed yet
  
  i4_file_class *fp;  // file we are reading wav from
  
  w32 buf_size, start_file_offset, prev_total, finish_pos;
  
  w32 total_size;
  w32 total_read;

  i4_bool first_time, loop, fits_in_memory, file_finished;
  
  // loads more data from disk or clears the buffer
  void load_buffer(i4_bool async=i4_T);

public:

  // - fp should remain open and will be closed by stream_wav's destructor
  // - buffer_size is the size of the sound buffer to keep in memory, half the
  //   sound buffer we be loaded in over time
  // - if loop is false, wav will play 0's ater song is over
  // - if you need to sync the start of the sound you should specifiy first_load_is_async
  //   is false in which case the program will block until first section of sample is loaded
  i4_voice_class *i4_voice() { return voice; }

  i4_stream_wav_player(i4_file_class *fp, 
                       w32 buffer_size, 
                       i4_bool loop,
                       i4_bool first_load_is_async=i4_T,
                       i4_bool is_3d_capable=i4_F);

  // called by async_read, do not call directly
  void PRIVATE_callback(w32 count);

  // should be called by main program at least 1/time per second, this routine
  // checks to see if new wav needs to be loaded, return i4_F if song is done,
  // if looping is on, poll will never report finished
  i4_bool poll();

  void pause();
  void unpause();

  void set_volume(i4_volume vol);
  i4_bool load_failed() { return voice==0; }
  
  // this will free sound memory, and delete the file pointer and make sure there
  // aren't any async_read's waiting to finish
  ~i4_stream_wav_player();
};


#endif
