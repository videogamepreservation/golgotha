/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include <ctype.h>
#include <stdlib.h>
#include <signal.h>
#include "loaders/mp3/mpg123.hh"
#include "loaders/mp3/tables.hh"
#include "file/file.hh"

/* max = 1728 */
#define MAXFRAMESIZE 1792

#define SKIP_JUNK 1

int tabsel_123[2][3][16] = {
  { {0,32,64,96,128,160,192,224,256,288,320,352,384,416,448,},
    {0,32,48,56, 64, 80, 96,112,128,160,192,224,256,320,384,},
    {0,32,40,48, 56, 64, 80, 96,112,128,160,192,224,256,320,} },

  { {0,32,48,56,64,80,96,112,128,144,160,176,192,224,256,},
    {0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,},
    {0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,} }
};

long freqs[7] = { 44100, 48000, 32000, 22050, 24000, 16000 , 11025 };

static int bitindex;
static unsigned char *wordpointer;

static int fsize=0,fsizeold=0,ssize;
static unsigned char bsspace[2][MAXFRAMESIZE+512]; /* MAXFRAMESIZE */
static unsigned char *bsbuf=bsspace[1],*bsbufold;
static int bsnum=0;

extern i4_file_class *mp3_out, *mp3_in;

struct ibuf {
  struct ibuf *next;
  struct ibuf *prev;
  unsigned char *buf;
  unsigned char *pnt;
  int len;
  /* skip,time stamp */
};

struct ibuf ibufs[2];
struct ibuf *cibuf;
int ibufnum=0;

unsigned char *pcm_sample;
int pcm_point = 0;
int audiobufsize = AUDIOBUFSIZE;

#ifdef VARMODESUPPORT
/*
	 *   This is a dirty hack!  It might burn your PC and kill your cat!
	 *   When in "loaders/mp3/varmode", specially formatted layer-3 mpeg files arhe
	 *   expected as input -- it will NOT work with standard mpeg files.
	 *   The reason for this:
	 *   Varmode mpeg files enable my own GUI player to perform fast
	 *   forward and backward functions, and to jump to an arbitrary
	 *   timestamp position within the file.  This would be possible
	 *   with standard mpeg files, too, but it would be a lot harder to
	 *   implement.
	 *   A filter for converting standard mpeg to varmode mpeg is
	 *   available on request, but it's really not useful on its own.
	 *
	 *   Oliver Fromme  <oliver.fromme@heim3.tu-clausthal.de>
	 *   Mon Mar 24 00:04:24 MET 1997
	 */
int varmode = FALSE;
int playlimit;
#endif


static unsigned long oldhead = 0;
static unsigned long firsthead=0;

static int halfphase = 0;

static char *lastdir = NULL;

void init_common(void)
{
#ifdef VARMODESUPPORT
  varmode = FALSE;
#endif
  pcm_point = 0;
  audiobufsize = AUDIOBUFSIZE;
  ibufnum=0;
  fsize=0;
  fsizeold=0;
  bsbuf=bsspace[1];
  bsnum=0;
  oldhead = 0;
  firsthead = 0;
  halfphase = 0;
  lastdir = NULL;
}

static void get_II_stuff(struct frame *fr)
{
  static int translate[3][2][16] = 
  { { { 0,2,2,2,2,2,2,0,0,0,1,1,1,1,1,0 } ,
      { 0,2,2,0,0,0,1,1,1,1,1,1,1,1,1,0 } } ,
    { { 0,2,2,2,2,2,2,0,0,0,0,0,0,0,0,0 } ,
      { 0,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0 } } ,
    { { 0,3,3,3,3,3,3,0,0,0,1,1,1,1,1,0 } ,
      { 0,3,3,0,0,0,1,1,1,1,1,1,1,1,1,0 } } };

  int table,sblim;
  static struct al_table *tables[5] = 
  { alloc_0, alloc_1, alloc_2, alloc_3 , alloc_4 };
  static int sblims[5] = { 27 , 30 , 8, 12 , 30 };

  if(fr->lsf)
    table = 4;
  else
    table = translate[fr->sampling_frequency][2-fr->stereo][fr->bitrate_index];
  sblim = sblims[table];

  fr->alloc = tables[table];
  fr->II_sblimit = sblim;
}

