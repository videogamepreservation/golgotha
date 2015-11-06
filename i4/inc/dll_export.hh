/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef I4_DLL_EXPORT_HH
#define I4_DLL_EXPORT_HH


#ifdef _WINDOWS
#ifdef BUILDING_DLL
#define I4_DLL_EXPORT __declspec( dllimport )
#else
#define I4_DLL_EXPORT __declspec( dllexport )
#endif

#else
#define I4_DLL_EXPORT
#endif


#endif
