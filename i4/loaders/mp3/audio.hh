/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

/* 
 * Audio 'LIB' defines
 */

enum { AUDIO_OUT_HEADPHONES,AUDIO_OUT_INTERNAL_SPEAKER,AUDIO_OUT_LINE_OUT };
enum { DECODE_TEST, DECODE_AUDIO, DECODE_STDOUT, DECODE_BUFFER };

#define AUDIO_FORMAT_SIGNED_16    0x1
#define AUDIO_FORMAT_UNSIGNED_8   0x2
#define AUDIO_FORMAT_SIGNED_8     0x4
#define AUDIO_FORMAT_ULAW_8       0x8
#define AUDIO_FORMAT_ALAW_8       0x10

#if defined(HPUX) || defined(SUNOS) || defined(SOLARIS) || defined(VOXWARE)
#define AUDIO_USES_FD
#endif

struct audio_info_struct
{
#ifdef AUDIO_USES_FD
  int fn; /* filenumber */
#endif
#ifdef SGI
  ALconfig config;
  ALport port;
#endif
  long rate;
  int gain;
  int output;
  char *device;
  int channels;
  int format;
};

extern int audio_play_samples(struct audio_info_struct *,unsigned char *,int);
extern int audio_open(struct audio_info_struct *);
extern int audio_reset_parameters(struct audio_info_struct *);
extern int audio_rate_best_match(struct audio_info_struct *ai);
extern int audio_set_rate(struct audio_info_struct *);
extern int audio_set_format(struct audio_info_struct *);
extern int audio_get_formats(struct audio_info_struct *);
extern int audio_set_channels(struct audio_info_struct *);
extern int audio_write_sample(struct audio_info_struct *,short *,int);
extern int audio_close(struct audio_info_struct *);
extern void audio_info_struct_init(struct audio_info_struct *);
#ifdef SOLARIS
extern void audio_queueflush(struct audio_info_struct *ai);
#endif