void audio_flush(int outmode, struct audio_info_struct *ai)
{
  if (pcm_point) 
  {
    mp3_out->write(pcm_sample, pcm_point);
    pcm_point = 0;
  }
}

void read_frame_init (void)
{
  oldhead = 0;
  firsthead = 0;
}

#define HDRCMPMASK 0xfffffd00
#if 0
#define HDRCMPMASK 0xfffffdft
#endif

/* 
 * HACK,HACK,HACK 
 * step back <num> frames 
 */
int back_frame(struct frame *fr,int num)
{
  long bytes;
  unsigned char buf[4];
  unsigned long newhead;

  if(!firsthead)
    return 0;

  bytes = (fsize+8)*(num+2);

  if(mp3_in->seek(mp3_in->tell()-bytes)<0)
    return -1;

  if(mp3_in->read(buf,4)!=4)
    return -1;

  newhead = (buf[0]<<24) + (buf[1]<<16) + (buf[2]<<8) + buf[3];
	
  while( (newhead & HDRCMPMASK) != (firsthead & HDRCMPMASK) ) {
    if(mp3_in->read(buf,1)!=1)
      return -1;
    newhead <<= 8;
    newhead |= buf[0];
    newhead &= 0xffffffff;
  }

  if (mp3_in->seek(mp3_in->tell()-4)<0)
    return -1;
	
  read_frame(fr);
  read_frame(fr);

  if(fr->lay == 3) {
    set_pointer(512);
  }

  return 0;
}

int head_read(unsigned char *hbuf,unsigned long *newhead)
{  
  if (mp3_in->read(hbuf, 4)!=4)
    return FALSE;

  *newhead = ((unsigned long) hbuf[0] << 24) |
    ((unsigned long) hbuf[1] << 16) |
    ((unsigned long) hbuf[2] << 8)  |
    (unsigned long) hbuf[3];

  return TRUE;
}

int head_check(unsigned long newhead) 
{
  if( (newhead & 0xffe00000) != 0xffe00000)
    return FALSE;
  if(!((newhead>>17)&3))
    return FALSE;
  if( ((newhead>>12)&0xf) == 0xf)
    return FALSE;
  if( ((newhead>>10)&0x3) == 0x3 )
    return FALSE;
  return TRUE;
}


