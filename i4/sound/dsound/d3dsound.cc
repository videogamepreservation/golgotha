/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

//#define INITGUID

#include "error/error.hh"
#include "error/alert.hh"
#include "sound/dsound/d3dsound.hh"
#include "loaders/wav_load.hh"
#include "string/string.hh"
#include "file/file.hh"
#include "video/win32/win32.hh"

direct_3dsound_class direct_3dsound;
static sw16 volume_table[I4_SOUND_VOLUME_LEVELS] = 
{
  // Volume ramp
  //{{{ Note:
  //  Generated with:
  //
  // perl -e 'printf "  %6d, ", -10000; for ($i=1; $i<64; $i++)
  //   { printf( "%6d, ", 1000*log($i/63)); if ($i%10 == 9) { print "\n  "; }}; print "\n"'
  //}}}

  -10000,  -4143,  -3449,  -3044,  -2756,  -2533,  -2351,  -2197,  -2063,  -1945, 
   -1840,  -1745,  -1658,  -1578,  -1504,  -1435,  -1370,  -1309,  -1252,  -1198, 
   -1147,  -1098,  -1052,  -1007,   -965,   -924,   -885,   -847,   -810,   -775, 
    -741,   -709,   -677,   -646,   -616,   -587,   -559,   -532,   -505,   -479, 
    -454,   -429,   -405,   -381,   -358,   -336,   -314,   -292,   -271,   -251, 
    -231,   -211,   -191,   -172,   -154,   -135,   -117,   -100,    -82,    -65, 
     -48,    -32,    -16,      0
};

int check_result(HRESULT err)
//{{{
{
  switch (err)
  {
    case DS_OK:
      return 1;
      break;
      
    case DSERR_ALLOCATED:
      i4_warning("i4_direct_sound - resources already used by another.");
      break;
      
    case DSERR_ALREADYINITIALIZED:
      i4_warning("i4_direct_sound - object already initialized.");
      break;
      
    case DSERR_BADFORMAT:
      i4_warning("i4_direct_sound - format not supported.");
      break;
      
    case DSERR_BUFFERLOST:
      i4_warning("i4_direct_sound - buffer memory lost.");
      break;
      
    case DSERR_CONTROLUNAVAIL:
      i4_warning("i4_direct_sound - control is unavailable.");
      break;
      
    case DSERR_GENERIC:
      i4_warning("i4_direct_sound - error.");
      break;
      
    case DSERR_INVALIDCALL:
      i4_warning("i4_direct_sound - function is invalid.");
      break;
      
    case DSERR_INVALIDPARAM:
      i4_warning("i4_direct_sound - invalid parameter.");
      break;
      
    case DSERR_NOAGGREGATION:
      i4_warning("i4_direct_sound - object does not support aggregation.");
      break;
      
    case DSERR_NODRIVER:
      i4_warning("i4_direct_sound - no sound driver available.");
      break;
      
    case DSERR_OUTOFMEMORY:
      i4_warning("i4_direct_sound - out of memory.");
      break;
      
    case DSERR_PRIOLEVELNEEDED:
      i4_warning("i4_direct_sound - insufficient priority level to perform function.");
      break;
      
    case DSERR_UNINITIALIZED:
      i4_warning("i4_direct_sound - uninitializd sound system.");
      break;
      
    case DSERR_UNSUPPORTED:
      i4_warning("i4_direct_sound - function currently unsupported.");
      break;
  }
  return 0;
}
//}}}


void d3dsound_buffer_class::play()
//{{{
{
  if (!pDSB)
    return;

  //  pDSB->SetCurrentPosition(0);
  pDSB->Play(0,0,flags);
}
//}}}


void d3dsound_buffer_class::update()
//{{{
{
  if (!pDSB)
    return;

  pDSB->SetFrequency(frequency);
  pDSB->SetVolume(volume_table[volume]);

  ds_param.vPosition.x = position.x*0.01;
  ds_param.vPosition.y = position.y*0.01;
  ds_param.vPosition.z = position.z*0.01;
  ds_param.vVelocity.x = velocity.x*0.5;
  ds_param.vVelocity.y = velocity.y*0.5;
  ds_param.vVelocity.z = velocity.z*0.5;

  pDS3DB->SetAllParameters(&ds_param, DS3D_IMMEDIATE);
}
//}}}


void d3dsound_buffer_class::set_completer(completion_function_type _completer, void *_context)
//{{{
{
  completer = _completer;
  context = _context;
}
//}}}


