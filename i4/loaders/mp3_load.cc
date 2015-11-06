/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/


#include "file/file.hh"
#include "loaders/wav_load.hh"
#include "error/error.hh"
#include "status/status.hh"

i4_file_class *mp3_in=0;
i4_file_class *mp3_out=0;



/* 
 * Mpeg Layer audio decoder (see version.h for version number)
 * ------------------------
 * copyright (c) 1995,1996,1997 by Michael Hipp, All rights reserved.
 * See also 'README' !
 *
 * Moved all staticly initialized variables to top and added init_variables()
 * so I can restart this monster.
 *
 */

#include <stdlib.h>
#include <sys/types.h>

/* #define SET_PRIO */

#include "loaders/mp3/mpg123.hh"
#include "loaders/mp3/getlopt.hh"
#include "loaders/mp3/version.hh"

static long rates[3][3] = { 
  { 32000,44100,48000 } ,
  { 16000,22050,24000 } ,
  {  8000,11025,12000 } 
};

struct flags flags = { 0 , 0 };

int supported_rates = 0;

int outmode = DECODE_AUDIO;

char *listname = NULL;
long outscale  = 32768;
int checkrange = FALSE;
int tryresync  = TRUE;
int quiet      = FALSE;
int verbose    = 0;
int doublespeed= 0;
int halfspeed  = 0;
int change_always = 1;
int force_8bit = 0;
int force_frequency = -1;
int force_mono = 0;
long numframes = -1;
long startFrame= 0;
int usebuffer  = 0;
int buffer_fd[2];
int buffer_pid;

static struct frame fr;
static struct audio_info_struct ai;
#define FRAMEBUFUNIT (18 * 64 * 4)

static int init_output_done = FALSE;

static FILE *listfile = NULL;

static void *synth_funcs[2][2][3][2] = { 
  { { { synth_1to1 , synth_1to1_mono2stereo } ,
      { synth_2to1 , synth_2to1_mono2stereo } ,
      { synth_4to1 , synth_4to1_mono2stereo } } ,
    { { synth_1to1_8bit , synth_1to1_8bit_mono2stereo } ,
      { synth_2to1_8bit , synth_2to1_8bit_mono2stereo } ,
      { synth_4to1_8bit , synth_4to1_8bit_mono2stereo } } } ,
  { { { synth_1to1_mono , synth_1to1_mono } ,
      { synth_2to1_mono , synth_2to1_mono } ,
      { synth_4to1_mono , synth_4to1_mono } } ,
    { { synth_1to1_8bit_mono , synth_1to1_8bit_mono } ,
      { synth_2to1_8bit_mono , synth_2to1_8bit_mono } ,
      { synth_4to1_8bit_mono , synth_4to1_8bit_mono } } } 
};

void init_mpg123(void)
{

  rates[0][0] = 32000;
  rates[0][1] = 44100;
  rates[0][2] = 48000;
  rates[1][0] = 16000;
  rates[1][1] = 22050;
  rates[1][2] = 24000;
  rates[2][0] = 8000;
  rates[2][1] = 11025;
  rates[2][2] = 12000;

  flags.equalizer = 0;
  flags.aggressive = 0;

  supported_rates = 0;

  outmode = DECODE_AUDIO;

  listname = NULL;
  outscale  = 32768;
  checkrange = FALSE;
  tryresync  = TRUE;
  quiet      = FALSE;
  verbose    = 0;
  doublespeed= 0;
  halfspeed  = 0;
  change_always = 1;
  force_8bit = 0;
  /*	force_frequency = -1; */
  force_mono = 1;
  numframes = -1;
  startFrame= 0;
  usebuffer  = 0;

  init_output_done = FALSE;

  listfile = NULL;

}

void audio_info_struct_init(struct audio_info_struct *ai)
{
  ai->rate = -1;
  ai->gain = -1;
  ai->output = -1;
  ai->device = NULL;
  ai->channels = -1;
  ai->format = -1;
}

void print_rheader(struct frame *fr);

void init_output(void)
{

  if (init_output_done)
    return;
  init_output_done = TRUE;
  if (!(pcm_sample = (unsigned char *) malloc(audiobufsize * 2))) {
    perror ("malloc()");
    exit (1);
  }

}

