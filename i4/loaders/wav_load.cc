/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "loaders/wav_load.hh"
#include "file/file.hh"

// system independently ordered 4 byte id
inline w32 CtoID(char a, char b, char c, char d)
{
  return l_to_msb( (((w32)a)<<24) | (((w32)b)<<16) | (((w32)c)<<8) | ((w32)d) );
}


i4_bool i4_load_wav_info(i4_file_class *f, i4_sound_info &info)
{
  w32 end_pos, chunk_type, len;

  // look for RIFF signature
  if (f->read_32() != CtoID('R','I','F','F'))
    return i4_F;

  end_pos = f->read_32();
  end_pos += f->tell();

  // look for WAVE sig
  if (f->read_32() != CtoID('W','A','V','E'))
    return i4_F;

  // read all data chunks
  while (f->tell() < end_pos)
  {
    chunk_type = f->read_32();
    len = f->read_32();

    if      (chunk_type == CtoID('f','m','t',' '))
    {
      // sound format info
      f->read_16();
      info.channels = f->read_16();
      info.sample_rate = f->read_32();
      f->read_32();
      f->read_16();
      info.sample_size = f->read_16()/8;

      f->seek(f->tell() + len - 16);
    }
    else if (chunk_type == CtoID('d','a','t','a'))
    {
      info.size = len;

      return i4_T;
    }
    else
      f->seek(f->tell() + len);

  }

  return i4_F;
}
