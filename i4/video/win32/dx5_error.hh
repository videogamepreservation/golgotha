/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef DX5_ERROR_HH
#define DX5_ERROR_HH

#include <windows.h>
#include <ddraw.h>


#ifdef DEBUG

i4_bool i4_dx5_check(HRESULT res);

#else

#define i4_dx5_check(a) ((a)==(DD_OK))

#endif




#endif
