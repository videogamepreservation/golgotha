/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef SOUND_HH
#define SOUND_HH

#include "math/num_type.hh"
#include "init/init.hh"
#include "sound/sound_types.hh"

class i4_stream_wav_player;

class i4_voice_class
{
public:
    
  i4_frequency default_frequency;
  w32          sound_length;


  virtual void stop() = 0;
  virtual void play() = 0;
  virtual i4_bool is_playing() = 0;
  virtual void set_frequency(i4_frequency freq) = 0;
  virtual void set_volume(i4_volume vol) = 0;  
  virtual void set_pan(i4_pan pan) = 0;
  virtual void set_looping(i4_bool yes_no) = 0;   // must be called before play

  virtual i4_frequency get_frequency() = 0;
  virtual i4_volume get_volume() = 0;
  virtual i4_pan get_pan() = 0;

  virtual void lock(w32 start_position, w32 size, 
                    void *&block1, w32 &block1_size,
                    void *&block2, w32 &block2_size) = 0;

  virtual void unlock(void *block1, w32 block1_size,
                      void *block2, w32 block2_size) = 0;

  virtual w32 get_sound_position() = 0; // current play position in bytes
  virtual void set_sound_position(w32 pos) = 0;

  virtual void set_hearable_distance(float x) = 0;
  virtual float get_hearable_distance() = 0;
  
  virtual ~i4_voice_class() { ; }

  // these function are only available if you created a 3d sound
  virtual void set_3d_position(i4_float x, i4_float y, i4_float z, i4_bool immediately) = 0;
  virtual void set_3d_velocity(i4_float x,i4_float y,i4_float z,i4_bool immediately) = 0;
};

class i4_sound_manager_class;
extern i4_sound_manager_class *i4_sound_man;
extern i4_sound_manager_class i4_null_sound;

class i4_sound_manager_class : public i4_init_class
{
public:
  class sound_parameters
  {
  public:
    w32          channels;     //1 for mono,  2 for stereo
    w32          sample_size;  //1 for 8-bit, 2 for 16-bit
    i4_frequency frequency;
    i4_volume    volume;
    i4_pan       pan;
    i4_bool      looping, 
                 reverb,
                 streaming,
                 capable_3d; //if you want this to be capable of being used as a 3d sound

    sound_parameters(i4_frequency  _frequency   = 11025,
                     w32           _channels    = 1,
                     w32           _sample_size = 1,
                     i4_volume     _volume      = I4_SOUND_VOLUME_LEVELS-1,
                     i4_pan        _pan         = 0,
                     i4_bool       _looping     = i4_F,
                     i4_bool       _reverb      = i4_F,
                     i4_bool       _streaming   = i4_F,
                     i4_bool       _capable_3d  = i4_F);
  };

  virtual void init();

  // if you want to play a sound twice (mix with self) you have to duplicate it
  // and play the duplicate and the oruginal,  the duplicate will not duplicate the
  // actual data in the sound buffer, but rather the position, frequency, etc.
  virtual i4_voice_class *duplicate_2d(i4_voice_class *voice) { return 0; }
  virtual i4_voice_class *duplicate_3d(i4_voice_class *voice) { return 0; }

  // returns 0 not enough memory to alloc buffer
  virtual i4_voice_class *alloc(w32 buffer_size, sound_parameters &description) { return 0; }

  virtual void set_listener_velocity(i4_float x,i4_float y,i4_float z){}
  virtual void set_listener_position(i4_float x,i4_float y,i4_float z){}
  virtual void set_listener_orientation(i4_float f_x,i4_float f_y,i4_float f_z,
                                        i4_float u_x,i4_float u_y,i4_float u_z){}

  virtual void commit_3d_changes() {}

  virtual void free_voice(i4_voice_class *voice) {delete voice;}

#ifndef I4_RETAIL
  virtual char *name() { return "null sound"; }
#endif
};



#endif
