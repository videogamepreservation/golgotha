/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "sound/sound.hh"
#include "main/main.hh"
#include "string/string.hh"

i4_sound_manager_class i4_null_sound;
i4_sound_manager_class *i4_sound_man = &i4_null_sound;

static i4_sound_manager_class *i4_disabled_sound_man = &i4_null_sound;


void i4_sound_manager_class::init()
{ 
  for (int i=1; i<i4_global_argc; i++)
    if (i4_global_argv[i]=="-no_sound")
      return;

  if (this!=&i4_null_sound) 
    i4_sound_man=this;    
}


i4_sound_manager_class::sound_parameters::sound_parameters
     (i4_frequency  _frequency,
      w32           _channels,
      w32           _sample_size,
      i4_volume     _volume,
      i4_pan        _pan,
      i4_bool       _looping,
      i4_bool       _reverb,
      i4_bool       _streaming,
      i4_bool       _capable_3d)
{
  frequency   = _frequency;
  channels    = _channels;
  sample_size = _sample_size;
  volume      = _volume;
  pan         = _pan;
  looping     = _looping;
  reverb      = _reverb;
  streaming   = _streaming;
  capable_3d  = _capable_3d;
}
