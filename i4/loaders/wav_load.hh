/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef WAV_LOAD_HH
#define WAV_LOAD_HH

#include "arch.hh"

class i4_file_class;

// structure used to describe a sound
class i4_sound_info
{
public:
  w32 size;          // size of sound data in bytes
  w32 sample_rate;   // anything i.e. 22000
  w32 channels;      // 1 or 2
  w32 sample_size;   // 1 or 2 
};


// fills in the info field and advances file pointer to where sound data start
// returns false if something screwy is detected in the wav file
i4_bool i4_load_wav_info(i4_file_class *f, i4_sound_info &info);



#endif
