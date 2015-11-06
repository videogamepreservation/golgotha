/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "lisp/lisp.hh"

#ifdef _WINDOWS
static li_object *msvc_inspect=0;
static char FP_SAVE[108];

void msvc_lip()
{
  __asm {
    pushfd
    pushad
    fsave FP_SAVE
    frstor FP_SAVE    
  }

  lip(msvc_inspect);

  __asm {
    frstor FP_SAVE    
    popad
    popfd
  }
}
#endif

