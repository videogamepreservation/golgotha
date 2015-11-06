/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "error/error.hh"
#include "error/alert.hh"
#include "sound/linux/linux_sound.hh"
#include "loaders/wav_load.hh"
#include "string/string.hh"
#include "file/file.hh"

#include <linux/soundcard.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

enum { LINUX_SOUND_BUFFER_BITS = 11 };
enum { LINUX_SOUND_SAMPLE_SPEED = 11025 };

// Thread State Enumeration
enum
{
  LINUX_SOUND_UNINITIALIZED,
  LINUX_SOUND_RUNNING,
  LINUX_SOUND_REQUEST_STOP,
  LINUX_SOUND_STOPPED,
};

static sw16 volume_table[I4_SOUND_VOLUME_LEVELS][256];
static sw32 mix_buffer[1<<LINUX_SOUND_BUFFER_BITS];
static sw32 output_buffer[1<<LINUX_SOUND_BUFFER_BITS];
linux_sound_class linux_sound;

void linux_voice_class::play()
{
  index = 0;
  active = 1;
}

void linux_voice_class::stop()
{
#error implement
}


void linux_voice_class::set_frequency(i4_frequency freq)
{
  linux_sound_index f(freq);
  
  f.value /= LINUX_SOUND_SAMPLE_SPEED;
  increment = f;
}


void linux_voice_class::set_volume(i4_volume _vol)
{
  volume = _vol;

  left_vol  = (pan<0) ? ((volume < -pan)? 0 : volume + pan) : volume;
  right_vol = (pan<0) ? volume : ((volume < pan) ? 0 : volume - pan);
}


void linux_voice_class::set_pan(i4_pan _pan)
{
  pan = _pan;

  left_vol  = (pan<0) ? ((volume < -pan)? 0 : volume + pan) : volume;
  right_vol = (pan<0) ? volume : ((volume < pan) ? 0 : volume - pan);
}


void linux_sound_class::start_thread()
{
  if (thread_state == LINUX_SOUND_UNINITIALIZED)
  {
    pthread_t handle;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_create(&handle, &attr, linux_sound_mixer, 0);

    thread_state = LINUX_SOUND_RUNNING;
  }
}


void linux_sound_class::stop_thread()
{
  if (thread_state != LINUX_SOUND_UNINITIALIZED)
  {
    if (thread_state == LINUX_SOUND_RUNNING)
      thread_state = LINUX_SOUND_REQUEST_STOP;

    while (thread_state != LINUX_SOUND_STOPPED && thread_state != LINUX_SOUND_UNINITIALIZED)
      sched_yield();

    thread_state = LINUX_SOUND_UNINITIALIZED;
  }
}


void linux_sound_class::initialize_volume_table()
{
  for (int level=0; level<I4_SOUND_VOLUME_LEVELS; level++)
    for (int i=0; i<256; i++)
      volume_table[level][i] = (i-128) * level;
}


extern int i4_global_native_argc;
extern char **i4_global_native_argv;


void linux_sound_class::init()
{
  int i;

  for (i=0; i<i4_global_native_argc; i++)
    if (!strcmp(i4_global_native_argv[i],"-nosound"))
      return ;

#if 0
  fd=open("/dev/mixer",O_WRONLY);
  if (fd!=-1)
  {
    int vol=127;
    ioctl(fd,MIXER_WRITE(SOUND_MIXER_VOLUME),&vol);
    close(fd);
  }
  else
    i4_warning("sound driver : Unable to open /dev/mixer, can't set volume\n");
#endif

  fd=open("/dev/dsp",O_WRONLY,0);
  if (fd<0)
  {
    i4_warning("sound driver : Unable to open /dev/dsp, sound effects disabled\n");
    return;
  }

  // 2 fragments of 2^LINUX_SOUND_BUFFER_BITS bytes
  i = 0x00020000|LINUX_SOUND_BUFFER_BITS;
  ioctl(fd, SNDCTL_DSP_SETFRAGMENT, &i);

  i = 16;     // samples are 16 bit
  if (ioctl(fd, SNDCTL_DSP_SAMPLESIZE, &i)<0)
  {
    i4_warning("SNDDRV : Sample size 16 failed, sound effects disabled\n");    
    close(fd);
    return;
  }

  i = LINUX_SOUND_SAMPLE_SPEED; 
  if (ioctl(fd, SNDCTL_DSP_SPEED, &i)<0)
  {
    i4_warning("SNDDRV : dsp_speed failed, sound effects disabled\n");    
    close(fd);
    return;
  }

  i = 1;     // stero
  if (ioctl(fd, SNDCTL_DSP_STEREO, &i)<0)
  {
    i4_warning("SNDDRV : set stereo failed, sound effects disabled\n");    
    close(fd);
    return;
  }

  thread_state = LINUX_SOUND_UNINITIALIZED;

  initialize_volume_table();

  start_thread();

  i4_sound_manager_class::init();
}


