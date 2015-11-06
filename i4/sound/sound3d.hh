/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef SOUND3D_HH
#define SOUND3D_HH

#include "sound/sound_types.hh"
#include "init/init.hh"
#include "math/vector.hh"

class i4_3d_sound_parameters
//{{{
{
protected:
  i4_3d_sound_parameters() {}
public:
  static i4_3d_vector null;

  i4_frequency frequency;
  i4_volume volume;
  i4_3d_vector position, velocity;
  i4_bool looping, reverb;
  
  i4_3d_sound_parameters(i4_frequency freq,
                         i4_volume vol = I4_SOUND_VOLUME_LEVELS-1,
                         const i4_3d_vector& pos = null,
                         const i4_3d_vector& vel = null,
                         i4_bool looping = i4_F,
                         i4_bool reverb = i4_F)
    : frequency(freq), 
      volume(vol), 
      position(pos),
      velocity(vel),
      looping(looping), 
      reverb(reverb) {}

  void set(const i4_3d_sound_parameters &param)
  //{{{
  {
    frequency = param.frequency;
    volume = param.volume;
    position = param.position; 
    velocity = param.velocity;
    looping = param.looping; 
    reverb = param.reverb;
  }
  //}}}
};
//}}}

class i4_3d_voice_class : public i4_3d_sound_parameters
{
protected:
  i4_3d_voice_class() : i4_3d_sound_parameters() {}
public:
  i4_3d_voice_class(const i4_3d_sound_parameters &param)
    : i4_3d_sound_parameters(param.frequency, 
                             param.volume, 
                             param.position, 
                             param.velocity,
                             param.looping, 
                             param.reverb) {}

  typedef i4_bool (*completion_function_type)(void *_context);

  virtual void set_completer(completion_function_type completer, void *_context) = 0;
  virtual void play() = 0;
  virtual void update() = 0;
};

class i4_3d_sound_manager_class;
extern i4_3d_sound_manager_class *i4_3d_sound_man;

class i4_3d_sound_manager_class : public i4_init_class
{
protected:
public:
  virtual void init() { i4_3d_sound_man = this; }
  virtual void load_sounds(w32 max_sounds) = 0;
  virtual i4_3d_voice_class *alloc(i4_sound_id sound_id, const i4_3d_sound_parameters& param) = 0;
  i4_3d_voice_class *play(i4_sound_id sound_id, const i4_3d_sound_parameters& param)
  //{{{
  {
    i4_3d_voice_class *voc = alloc(sound_id,param);
    
    if (voc)
      voc->play();

    return voc;
  }
  //}}}

#ifndef I4_RETAIL
  virtual char *name() const = 0;
#endif
};

class i4_null_3d_sound_class;
extern class i4_null_3d_sound_class i4_null_3d_sound;

#endif

//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}