int read_frame(struct frame *fr)
{
  static unsigned long newhead;

  static unsigned char ssave[34];
  unsigned char hbuf[8];
  static int framesize;
  int l;
  int try_c = 0;

  if (halfspeed)
    if (halfphase--) {
      bitindex = 0;
      wordpointer = (unsigned char *) bsbuf;
      if (fr->lay == 3)
        memcpy (bsbuf, ssave, ssize);
      return 1;
    }
    else
      halfphase = halfspeed - 1;

#ifdef VARMODESUPPORT
  if (varmode) {
    if (mp3_in->read(hbuf,8)!=8)
      return 0;
  }
  else
#endif

    read_again:
  if(!head_read(hbuf,&newhead))
    return FALSE;

  if(oldhead != newhead || !oldhead)
  {
    fr->header_change = 1;

  init_resync:

#ifdef SKIP_JUNK
    if(!firsthead && !head_check(newhead) ) {
      int i;

      /* I even saw RIFF headers at the beginning of MPEG streams ;( */

      if(newhead == ('R'<<24)+('I'<<16)+('F'<<8)+'F') {
        char buf[40];
        mp3_in->read(buf,68);
        goto read_again;
      }
      /* search in 32 bit steps through the first 2K */
      for(i=0;i<512;i++) {
        if(!head_read(hbuf,&newhead))
          return 0;
        if(head_check(newhead))
          break;
      }
      if(i==512) {
        /* step in byte steps through next 2K */
        for(i=0;i<2048;i++) {
          memmove (&hbuf[0], &hbuf[1], 3);
          if(mp3_in->read(hbuf+3,1) != 1)
            return 0;
          newhead <<= 8;
          newhead |= hbuf[3];
          newhead &= 0xffffffff;
          if(head_check(newhead))
            break;
        }
        if(i == 2048) {
          return 0;
        }
      }
      /* 
       * should we check, whether a new frame starts at the next
       * expected position? (some kind of read ahead)
       * We could implement this easily, at least for files.
       */
    }
#endif

    if( (newhead & 0xffe00000) != 0xffe00000) {
      if (tryresync) {
        /* Read more bytes until we find something that looks
           reasonably like a valid header.  This is not a
           perfect strategy, but it should get us back on the
           track within a short time (and hopefully without
           too much distortion in the audio output).  */
        do {
          try_c++;
          memmove (&hbuf[0], &hbuf[1], 7);
#ifdef VARMODESUPPORT
          if (mp3_in->read(&hbuf[varmode?7:3],1) != 1)
#else
            if (mp3_in->read(&hbuf[3],1) != 1)
#endif
              return 0;

          /* This is faster than combining newhead from scratch */
          newhead = ((newhead << 8) | hbuf[3]) & 0xffffffff;

          if (!oldhead)
            goto init_resync;       /* "considered harmful", eh? */

        } while ((newhead & HDRCMPMASK) != (oldhead & HDRCMPMASK)
                 && (newhead & HDRCMPMASK) != (firsthead & HDRCMPMASK));
      }
      else
        return (0);
    }
    if (!firsthead)
      firsthead = newhead;

    if( newhead & (1<<20) ) {
      fr->lsf = (newhead & (1<<19)) ? 0x0 : 0x1;
      fr->mpeg25 = 0;
    }
    else {
      fr->lsf = 1;
      fr->mpeg25 = 1;
    }
    
    if (!tryresync || !oldhead) {
      /* If "tryresync" is true, assume that certain
         parameters do not change within the stream! */
      fr->lay = 4-((newhead>>17)&3);
      fr->bitrate_index = ((newhead>>12)&0xf);
      if( ((newhead>>10)&0x3) == 0x3) {
        exit(1);
      }
      if(fr->mpeg25)
        fr->sampling_frequency = 6;
      else
        fr->sampling_frequency = ((newhead>>10)&0x3) + (fr->lsf*3);
      fr->error_protection = ((newhead>>16)&0x1)^0x1;
    }

    fr->padding   = ((newhead>>9)&0x1);
    fr->extension = ((newhead>>8)&0x1);
    fr->mode      = ((newhead>>6)&0x3);
    fr->mode_ext  = ((newhead>>4)&0x3);
    fr->copyright = ((newhead>>3)&0x1);
    fr->original  = ((newhead>>2)&0x1);
    fr->emphasis  = newhead & 0x3;

    fr->stereo    = (fr->mode == MPG_MD_MONO) ? 1 : 2;

    oldhead = newhead;

    if(!fr->bitrate_index)
    {
      return (0);
    }

    switch(fr->lay)
    {
      case 1:
        fr->do_layer = do_layer1;
#ifdef VARMODESUPPORT
        if (varmode) {
          return (0);
        }
#endif
        fr->jsbound = (fr->mode == MPG_MD_JOINT_STEREO) ? 
          (fr->mode_ext<<2)+4 : 32;
        framesize  = (long) tabsel_123[fr->lsf][0][fr->bitrate_index] * 12000;
        framesize /= freqs[fr->sampling_frequency];
        framesize  = ((framesize+fr->padding)<<2)-4;
        break;
      case 2:
        fr->do_layer = do_layer2;
#ifdef VARMODESUPPORT
        if (varmode) {
          return (0);
        }
#endif
        get_II_stuff(fr);
        fr->jsbound = (fr->mode == MPG_MD_JOINT_STEREO) ?
          (fr->mode_ext<<2)+4 : fr->II_sblimit;
        framesize = (long) tabsel_123[fr->lsf][1][fr->bitrate_index] * 144000;
        framesize /= freqs[fr->sampling_frequency];
        framesize += fr->padding - 4;
        break;
      case 3:
        fr->do_layer = do_layer3;
        if(fr->lsf)
          ssize = (fr->stereo == 1) ? 9 : 17;
        else
          ssize = (fr->stereo == 1) ? 17 : 32;
        if(fr->error_protection)
          ssize += 2;
#ifdef VARMODESUPPORT
        if (varmode)
          playlimit = ((unsigned int) hbuf[6] << 8) | (unsigned int) hbuf[7];
        framesize = ssize + 
          (((unsigned int) hbuf[4] << 8) | (unsigned int) hbuf[5]);
        else {
#endif
          framesize  = (long) tabsel_123[fr->lsf][2][fr->bitrate_index] * 144000;
          framesize /= freqs[fr->sampling_frequency]<<(fr->lsf);
          framesize = framesize + fr->padding - 4;
#ifdef VARMODESUPPORT
        }
#endif
        break; 
      default:
        return (0);
    }
  }
  else
    fr->header_change = 0;

  fsizeold=fsize;	/* for Layer3 */
  bsbufold = bsbuf;	
  bsbuf = bsspace[bsnum]+512;
  bsnum = (bsnum + 1) & 1;

  fsize = framesize;
 
  if( (l=mp3_in->read(bsbuf,fsize)) != fsize)
  {
    if(l <= 0)
      return 0;
    memset(bsbuf+l,0,fsize-l);
  }

  if (halfspeed && fr->lay == 3)
    memcpy (ssave, bsbuf, ssize);

  bitindex = 0;
  wordpointer = (unsigned char *) bsbuf;

  return 1;
}

