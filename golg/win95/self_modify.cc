/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/


#include <windows.h>
#include <process.h>
#include "error/error.hh"

int allow_self_modification()
{
  int                           ReturnValue = 0;

  HMODULE                       OurModule = GetModuleHandle(0);
  BYTE                         *pBaseOfImage = 0;

  if ( (GetVersion() & 0xC0000000) == 0x80000000)
  {
    // We're on Win32s, so get the real pointer
    HMODULE                       Win32sKernel = GetModuleHandle("W32SKRNL.DLL");

    typedef DWORD __stdcall       translator(DWORD);
    translator                   *pImteFromHModule =
      (translator *) GetProcAddress(Win32sKernel, "_ImteFromHModule@4");
    translator                   *pBaseAddrFromImte =
      (translator *) GetProcAddress(Win32sKernel, "_BaseAddrFromImte@4");

    if (pImteFromHModule && pBaseAddrFromImte)
    {
      DWORD                         Imte = (*pImteFromHModule) ( (DWORD) OurModule);

      pBaseOfImage = (BYTE *) (*pBaseAddrFromImte) (Imte);
    }
  }
  else
  {
    pBaseOfImage = (BYTE *) OurModule;
  }

  if (pBaseOfImage)
  {
    IMAGE_OPTIONAL_HEADER        *pHeader = (IMAGE_OPTIONAL_HEADER *)
      (pBaseOfImage + ( (IMAGE_DOS_HEADER *) pBaseOfImage)->e_lfanew +
       sizeof (IMAGE_NT_SIGNATURE) + sizeof (IMAGE_FILE_HEADER));

    DWORD                         OldRights;

    if (VirtualProtect(pBaseOfImage + pHeader->BaseOfCode, pHeader->SizeOfCode,
                       PAGE_READWRITE, &OldRights))
    {
      ReturnValue = 1;
    }
  }

  return ReturnValue;
}
