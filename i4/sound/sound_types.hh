/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef SOUND_TYPES_HH
#define SOUND_TYPES_HH

#include "arch.hh"

#ifdef I4_RETAIL
#define I4_SOUND_NAME(x)
#else
#define I4_SOUND_NAME(x) virtual char *name() { return x; }
#endif

typedef sw32 i4_frequency;
typedef sw32 i4_volume;
typedef sw32 i4_pan;

enum { I4_SOUND_VOLUME_LEVELS = 64 };
enum { I4_SOUND_MAX_PAN=10000 };

#endif

//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}
