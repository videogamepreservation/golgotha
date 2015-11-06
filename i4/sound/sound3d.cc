/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "sound/sound3d.hh"

i4_3d_vector i4_3d_sound_parameters::null(0,0,0);

class i4_null_3d_voice_class : public i4_3d_voice_class
//{{{
{
protected:
  completion_function_type complete;
  void *context;
public:
  i4_null_3d_voice_class() : i4_3d_voice_class(), complete(0), context(0) {}

  virtual void set_completer(completion_function_type _complete, void *_context)
  //{{{
  {
    complete = _complete;
    context = _context;
  }
  //}}}
  virtual void play() { if (complete) (*complete)(this); }
  virtual void update() {}
};
//}}}

class i4_null_3d_sound_class : public i4_3d_sound_manager_class
//{{{
{
public:
  i4_null_3d_voice_class null_3d_voice;

  I4_SOUND_NAME("No 3D sound");

  virtual void init() {}
  virtual void load_sounds(w32 max_sounds) { }

  virtual i4_3d_voice_class *alloc(i4_sound_id sound_id, const i4_3d_sound_parameters& param)
  //{{{
  {
    return &null_3d_voice;
  }
  //}}}
};
//}}}

i4_null_3d_sound_class i4_null_3d_sound;

i4_3d_sound_manager_class *i4_3d_sound_man = &i4_null_3d_sound;

//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}