/*
 *   Allocate space for a new string containing the first
 *   "num" characters of "src".  The resulting string is
 *   always zero-terminated.  Returns NULL if malloc fails.
 */

#if 1
char *strndup (const char *src, int num)
{
  char *dst;

  if (!(dst = (char *) malloc(num+1)))
    return (NULL);
  dst[num] = '\0';
  return (strncpy(dst, src, num));
}
#endif

/*
 *   Split "path" into directory and filename components.
 *
 *   Return value is 0 if no directory was specified (i.e.
 *   "path" does not contain a '/'), OR if the directory
 *   is the same as on the previous call to this function.
 *
 *   Return value is 1 if a directory was specified AND it
 *   is different from the previous one (if any).
 */

int split_dir_file (const char *path, char **dname, char **fname)
{
  char *slashpos;

  if ((slashpos = strrchr(path, '/'))) {
    *fname = slashpos + 1;
    *dname = strndup(path, 1 + slashpos - path);
    if (lastdir && !strcmp(lastdir, *dname)) {
      /***   same as previous directory   ***/
      free (*dname);
      *dname = lastdir;
      return 0;
    }
    else {
      /***   different directory   ***/
      if (lastdir)
        free (lastdir);
      lastdir = *dname;
      return 1;
    }
  }
  else {
    /***   no directory specified   ***/
    if (lastdir) {
      free (lastdir);
      lastdir = NULL;
    };
    *dname = NULL;
    *fname = (char *)path;
    return 0;
  }
}

unsigned int getbits(int number_of_bits)
{
  unsigned long rval;

  if(!number_of_bits)
    return 0;

  {
    rval = wordpointer[0];
    rval <<= 8;
    rval |= wordpointer[1];
    rval <<= 8;
    rval |= wordpointer[2];
    rval <<= bitindex;
    rval &= 0xffffff;

    bitindex += number_of_bits;

    rval >>= (24-number_of_bits);

    wordpointer += (bitindex>>3);
    bitindex &= 7;
  }

  return rval;
}

unsigned int getbits_fast(int number_of_bits)
{
  unsigned long rval;

  {
    rval = wordpointer[0];
    rval <<= 8;	
    rval |= wordpointer[1];
    rval <<= bitindex;
    rval &= 0xffff;
    bitindex += number_of_bits;

    rval >>= (16-number_of_bits);

    wordpointer += (bitindex>>3);
    bitindex &= 7;
  }

  return rval;
}

unsigned int get1bit(void)
{
  unsigned char rval;

  rval = *wordpointer << bitindex;

  bitindex++;
  wordpointer += (bitindex>>3);
  bitindex &= 7;

  return rval>>7;
}

void set_pointer(long backstep)
{
  wordpointer = bsbuf + ssize - backstep;
  if (backstep)
    memcpy(wordpointer,bsbufold+fsizeold-backstep,backstep);
  bitindex = 0; 
}
