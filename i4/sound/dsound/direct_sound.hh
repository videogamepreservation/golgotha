/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef DIRECT_SOUND_HH
#define DIRECT_SOUND_HH

#include "sound/sound.hh"
#include "math/transform.hh"
#include <dsound.h>
#include "sound/dsound/ia3d.h"

class dsound_buffer_class : public i4_voice_class
{
public:
  DWORD flags;
  IDirectSoundBuffer   *pDSB;
  IDirectSound3DBuffer *p3DSB;  
  IDirectSoundNotify   *pNotify;
  i4_stream_wav_player *stream_man;
  float                 hearable_distance;

  i4_stream_wav_player *stream_manager() { return stream_man; }
  void set_stream_manager(i4_stream_wav_player *s) { stream_man = s; }

  dsound_buffer_class(IDirectSoundBuffer *_pDSB,
                      DWORD _flags, w32 _buffer_size);
  
  ~dsound_buffer_class();
  
  virtual void stop();
  virtual void play();
  virtual void set_frequency(i4_frequency freq);
  virtual void set_volume(i4_volume vol);
  virtual void set_pan(i4_pan pan);
  virtual void set_looping(i4_bool yes_no);
  virtual void set_3d_position(i4_float x, i4_float y, i4_float z, i4_bool immediately);
  virtual void set_3d_velocity(i4_float x,i4_float y,i4_float z, i4_bool immediately);

  virtual void lock(w32 start_position, w32 size, 
                    void *&block1, w32 &block1_size,
                    void *&block2, w32 &block2_size);

  virtual void unlock(void *block1, w32 block1_size,
                      void *block2, w32 block2_size);

  //returns whether or not the sound is playing
  virtual i4_bool is_playing();


  i4_frequency get_frequency();  

  i4_volume get_volume();

  i4_pan get_pan();


  virtual w32 get_sound_position();  // current position of the sound
  virtual void set_sound_position(w32 pos);

  virtual void set_hearable_distance(float x) { hearable_distance=x; }
  virtual float get_hearable_distance() { return hearable_distance; }
};

class direct_sound_class : public i4_sound_manager_class
{
protected:
  LPDIRECTSOUND lpDirectSound;
  IA3d *lpA3D;
  IDirectSoundBuffer *lpPrimary;  

  void init();
  void uninit();

  i4_bool initialized;
  i4_bool setup();  


public:
  i4_bool use_3d_sound;

  i4_3d_vector listener_position;
  i4_transform_class listener_transform;

  IDirectSound3DListener *lpListener;

  direct_sound_class()
  {
    initialized=i4_F;
  }

  I4_SOUND_NAME("DirectSound");
  
  void set_listener_velocity(i4_float x,i4_float y,i4_float z);
  void set_listener_position(i4_float x,i4_float y,i4_float z);

  i4_voice_class *alloc(w32 buffer_size, sound_parameters &description);
  
  i4_voice_class *duplicate_2d(i4_voice_class *voice);
  i4_voice_class *duplicate_3d(i4_voice_class *voice);
  
  void set_listener_orientation(i4_float f_x,i4_float f_y,i4_float f_z,
                                i4_float u_x,i4_float u_y,i4_float u_z);

  void commit_3d_changes();

  void free_voice(i4_voice_class *voice);

};

extern direct_sound_class direct_sound;

#endif
