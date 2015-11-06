/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "crkutil.hh"
#include "memory/malloc.hh"
#include "string/string.hh"
#include "debug.hh"

char *gmod_sig = "GMOD";

HINSTANCE hInstance;
extern void * my_instance;

int controlsInit = FALSE;

/** public functions **/
BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved)
{
  my_instance = hInstance = hinstDLL;

  if (!controlsInit) 
  {
    controlsInit = TRUE;
		
    // jaguar controls
    InitCustomControls(hInstance);
		
    // initialize Chicago controls
    InitCommonControls();
  }
			
  return (TRUE);
}


//------------------------------------------------------
// This is the interface to Jaguar:
//------------------------------------------------------

__declspec( dllexport ) const TCHAR * LibDescription() 
{ 
  return _T("Crack Utilities"); 
}

/// MUST CHANGE THIS NUMBER WHEN ADD NEW CLASS
__declspec( dllexport ) int LibNumberClasses() 
{
  return 3;
}

__declspec( dllexport ) ClassDesc* LibClassDesc(int i) 
{
  switch(i) 
  {
    case 0: return GetCrackUtilDesc();
    case 1: return GetCrackImportDesc();
//     case 2: return GetGMODMatDesc();
    default: return 0;
  }
}

// Return version so can detect obsolete DLLs
__declspec( dllexport ) ULONG LibVersion() 
{
  return VERSION_3DSMAX; 
}

TCHAR *GetString(int id)
{
  static TCHAR buf[256];

  if (hInstance)
    return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;

  return NULL;
}

int i4_crkutil_error(const char *st)
{
  dbg("Error: %s\n",st);
  return 0;
}


int i4_crkutil_warning(const char *st)
{
  dbg("Warning: %s\n",st);
  return 0;
}

void initialize_i4()
{
  if (!i4_is_initialized())
  {
    i4_set_error_function(i4_crkutil_error);
    i4_set_error_function(i4_crkutil_warning);
    i4_set_max_memory_used(4096000);
    i4_init();
    i4_string_man.load("resource.res"); 
  }
}
