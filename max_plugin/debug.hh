/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef M1_DEBUG_HH
#define M1_DEBUG_HH

#include <stdio.h>

class m1_debugfile
{
public:
  char name[256];
  FILE *f;

  m1_debugfile(char *name = 0);
  ~m1_debugfile();

  void restart();
  void printf(char *format, ...);
  void operator()(char *format,...);
};

extern m1_debugfile dbg;

#endif

//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}
