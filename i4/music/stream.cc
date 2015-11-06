/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "music/stream.hh"
#include "loaders/wav_load.hh"
#include "file/file.hh"
#include "sound/sound.hh"
#include "error/error.hh"

#include <memory.h>

void i4_wav_callback(w32 count, void *context)
{
  ((i4_stream_wav_player *)context)->PRIVATE_callback(count);
}

void i4_stream_wav_player::set_volume(i4_volume vol)
{
  if (voice)
    voice->set_volume(vol);
}

// loads more data from disk or clears the buffer
void i4_stream_wav_player::load_buffer(i4_bool async)
{
  wait_read  = i4_F;
  prev_total = 0;

  if (async)
  {
    wait_read = i4_T; //we're waiting for this read to finish now

    if (!fp->async_read(locked_buffer_start, locked_buffer_size, i4_wav_callback, this))
    {
      wait_read = i4_F;

      i4_warning("i4_stream_wav_player::async read failed");
      i4_wav_callback(fp->read(locked_buffer_start, locked_buffer_size), this);
    }
  }
  else
    i4_wav_callback(fp->read(locked_buffer_start, locked_buffer_size), this);
}


i4_stream_wav_player::i4_stream_wav_player(i4_file_class *_fp,
                                           w32 _buf_size, 
                                           i4_bool _loop,
                                           i4_bool first_load_is_async,
                                           i4_bool _3d_capable)
{
  
  fp            = _fp;
  loop          = _loop;
  buf_size      = _buf_size;
  first_time    = i4_T;
  file_finished = i4_F;
  voice         = 0;
 
  // don't bother if there is no sound
  if (i4_sound_man==&i4_null_sound)
    return ;
 
  i4_sound_info info;
  if (i4_load_wav_info(fp, info))
  {    
    total_size = info.size;

    if (info.size <= buf_size)
    {
      buf_size       = info.size;
      fits_in_memory = i4_T;
    }
    else
      fits_in_memory = i4_F;

    if (_3d_capable)
      first_time = i4_F;

    i4_sound_manager_class::sound_parameters p(info.sample_rate,
                                               info.channels,
                                               info.sample_size,
                                               I4_SOUND_VOLUME_LEVELS-1,
                                               0,      // pan = 0
                                               (loop || !fits_in_memory), // loop ones that dont fit in memory
                                               0,      // no reverb
                                               i4_T,   // turn on streaming
                                               _3d_capable);  // 3d capable

    voice = i4_sound_man->alloc(buf_size, p); 
    if (voice)
    {
      start_file_offset = fp->tell();

      total_read = 0;
      
      last_read = SECOND_HALF;
      
      //poll() will see that the second half has already been read, wont read in
      //the first half until the play cursor gets halfway into the buffer

      voice->lock(0, buf_size, locked_buffer_start, locked_buffer_size, unused1, unused2);
      
      load_buffer(first_load_is_async);
    }
  }
}

//be careful in this function, its recursive (although, it recurses in a different thread)
//but watch out for how you manipulate the variable wait_read 
void i4_stream_wav_player::PRIVATE_callback(w32 count)
{
  total_read += count;

  if (total_read > total_size)
  {
    count -= (total_read - total_size); //we really only read this many *valid* bytes
    total_read = total_size;
  }

  if (count + prev_total < locked_buffer_size)
  {
    // we reached the end of the sample data    

    if (loop)
    {
      //start back at the beginning
      
      fp->seek(start_file_offset);
      total_read = 0;
      
      prev_total += count;

      if (!fp->async_read(((w8 *)locked_buffer_start) + prev_total, locked_buffer_size - prev_total, i4_wav_callback, this))
      {
        wait_read = i4_F;

        i4_warning("i4_stream_wav_player::async read failed");
        i4_wav_callback(fp->read(((w8 *)locked_buffer_start) + prev_total, locked_buffer_size - prev_total), this);
      }
    }
    else
    {
      //indicate that we're done
      file_finished = i4_T;

      //set the remaining part of the buffer to 0
      
      w8 *start_clearing_here = (w8 *)locked_buffer_start + count;

      memset(start_clearing_here, 0, locked_buffer_size - count);
      
      voice->unlock(locked_buffer_start, locked_buffer_size, unused1, unused2);

      finish_pos = count;
      
      wait_read  = i4_F;
    }
  }
  else
  {
    if (fits_in_memory)
    {
      file_finished = i4_T;
      finish_pos    = total_size;
    }

    voice->unlock(locked_buffer_start, locked_buffer_size, unused1, unused2);
    
    wait_read = i4_F;
  }

  if (first_time)     // if this was the first load, we need to start playing the sound
  {
    voice->play();
    first_time = i4_F;
  }
}


void i4_stream_wav_player::pause()
{
  if (voice && voice->is_playing())
    voice->stop();
}

void i4_stream_wav_player::unpause()
{
  if (voice && !voice->is_playing())
    voice->play();
}


i4_bool i4_stream_wav_player::poll()
{
  if (!voice)
    return i4_F;

  if (wait_read)   // if we are waiting for a read to finish or init failed
    return i4_T;

  if (fits_in_memory && loop)
    return i4_T;

  w32 pos = voice->get_sound_position();
    
  if (!file_finished && !fits_in_memory)
  {
    if (pos > buf_size/2 && last_read!=FIRST_HALF)
    {
      //read more into the first half of the buffer
      voice->lock(0, buf_size/2, locked_buffer_start, locked_buffer_size, unused1, unused2);
      last_read = FIRST_HALF;
      load_buffer();
    }
    else
    if (pos < buf_size/2 && last_read != SECOND_HALF)
    {
      //read more into the second half of the buffer
      voice->lock(buf_size/2, buf_size/2, locked_buffer_start, locked_buffer_size, unused1, unused2);
      last_read = SECOND_HALF;
      load_buffer();
    }
  }
  else
  {
    if (!voice->is_playing())
      return i4_F;

    if (pos < buf_size/2)
    {
      if (last_read==FIRST_HALF)
      {
        if (pos >= finish_pos)
        {
          voice->stop();
          return i4_F;
        }
      }
    }
    else
    {
      if (last_read==SECOND_HALF)
      {
        if (pos >= finish_pos+buf_size/2)
        {
          voice->stop();
          return i4_F;
        }
      }
    }
  }
  
  return i4_T;
}

i4_stream_wav_player::~i4_stream_wav_player()
{
  if (voice)
  {
    while (wait_read);    // if we need to wait for an async read to finish

    i4_sound_man->free_voice(voice);
    
    voice = 0;
  }

  if (fp)
  {
    delete fp;
    fp = 0;
  }
}
