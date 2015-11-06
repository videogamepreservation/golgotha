/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef LINUX_SOUND_HH
#define LINUX_SOUND_HH

#include "sound/sound.hh"
#include "math/fixed_point.hh"
#include <pthread.h>

typedef i4_fixed_point<10> linux_sound_index;
enum { LINUX_SOUND_NUM_VOICE = 8 };

class linux_sample_class
{
public:
  w8 *data;
  linux_sound_index sample_rate;
  linux_sound_index size;
};

class linux_sound_class;

class linux_voice_class : public i4_voice_class
{
public:
  linux_sound_index index;
  linux_sound_index increment;
  w16 volume;
  sw16 pan;
  w16 left_vol,right_vol;
  i4_bool looping, active;
  linux_sample_class *sound;

  linux_voice_class() : sound(0) {}

  ~linux_voice_class() {}

  virtual void stop() = 0;
  virtual void play();
  virtual void set_frequency(i4_frequency freq);
  virtual void set_volume(i4_volume vol);
  virtual void set_pan(i4_pan pan);
};

class linux_sound_class : public i4_sound_manager_class
{
  friend void *linux_sound_mixer(void *arg);
protected:
  int fd;
  linux_sample_class *sound;
  linux_voice_class voice[LINUX_SOUND_NUM_VOICE];
  int thread_state;

  void start_thread();
  void stop_thread();

  void initialize_volume_table();
public:
  I4_SOUND_NAME("Linux Sound");
  
  virtual void init();
  virtual void uninit() { stop_thread(); }

  virtual void load_sounds(w32 max_sounds);

  virtual i4_voice_class *alloc(i4_sound_id sound_id, const sound_parameters& param);
};
extern linux_sound_class linux_sound;

#endif

//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}