char *get_next_file (int argc, char *argv[])
{
  static char line[1024];

  if (listname || listfile) {
    if (!listfile) {
      if (!*listname || !strcmp(listname, "-")) {
        listfile = stdin;
        listname = NULL;
      }
      else if (!(listfile = fopen(listname, "rb"))) {
        perror (listname);
        exit (1);
      }
    }
    do {
      if (fgets(line, 1023, listfile)) {
        line[strcspn(line, "\t\n\r")] = '\0';
        if (line[0]=='\0' || line[0]=='#')
          continue;
        return (line);
      }
      else {
        if (*listname)
          fclose (listfile);
        listname = NULL;
        listfile = NULL;
      }
    } while (listfile);
  }

  if (loptind < argc)
    return (argv[loptind++]);
  return (NULL);
}

void set_synth (char *arg)
{
  if (*arg == '2') {
    fr.down_sample = 1;
  }
  else {
    fr.down_sample = 2;
  }
}

#ifdef VARMODESUPPORT
void set_varmode (char *arg)
{
  audiobufsize = ((audiobufsize >> 1) + 63) & 0xffffc0;
}
#endif

void set_verbose (char *arg)
{
  verbose++;
}

topt opts[] = {
  {'k', "skip",        GLO_ARG | GLO_NUM,  0, &startFrame, 0},
  {'a', "audiodevice", GLO_ARG | GLO_CHAR, 0, &ai.device,  0},
  {'2', "2to1",        0,          set_synth, 0,           0},
  {'4', "4to1",        0,          set_synth, 0,           0},
  {'t', "test",        0,                  0, &outmode, DECODE_TEST},
  {'s', "stdout",      0,                  0, &outmode, DECODE_STDOUT},
  {'c', "check",       0,                  0, &checkrange, TRUE},
  {'v', "verbose",     0,        set_verbose, 0,           0},
  {'q', "quiet",       0,                  0, &quiet,      TRUE},
  {'y', "resync",      0,                  0, &tryresync,  FALSE},
  {'0', "single0",     0,                  0, &fr.single,  0},
  {0,   "left",        0,                  0, &fr.single,  0},
  {'1', "single1",     0,                  0, &fr.single,  1},
  {0,   "right",       0,                  0, &fr.single,  1},
  {'m', "singlemix",   0,                  0, &fr.single,  3},
  {0,   "mix",         0,                  0, &fr.single,  3},
  {'g', "gain",        GLO_ARG | GLO_NUM,  0, &ai.gain,    0},
  {'r', "rate",        GLO_ARG | GLO_NUM,  0, &force_frequency,  0},
  {0,   "8bit",        0,                  0, &force_8bit, 1},
  {'f', "scale",       GLO_ARG | GLO_NUM,  0, &outscale,   0},
  {'n', "frames",      GLO_ARG | GLO_NUM,  0, &numframes,  0},
#ifdef VARMODESUPPORT
  {'v', "var",         0,        set_varmode, &varmode,    TRUE},
#endif
  {'b', "buffer",      GLO_ARG | GLO_NUM,  0, &usebuffer,  0},
  {'d', "doublespeed", GLO_ARG | GLO_NUM,  0, &doublespeed,0},
  {'h', "halfspeed",   GLO_ARG | GLO_NUM,  0, &halfspeed,  0},
  {'@', "list",        GLO_ARG | GLO_CHAR, 0, &listname,   0},
  {0,   "equalizer",	0,				0, &flags.equalizer,1},
  {0,   "aggressive",	0,				0, &flags.aggressive,2},
  {0, 0, 0, 0, 0, 0}
};

/*
 *   Change the playback sample rate.
 */
void reset_audio_samplerate(void)
{
}

/*
 * play a frame read read_frame();
 * (re)initialize audio if necessary.
 */