void linux_sound_class::load_sounds(w32 max_sounds)
{
  sound = new linux_sample_class[max_sounds];

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
        linux_sample_class *snd = &sound[count];

        if (info.sample_size==1)
        {
          snd->data = (w8*)info.data;
          snd->sample_rate = linux_sound_index(info.sample_rate);
          snd->size = linux_sound_index(info.size);
        }
        else
        {
          snd->size = 0;
          i4_alert(i4gets("bad_format"),200,&sounds[count]);
        }
      }
      else
        i4_alert(i4gets("bad_format"),200,&sounds[count]);
      delete fp;
    }
  }

  i4_free(sounds);
}

i4_voice_class *linux_sound_class::alloc(i4_sound_id sound_id, const sound_parameters& param)
{
  int i=0; 

  if (!sound[sound_id].data)
    return 0;

  while (i<LINUX_SOUND_NUM_VOICE && voice[i].sound)
    i++;

  if (i<LINUX_SOUND_NUM_VOICE)
  {
    voice[i].looping = param.looping;
    voice[i].index = 0;
    voice[i].active = 0;

    voice[i].set_frequency(param.frequency);
    voice[i].set_volume(param.volume);
    voice[i].set_pan(param.pan);

    voice[i].sound = &sound[sound_id];

    return &voice[i];
  }
  else
    return 0;
}


void *linux_sound_mixer(void *arg)
{
  w16 voc,i;

  while (linux_sound.thread_state == LINUX_SOUND_RUNNING)
  {
    memset(mix_buffer, 0, sizeof(mix_buffer));
    memset(output_buffer, 0, sizeof(output_buffer));
    for (voc=0; voc<LINUX_SOUND_NUM_VOICE; voc++)
    {
      linux_voice_class& v(linux_sound.voice[voc]);

      if (v.sound && v.active)
      {
        for (i=0; i<1<<(LINUX_SOUND_BUFFER_BITS-2); i++)
        {
          mix_buffer[i] += volume_table[v.left_vol][ v.sound->data[ w32(v.index) ] ];
          output_buffer[i] += volume_table[v.right_vol][ v.sound->data[ w32(v.index) ] ];
          v.index += v.increment;

          if (v.index >= v.sound->size)
          {
            if ( !v.looping && (v.complete == 0 || v.complete(&v)) )
            {
              v.sound = 0;
              break;
            }
            else
              while (v.index >= v.sound->size)
                v.index -= v.sound->size;
          }
        }
      }
    }
    for (i=0; i<1<<(LINUX_SOUND_BUFFER_BITS-2); i++)
    {
      sw32 val;

      val = output_buffer[i];
      val = (w16)((val<-32768) ? -32768 : ( (val>32767)? 32767 : val ));
      output_buffer[i] = val;
      val = mix_buffer[i];
      val = (w16)((val<-32768) ? -32768 : ( (val>32767)? 32767 : val ));
      output_buffer[i] |= val<<16;
    }
    write(linux_sound.fd, output_buffer, 1<<LINUX_SOUND_BUFFER_BITS);
    sched_yield();
  }
  linux_sound.thread_state = LINUX_SOUND_STOPPED;


  pthread_exit(0);
}


