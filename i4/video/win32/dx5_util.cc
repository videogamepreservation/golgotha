/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "video/win32/dx5_util.hh"
#include "video/win32/dx5_error.hh"
#include "threads/threads.hh"
#include "error/error.hh"

#define DEPTH 16


dx5_common_class       dx5_common;
IDirectDraw2          *dx5_common_class::ddraw;
IDirectDrawSurface3   *dx5_common_class::primary_surface, *dx5_common_class::back_surface,
                      *dx5_common_class::front_surface;
DDPIXELFORMAT          dx5_common_class::dd_fmt_565, dx5_common_class::dd_fmt_1555;
i4_pixel_format        dx5_common_class::i4_fmt_565, dx5_common_class::i4_fmt_1555;



IDirectDrawSurface3 *dx5_common_class::create_surface(dx5_surface_type type,
                                                      int width, int height,
                                                      int flags,
                                                      DDPIXELFORMAT *format)
{        
  HRESULT result;
  DDSURFACEDESC ddsd;
  IDirectDrawSurface *surface;
  IDirectDrawSurface3 *surface3;

  memset(&ddsd,0,sizeof(DDSURFACEDESC));
  ddsd.dwSize = sizeof(DDSURFACEDESC);

  if (type==DX5_PAGE_FLIPPED_PRIMARY_SURFACE)
  {
    ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE |
                          DDSCAPS_FLIP |
                          DDSCAPS_COMPLEX |
                          DDSCAPS_3DDEVICE |
                          DDSCAPS_VIDEOMEMORY;
    ddsd.dwBackBufferCount = 1;    
  }
  else if (type==DX5_BACKBUFFERED_PRIMARY_SURFACE)
  {
    ddsd.dwFlags = DDSD_CAPS; 
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_3DDEVICE | DDSCAPS_VIDEOMEMORY;
  }
  else
  {        
    ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
    ddsd.ddsCaps.dwCaps = 0;

    if (format)
    {
      ddsd.dwFlags |= DDSD_PIXELFORMAT;
      ddsd.ddpfPixelFormat = *format;
    }

    ddsd.dwWidth = width;
    ddsd.dwHeight = height;      

    if (flags & DX5_SYSTEM_RAM)
      ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;

    if (flags & DX5_VRAM)
      ddsd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;

    if (flags & DX5_RENDERING_SURFACE)
      ddsd.ddsCaps.dwCaps |= DDSCAPS_3DDEVICE;

    if (type==DX5_TEXTURE)
      ddsd.ddsCaps.dwCaps |= DDSCAPS_TEXTURE;
  }


  result = ddraw->CreateSurface(&ddsd, &surface, NULL);
  if (!i4_dx5_check(result))
    surface=0;

  if (surface)
  {
    surface->QueryInterface(IID_IDirectDrawSurface3,(void **)&surface3);  
    surface->Release();

    if (flags & DX5_CLEAR)
    {
      DDBLTFX fx;
      memset(&fx,0,sizeof(DDBLTFX));
      fx.dwSize = sizeof(DDBLTFX);
      
      RECT r;
      r.left   = 0;
      r.top    = 0;
      r.right  = width;
      r.bottom = height;
      
      surface3->Blt(&r, 0, 0, DDBLT_COLORFILL,&fx);
    }

    if (type==DX5_PAGE_FLIPPED_PRIMARY_SURFACE)
    {
      primary_surface = surface3;
    
      // get the back buffer surface
      DDSCAPS caps;
      caps.dwCaps = DDSCAPS_BACKBUFFER;
      result = primary_surface->GetAttachedSurface(&caps, &back_surface);
      if (!i4_dx5_check(result))
      {
        primary_surface->Release();   primary_surface=0;
        surface3=0;
      }

      caps.dwCaps = DDSCAPS_FRONTBUFFER;
      result = primary_surface->GetAttachedSurface(&caps, &front_surface);
      if (!i4_dx5_check(result))
      {
        back_surface->Release();      back_surface=0;
        primary_surface->Release();   primary_surface=0;
        surface3=0;
      }

    }
    else if (type==DX5_BACKBUFFERED_PRIMARY_SURFACE)
    {   
      DDSURFACEDESC desc;

      get_desc(surface3, desc);

      primary_surface = surface3;
      back_surface = create_surface(DX5_SURFACE, width, height, 
                                    DX5_RENDERING_SURFACE | DX5_VRAM | DX5_CLEAR);

      if (!back_surface)
      {
        primary_surface->Release();
        surface3=0;
      }
    }

  }
  else if (flags & DX5_VRAM)  // try to create it in system ram
  {
    i4_warning("failed to create surface in video memory");
    surface3=create_surface(DX5_SURFACE, width, height, 
                            (flags & (~DX5_VRAM)) | DX5_SYSTEM_RAM, format);
  }

  return surface3;    
}
    

