/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef D3DSOUND_HH
#define D3DSOUND_HH

#include <dsound.h>
#include "sound/sound3d.hh"

class d3dsound_buffer_class : public i4_3d_voice_class
{
public:
  w32 flags;
  w8 *data;
  DS3DBUFFER ds_param;
  IDirectSoundBuffer *pDSB;
  IDirectSound3DBuffer *pDS3DB;
  completion_function_type completer;
  void *context;

  d3dsound_buffer_class() : pDSB(0), pDS3DB(0), data(0) {}

  ~d3dsound_buffer_class()
  //{{{
  {
    if (pDS3DB)
      pDS3DB->Release();
    if (pDSB)
      pDSB->Release();
  }
  //}}}

  virtual void set_completer(completion_function_type _completer, void *_context);
  virtual void play();
  virtual void update();
};

class direct_3dsound_class : public i4_3d_sound_manager_class
{
protected:
  LPDIRECTSOUND lpDirectSound;
  IDirectSoundBuffer *lpPrimary;
  d3dsound_buffer_class *sound;

  void load(w32 max_sounds);
public:
  I4_SOUND_NAME("DirectSound");
  
  virtual void load_sounds(w32 max_sounds);

  virtual i4_3d_voice_class *alloc(i4_sound_id sound_id, const i4_3d_sound_parameters& param);
};
extern direct_3dsound_class direct_3dsound;

#endif

//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}
