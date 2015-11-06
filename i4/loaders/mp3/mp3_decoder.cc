/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "main/main.hh"
#include "error/alert.hh"
#include "error/error.hh"
#include "file/file.hh"
#include "string/str_checksum.hh"
#include "memory/malloc.hh"
#include "loaders/mp3_load.hh"
#include "loaders/wav_load.hh"
#include "status/status.hh"



void write_header(i4_file_class *fp, i4_sound_info &fmt)
{
  fp->write("RIFF",4);
  fp->write_32(36+fmt.size);       // 36 + snd->size
  fp->write("WAVE", 4);
  fp->write("fmt ", 4);

  fp->write_32(16);        // size of chunk

  fp->write_16(1);         // data type

  fp->write_16(fmt.channels);           // num channels
  fp->write_32(fmt.sample_rate);   // sample rate
  fp->write_32(fmt.sample_rate*
               fmt.sample_size*fmt.channels);       // snd->sample_rate * snd->sample_size
  fp->write_16(fmt.sample_size * fmt.channels);
  fp->write_16(fmt.sample_size*8);


  fp->write("data",4);
  fp->write_32(fmt.size);       // snd->size

}

void i4_main(w32 argc, i4_const_str *argv)
{
  i4_init();


  i4_file_class *in=i4_open("in.mp3");
  i4_file_class *out=i4_open("out.wav", I4_WRITE);

  i4_sound_info fmt;
//   fmt.channels=2;
//   fmt.sample_rate=44*1024;
//   fmt.sample_size=2;
//   fmt.size=500000;

  write_header(out, fmt);
  int header_size=out->tell();

  i4_status_class *status=i4_create_status("decoding");

  if (i4_load_mp3(in, out, fmt, status))
    i4_warning("decode success");
  else
    i4_warning("decode failed");

  int size=out->tell();
  out->seek(0);

  write_header(out, fmt);
  out->seek(size);

  delete status;

  delete in;
  delete out;

 
  in=i4_open("out.wav");
  if (i4_load_wav_info(in, fmt))
  {
    i4_warning("wrote :\n"
               "sample size = %d\n"
               "channels    = %d\n"
               "sample_rate = %d\n"
               "size        = %d\n",
               fmt.sample_size,
               fmt.channels,
               fmt.sample_rate,
               fmt.size);
  }
  delete in;
  
  i4_uninit();

}