void dx5_common_class::cleanup()
{
  if (back_surface)
  {
    back_surface->Release();
    back_surface=0;
  }

  if (front_surface)
  {
    front_surface->Release();
    front_surface=0;
  }

  if (primary_surface)
  {
    primary_surface->Release();
    primary_surface=0;
  }  

  if (ddraw)
  {
    ddraw->Release();
    ddraw=0;
  }
}
  

dx5_common_class::dx5_common_class()
{
  ddraw=0;
  primary_surface=back_surface=front_surface=0;    

  i4_fmt_565.pixel_depth = I4_16BIT;  
  i4_fmt_565.red_mask    = 31 << 11;
  i4_fmt_565.green_mask  = 63 << 5;
  i4_fmt_565.blue_mask   = 31;
  i4_fmt_565.alpha_mask  = 0;
  i4_fmt_565.lookup = 0;
  i4_fmt_565.calc_shift();

  i4_fmt_1555.pixel_depth = I4_16BIT;  
  i4_fmt_1555.red_mask    = 31 << 10;
  i4_fmt_1555.green_mask  = 31 << 5;
  i4_fmt_1555.blue_mask   = 31;
  i4_fmt_1555.alpha_mask  = 0;
  i4_fmt_1555.lookup = 0;
  i4_fmt_1555.calc_shift();


  memset(&dd_fmt_565,0,sizeof(DDPIXELFORMAT));
  dd_fmt_565.dwSize = sizeof(DDPIXELFORMAT);
  dd_fmt_565.dwFlags = DDPF_RGB;
  dd_fmt_565.dwRGBBitCount = 16;
  dd_fmt_565.dwRBitMask = 31 << 11;
  dd_fmt_565.dwGBitMask = 63 << 5;
  dd_fmt_565.dwBBitMask = 31;

  memset(&dd_fmt_1555,0,sizeof(DDPIXELFORMAT));
  dd_fmt_1555.dwSize = sizeof(DDPIXELFORMAT);
  dd_fmt_1555.dwFlags = DDPF_RGB | DDPF_ALPHAPIXELS;
  dd_fmt_1555.dwRGBBitCount     = 16;    
  dd_fmt_1555.dwRBitMask        = 31 << 10;
  dd_fmt_1555.dwGBitMask        = 31 << 5;
  dd_fmt_1555.dwBBitMask        = 31;
  dd_fmt_1555.dwRGBAlphaBitMask = 1 << 15;

}


//******************************* VIDEO MODE ENUMERATION ***************************
static dx5_mode *dx_mode_list;

HRESULT WINAPI dx5_vidmode_callback(LPDDSURFACEDESC lpDDSurfaceDesc,LPVOID lpContext)
{
  dx5_mode *m=new dx5_mode(dx_mode_list);
  dx_mode_list=m;  
  m->desc=*lpDDSurfaceDesc;

  return DDENUMRET_OK;  
}

dx5_mode *dx5_common_class::get_mode_list(IDirectDraw2 *dd)
{
  dx_mode_list=0;

  DDSURFACEDESC ddsd;
  DDPIXELFORMAT p_format;

  memset(&ddsd,0,sizeof(DDSURFACEDESC));
  ddsd.dwSize   = sizeof(DDSURFACEDESC);  
  ddsd.dwFlags  = DDSD_PIXELFORMAT;

  memset(&p_format,0,sizeof(DDPIXELFORMAT));
  p_format.dwSize = sizeof(DDPIXELFORMAT);
  p_format.dwFlags = DDPF_RGB;
  p_format.dwRGBBitCount = DEPTH;
  ddsd.ddpfPixelFormat = p_format;

  if (!i4_dx5_check(dd->EnumDisplayModes(0,&ddsd,0,(LPDDENUMMODESCALLBACK)dx5_vidmode_callback)))
  {
    free_mode_list(dx_mode_list);
    dx_mode_list=0;
  }

  return dx_mode_list;
}

void dx5_common_class::free_mode_list(dx5_mode *list)
{
  while (list)
  {
    dx5_mode *p=list;
    list=list->next;
    delete p;
  }
}



//************************** DRIVER ENUMERATION AND CREATION ************************
static dx5_driver *dx_driver_list;

BOOL WINAPI dd_device_callback(LPGUID lpGuid, LPSTR lpDeviceDesc,
                               LPSTR lpDriverName, LPVOID lpUserArg)
{
  dx5_driver *d=new dx5_driver(dx_driver_list);
  dx_driver_list=d;
  d->lpGuid = lpGuid;
  strcpy(d->DriverName, lpDriverName);
  strcpy(d->DeviceDesc, lpDeviceDesc);
  return DDENUMRET_OK;  
}

dx5_driver *dx5_common_class::get_driver_list()
{
  dx_driver_list=0;
  if (!i4_dx5_check(DirectDrawEnumerate(dd_device_callback,0)))
  {
    free_driver_list(dx_driver_list);
    dx_driver_list=0;
  }
  return dx_driver_list;
}