void direct_3dsound_class::load(w32 max_sounds)
//{{{
{
  sound = new d3dsound_buffer_class[max_sounds];

  i4_sound_info info;
  i4_const_str *sounds=i4_string_man.get_array("sounds");
  
  for (w32 count=0; !sounds[count].null(); count++)
  {  
    i4_file_class *fp=i4_file_man.open(sounds[count]);
    if (!fp)    
      i4_alert(i4gets("file_missing"),200,&sounds[count]);
    else
    {
      if (i4_load_wav(fp,info))
      {
        DSBUFFERDESC dsBD = {0};
        PCMWAVEFORMAT pcmwf;
        d3dsound_buffer_class *snd = &sound[count];
      
        ZeroMemory(&pcmwf, sizeof(PCMWAVEFORMAT));
        pcmwf.wf.wFormatTag = WAVE_FORMAT_PCM;
        pcmwf.wf.nChannels = info.channels;
        pcmwf.wf.nSamplesPerSec = info.sample_rate;
        pcmwf.wf.nBlockAlign = info.sample_size*info.channels;
        pcmwf.wf.nAvgBytesPerSec = pcmwf.wf.nSamplesPerSec * pcmwf.wf.nBlockAlign;
        pcmwf.wBitsPerSample = info.sample_size*8;
      
        dsBD.dwSize = sizeof(dsBD);
        dsBD.dwFlags = DSBCAPS_CTRL3D | DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLVOLUME;
        dsBD.dwBufferBytes = info.size;
        dsBD.lpwfxFormat = (LPWAVEFORMATEX)&pcmwf;
      
        if (!check_result(lpDirectSound->CreateSoundBuffer(&dsBD, &snd->pDSB, 0)))
        {
          i4_warning("couldn't create sound buffer");
          snd->pDSB = 0;
          return;
        }
      
        if (!check_result(snd->pDSB->QueryInterface(IID_IDirectSound3DBuffer, 
                                                    (void**)&snd->pDS3DB)))
        {
          i4_warning("couldn't create 3D sound buffer");
          snd->pDSB = 0;
          snd->pDS3DB = 0;
          return;
        }

        snd->ds_param.dwSize = sizeof(snd->ds_param);
        snd->ds_param.vPosition.x = 0;
        snd->ds_param.vPosition.y = 0;
        snd->ds_param.vPosition.z = 0;
        snd->ds_param.vVelocity.x = 0;
        snd->ds_param.vVelocity.y = 0;
        snd->ds_param.vVelocity.z = 0;
        snd->ds_param.dwInsideConeAngle = 360;
        snd->ds_param.dwOutsideConeAngle = 360;
        snd->ds_param.vConeOrientation.x;
        snd->ds_param.vConeOrientation.y;
        snd->ds_param.vConeOrientation.z;
        snd->ds_param.lConeOutsideVolume = 0;
        snd->ds_param.flMinDistance = 1;
        snd->ds_param.flMaxDistance = 100;
        snd->ds_param.dwMode = DS3DMODE_NORMAL;

        check_result(snd->pDS3DB->SetAllParameters(&snd->ds_param, DS3D_IMMEDIATE));
      
        LPVOID m1,m2;
        DWORD s1,s2;
      
        snd->data = (w8*)info.data;
      
        if (!check_result(snd->pDSB->Lock(0, info.size, &m1, &s1, &m2, &s2, 0)))
        {
          i4_warning("i4_sound_manager_class::play - couldn't get write access to sound buffer");
          snd->pDSB->Release();
          snd->pDSB = 0;
          return;
        }
      
        CopyMemory(m1, snd->data, s1);
      
        if (s2 != 0)
          CopyMemory(m2, snd->data+s1, s2);
      
        if (!check_result(snd->pDSB->Unlock(m1,s1,m2,s2)))
        {
          i4_warning("i4_3d_sound_manager_class::load - couldn't unlock sound buffer");
          snd->pDSB->Release();
          snd->pDSB = 0;
          return;
        }
      }
      delete fp;
    }
  }

  i4_free(sounds);
}
//}}}


void direct_3dsound_class::load_sounds(w32 max_sounds)
//{{{
{
  i4_3d_sound_man = (i4_3d_sound_manager_class*)&i4_null_3d_sound;

  DSBUFFERDESC dsBD;

  if (DirectSoundCreate(0, &lpDirectSound, 0) != DS_OK)
  {
    i4_warning("i4_sound_manager_class::load_sounds - DirectSound not initialized");
    return;
  }

  if (lpDirectSound->SetCooperativeLevel(win32_display_instance.window_handle,
                                         DSSCL_EXCLUSIVE) != DS_OK)
  {
    i4_warning("i4_sound_manager_class::load_sounds - couldn't get exclusive sound");
    return;
  }

  ZeroMemory( &dsBD, sizeof(DSBUFFERDESC) ); 
  dsBD.dwSize = sizeof(dsBD);
  dsBD.dwFlags = DSBCAPS_PRIMARYBUFFER;

  if (!check_result( lpDirectSound->CreateSoundBuffer( &dsBD, &lpPrimary, 0)))
  {
    i4_warning("i4_sound_manager_class::load_sounds - couldn't create primary buffer");
    return;
  }

#if 0
  PCMWAVEFORMAT pcmwf;
  ZeroMemory(&pcmwf, sizeof(PCMWAVEFORMAT));
  pcmwf.wf.wFormatTag = WAVE_FORMAT_PCM;
  pcmwf.wf.nChannels = 1;
  pcmwf.wf.nSamplesPerSec = 22050;
  pcmwf.wf.nBlockAlign = 1;
  pcmwf.wf.nAvgBytesPerSec = pcmwf.wf.nSamplesPerSec * pcmwf.wf.nBlockAlign;
  pcmwf.wBitsPerSample = 8;

  if (!check_result( lpPrimary->SetFormat((LPWAVEFORMATEX)&pcmwf) ))
  {
    i4_warning("i4_sound_manager_class::load_sounds - couldn't format primary buffer");
    return;
  }
#endif

  if (!check_result( lpPrimary->Play( 0, 0, DSBPLAY_LOOPING ) ))
  {
    i4_warning("i4_sound_manager_class::load_sounds - couldn't start primary buffer");
    return;
  }

  lpPrimary->Release();

  i4_3d_sound_man = this;

  load(max_sounds);
}
//}}}


i4_3d_voice_class *direct_3dsound_class::alloc(i4_sound_id sound_id, 
                                               const i4_3d_sound_parameters& param)
//{{{
{
  sound[sound_id].flags = 0;
  if (param.looping)
    sound[sound_id].flags |= DSBPLAY_LOOPING;

  sound[sound_id].set_completer(0,0);
  sound[sound_id].set(param);
  sound[sound_id].update();

  return &sound[sound_id];
}
//}}}


//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}
