/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "arch.hh"
#include <ddraw.h>
#include <windows.h>
#include "dx5/r1_dx5.hh"

i4_bool ddraw_check(HRESULT res);

typedef struct
{
  LPGUID lpGuid;
  LPSTR  lpDriverName;
} dd_device;

dd_device dd_device_list[32];
sw32      num_dd_devices = 0;

BOOL WINAPI dd_device_callback(LPGUID lpGuid,	LPSTR lpDeviceDescription,
                                   LPSTR lpDriverName, LPVOID lpUserArg)
{
  //quit the enumeration if we've found everything
  for (sw32 i=0;i<num_dd_devices;i++) 
    if (dd_device_list[i].lpGuid==lpGuid) return DDENUMRET_CANCEL;

  dd_device_list[num_dd_devices].lpGuid       = lpGuid;
  dd_device_list[num_dd_devices].lpDriverName = lpDriverName;
  num_dd_devices++;

  if (num_dd_devices==31) return DDENUMRET_CANCEL;

  return DDENUMRET_OK;  
}

BOOL WINAPI dd_create_callback(LPGUID lpGuid,	LPSTR lpDeviceDescription,
                               LPSTR lpDriverName, LPVOID lpUserArg)
{
  char *desired_driver = (char *)lpUserArg;

  if (!stricmp(desired_driver,lpDriverName))
  {
    HRESULT res = DirectDrawCreate(lpGuid,i4_dx5_display->ddraw,0);
    return DDENUMRET_CANCEL;
  }

  return DDENUMRET_OK;  
}

i4_bool r1_dx5_class::init_ddraw(HWND main_hwnd)
{  
  HRESULT res;
  
  res = DirectDrawEnumerate(dd_device_callback,0);
  if (res != DD_OK)
    return i4_F;
  
  for (sw32 i=0;i<num_dd_devices;i++)
  {
    if (!stricmp(dd_device_list[i].lpDriverName,dd_driver_name)) break;
  }
  if (i==num_dd_devices) i=0;

  //create the direct draw object within the callback. GUID's are apparently not reliable outside
  //of it??
  DirectDrawEnumerate(dd_create_callback,(void *)dd_device_list[i].lpDriverName);

  //if it didnt get set, something fucked up
  if (ddraw==0)
    return i4_F;
  
  if (use_page_flip)
    res = ddraw->SetCooperativeLevel(main_hwnd,DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN);
  else
    res = ddraw->SetCooperativeLevel(main_hwnd,DDSCL_NORMAL);

  if (res != DD_OK) return i4_F;
 
  res = ddraw->QueryInterface(IID_IDirectDraw2,(void **)&ddraw2);
  if(res != DD_OK) return i4_F;

  res = ddraw2->SetDisplayMode(640,480,16,0,0);
  ddraw_check(res);
  if (res != DD_OK) return i4_F;

  if (!create_ddraw_surfaces()) return i4_F;

  return i4_T;
}

void r1_dx5_class::uninit_ddraw()
{
  if (back_surface)
    back_surface->Release();
  if (primary_surface)
    primary_surface->Release();
  if (ddraw2)
    ddraw2->Release();
  if (ddraw)
    ddraw->Release();
}

i4_bool r1_dx5_class::create_ddraw_surfaces()
{
  HRESULT res;
  DDSURFACEDESC ddsd;

  if (use_page_flip)
  {
    //if we're page flipping, its a whole different ball game as far as creating the primary and secondary surfaces go
    ZeroMemory(&ddsd, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE |
                          DDSCAPS_FLIP |
                          DDSCAPS_COMPLEX |
                          DDSCAPS_3DDEVICE |
                          DDSCAPS_VIDEOMEMORY;
  
    ddsd.dwBackBufferCount = 1;

    //get the primary surface
    res = ddraw->CreateSurface(&ddsd, &primary_surface, NULL);
    if (!ddraw_check(res))
      return i4_F;

    // get the back buffer surface
    DDSCAPS caps;
    caps.dwCaps = DDSCAPS_BACKBUFFER;
    res = primary_surface->GetAttachedSurface(&caps, &back_surface);  
    if (!ddraw_check(res))
      return i4_F;      
  }
  else
  {  
  
    //this will be non-page flipped (aka blitted)

    // Create the primary surface   
    memset(&ddsd,0,sizeof(DDSURFACEDESC));  
    ddsd.dwSize  = sizeof(ddsd);
    ddsd.dwFlags = DDSD_CAPS; 
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;


    res = ddraw2->CreateSurface(&ddsd,&primary_surface,0);
    if (res != DD_OK) return i4_F;  
    
    primary_surface->GetSurfaceDesc(&ddsd);

    ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
    ddsd.dwWidth = 640;
    ddsd.dwHeight = 480;
    ddsd.ddsCaps.dwCaps = DDSCAPS_3DDEVICE;
  
    res = ddraw2->CreateSurface(&ddsd,&back_surface,0);
    if (res != DD_OK) return i4_F;      
  }
  
  memset(&ddsd,0,sizeof(DDSURFACEDESC));
  ddsd.dwSize = sizeof(DDSURFACEDESC);
  primary_surface->GetSurfaceDesc(&ddsd);

  i4_pixel_format fmt;

  fmt.pixel_depth = I4_16BIT;  
  fmt.red_mask    = ddsd.ddpfPixelFormat.dwRBitMask;
  fmt.green_mask  = ddsd.ddpfPixelFormat.dwGBitMask;
  fmt.blue_mask   = ddsd.ddpfPixelFormat.dwBBitMask;
  fmt.calc_shift();

  screen_palette = i4_pal_man.register_pal(&fmt);

  return i4_T;
}