void dx5_common_class::free_driver_list(dx5_driver *list)
{
  while (list)
  {
    dx5_driver *n=list;
    list=list->next;
    delete n;
  }
}

static IDirectDraw         *dx5_ddraw;

BOOL WINAPI dd_create_callback(LPGUID lpGuid,	LPSTR lpDeviceDescription,
                               LPSTR lpDriverName, LPVOID lpUserArg)
{
  char *desired_driver = (char *)lpUserArg;
  if (!stricmp(desired_driver,lpDriverName))
  {
    HRESULT res = DirectDrawCreate(lpGuid, &dx5_ddraw, 0);
    return DDENUMRET_CANCEL;
  }
  return DDENUMRET_OK;  
}

IDirectDraw2 *dx5_common_class::initialize_driver(dx5_driver *driver)
{
  if (!driver) return 0;

  dx5_ddraw=0;
  if (!i4_dx5_check(DirectDrawEnumerate(dd_create_callback,(void *)driver->DriverName)))
    return 0;

  if (!dx5_ddraw)
    return 0;

  IDirectDraw2 *dd=0;
  if (!i4_dx5_check(dx5_ddraw->QueryInterface(IID_IDirectDraw2,(void **)&dd)))
  {
    dx5_ddraw->Release();
    return 0;
  }

  dx5_ddraw->Release();  // we are only using DirectDraw2 interface, free this one

  return dd;
}

static LPGUID dx5_hardware_guid;
static dx5_d3d_info dx5_d3d;

HRESULT __stdcall d3d_device_callback(LPGUID lpGuid,	LPSTR lpDeviceDescription,
                                      LPSTR lpDeviceName, LPD3DDEVICEDESC lpD3DHWDeviceDesc,
                                      LPD3DDEVICEDESC lpD3DHELDeviceDesc, LPVOID lpUserArg)
{
  if (strcmp(lpDeviceName, "Direct3D HAL")==0 && lpD3DHWDeviceDesc->dwDeviceZBufferBitDepth!=0)
  {
    dx5_d3d.lpGuid = lpGuid;
    dx5_d3d.lpDeviceName = lpDeviceName;
    dx5_d3d.hw_desc = *lpD3DHWDeviceDesc;
    dx5_d3d.sw_desc = *lpD3DHELDeviceDesc;
  }

  return D3DENUMRET_OK;
}


dx5_d3d_info *dx5_common_class::get_driver_hardware_info(IDirectDraw2 *dd)
{  
  dx5_d3d.lpGuid=0;

  IDirect3D2 *d3d;
  if (!i4_dx5_check(dd->QueryInterface(IID_IDirect3D2,(void **)&d3d)))
    return 0;  
  
  d3d->EnumDevices(d3d_device_callback,0);
  d3d->Release();

  if (dx5_d3d.lpGuid)
    return &dx5_d3d;

  return 0;
}


i4_dx5_image_class::i4_dx5_image_class(w16 _w, w16 _h, w32 flags)
{
  w=_w;
  h=_h;

  DDSURFACEDESC ddsd;
  
  dx5_common.get_surface_description(dx5_common.primary_surface,ddsd);

  i4_pixel_format fmt;
  fmt.pixel_depth = I4_16BIT;  
  fmt.red_mask    = ddsd.ddpfPixelFormat.dwRBitMask;
  fmt.green_mask  = ddsd.ddpfPixelFormat.dwGBitMask;
  fmt.blue_mask   = ddsd.ddpfPixelFormat.dwBBitMask;
  fmt.alpha_mask  = 0;
  fmt.calc_shift();
    
  pal = i4_pal_man.register_pal(&fmt);

  //we want to create the surface w/the same pixel format as the primary surface
  surface = dx5_common.create_surface(DX5_SURFACE, w,h, flags, &ddsd.ddpfPixelFormat);

  if (!surface)  
    i4_warning("i4_dx5_image_class::create_surface failed");
  else
  {
    lock();
    unlock();
  }
}



void i4_dx5_image_class::lock()
{
  DDSURFACEDESC ddsd;
  memset(&ddsd, 0, sizeof(ddsd));
  ddsd.dwSize=sizeof(ddsd);

  if (!i4_dx5_check(surface->Lock(0, &ddsd,DDLOCK_WRITEONLY | DDLOCK_WAIT | DDLOCK_NOSYSLOCK,0)))
    i4_error("couldn't lock surface");

  data = (w8 *)ddsd.lpSurface;
  bpl  = ddsd.lPitch;  
}

void i4_dx5_image_class::unlock()
{
  surface->Unlock(0);
}



IDirectDrawSurface3 *dx5_common_class::get_surface(i4_image_class *im)
{
  return ((i4_dx5_image_class *)im)->surface;

}



i4_image_class *dx5_common_class::create_image(int w, int h, w32 surface_flags)
{
  return new i4_dx5_image_class(w,h,surface_flags); 
}
