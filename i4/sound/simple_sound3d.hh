/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef SIMPLE_SOUND3D_HH
#define SIMPLE_SOUND3D_HH

#include "sound/sound3d.hh"
#include "sound/sound.hh"

class i4_simple_3d_voice_class : public i4_3d_voice_class 
{
protected:
  i4_voice_class *voc;
public:
  i4_simple_3d_voice_class(i4_voice_class *_voc, const i4_3d_sound_parameters &param) :
    voc(_voc), i4_3d_voice_class(param)
  //{{{
  {

  }
  //}}}

  virtual void set_completer(completion_function_type completer, void *_context)
  //{{{
  {
    voc->set_completer((i4_voice_class::completion_function_type)completer,_context);
  }
  //}}}
  virtual void play();
  virtual void update();
};

class i4_simple_3d_sound_manager_class : public i4_3d_sound_manager_class 
{
protected:
public:
  I4_SOUND_NAME("SimpleSound");

  virtual void init();
  virtual void load_sounds(w32 max_sounds);
  virtual i4_3d_voice_class *alloc(i4_sound_id sound_id, const i4_3d_sound_parameters& param);
};

#endif

//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}
