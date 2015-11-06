/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef __CRKUTIL_HH
#define __CRKUTIL_HH

#include <Max.h>
#include "crkutilr.hh"

TCHAR *GetString(int id);
void initialize_i4();

enum { MAX_MATERIAL_OFFSET = 1024 };

extern char *gmod_sig;

extern ClassDesc* GetCrackUtilDesc();
extern ClassDesc* GetCrackImportDesc();
// extern ClassDesc* GetGMODMatDesc();

#endif