void play_frame(int init,struct frame *fr)
{
  int clip;

  if((fr->header_change && change_always) || init) {
    int reset_audio = 0;

    if(force_frequency < 0) {
      if(ai.rate != freqs[fr->sampling_frequency]>>(fr->down_sample)) {
        ai.rate = freqs[fr->sampling_frequency]>>(fr->down_sample);
        reset_audio = 1;
      }
    }
    else if(ai.rate != force_frequency) {
      ai.rate = force_frequency;
      reset_audio = 1;
    }
    init_output();
    if(reset_audio) {
      reset_audio_samplerate();
    }
  }

  if (fr->error_protection) {
    getbits(16); /* crc */
  }

  clip = (fr->do_layer)(fr,outmode,&ai);

  /*
    if(clip > 0 && checkrange)
    fprintf(stderr,"%d samples clipped\n", clip);
    */
}

void set_synth_functions(struct frame *fr)
{

  *((void **)(&fr->synth)) = synth_funcs[force_mono][force_8bit][fr->down_sample][0];
  *((void **)(&fr->synth_mono)) = synth_funcs[force_mono][force_8bit][fr->down_sample][1];
  fr->block_size = 128 >> (force_mono+force_8bit+fr->down_sample);

  if(force_8bit) {
    ai.format = AUDIO_FORMAT_ULAW_8;
    make_conv16to8_table(ai.format);
  }
}


i4_bool i4_load_mp3(i4_file_class *in,  // open file at beginging of mp3 data (after header)
                    i4_file_class *out,    // open file ready to write raw data
                    i4_sound_info &actual_output,           // returned
                    i4_status_class *status)   // tells user what's going on
{                      
  int result;
  long frameNum = 0;
  char *fname;
  int init;

  extern void init_mpg123(void);
  extern void init_common(void);
  extern void init_decode(void);
  extern void init_decode_2to1(void);
  extern void init_decode_4to1(void);
  extern void init_getlopt(void);

  init_mpg123();
  init_common();
  init_decode();
  init_decode_2to1();
  init_decode_4to1();
  /*		init_decode_i386(); */
  init_getlopt();

  fr.synth = synth_1to1;
  fr.down_sample = 0;
  fr.single=0;
  force_mono=1;
  force_frequency=-1;

  

  ai.format = AUDIO_FORMAT_SIGNED_16;
  ai.gain = ai.rate = ai.output = -1;
  ai.device = NULL;
  ai.channels = 1;

  prgName = "mp3_load";
  
  mp3_in=in;
  mp3_out=out;


  int start=mp3_out->tell();

  actual_output.channels=1;
  actual_output.sample_size=2;


  /*
  if (suggested_output.sample_rate<=11*1024)
  {
    fr.down_sample = 2;
  } else if (suggested_output.sample_rate<=22*1024)
    fr.down_sample = 1;
  else
    fr.down_sample = 0;
    */





  {
    int fmts;
    int i,j;

    struct audio_info_struct ai;

    audio_info_struct_init(&ai);
    fmts = AUDIO_FORMAT_SIGNED_16;

    supported_rates = 0;
    for(i=0;i<3;i++) {
      for(j=0;j<3;j++) {
        ai.rate = rates[i][j];
        /* allow about 2% difference */
        if( ((rates[i][j]*98) < (ai.rate*100)) &&
           ((rates[i][j]*102) > (ai.rate*100)) )
          supported_rates |= 1<<(i*3+j);
      } 
    }

    if(!force_8bit && !(fmts & AUDIO_FORMAT_SIGNED_16))
      force_8bit = 1;

    if(force_8bit && !(fmts & AUDIO_FORMAT_ULAW_8)) {
      exit(1);
    }
  }


  set_synth_functions(&fr);

  make_decode_tables(outscale);
  init_layer2(); /* inits also shared tables with layer1 */
  init_layer3(fr.down_sample);

      
  read_frame_init();


  init = 1;
  int mp3_end=in->size();
  int mp3_start=in->tell();


  for(frameNum=0;
      in->tell()!=mp3_end &&
        read_frame(&fr) && 
        numframes;
      frameNum++) 
  {
    if (status)
      status->update((in->tell()+2)/(float)(mp3_end+1));

    if(frameNum < startFrame || (doublespeed && (frameNum % doublespeed))) 
    {
      if(fr.lay == 3)
        set_pointer(512);
      continue;
    }

    numframes--;
    play_frame(init,&fr);
    actual_output.sample_rate=ai.rate;

    init = 0;

  }


  audio_flush(outmode, &ai);

  actual_output.size=mp3_out->tell()-start;

  free (pcm_sample);
  return i4_T;

}


