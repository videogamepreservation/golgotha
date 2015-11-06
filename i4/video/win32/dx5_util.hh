/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef DX5_UTIL_HH
#define DX5_UTIL_HH




#include <ddraw.h>
#include <d3d.h>
#include "image/image.hh"
#include "palette/pal.hh"
#include "threads/threads.hh"

enum dx5_surface_type
{ 
  DX5_SURFACE,                          // used only for Blt's
  DX5_TEXTURE,
  DX5_PAGE_FLIPPED_PRIMARY_SURFACE,    // will grab backbuffer surface
  DX5_BACKBUFFERED_PRIMARY_SURFACE     // will grab backbuffer surface
} ;


enum
{
  DX5_VRAM              = 1,
  DX5_SYSTEM_RAM        = 2,
  DX5_RENDERING_SURFACE = 4,
  DX5_CLEAR
};

struct dx5_mode
{
  DDSURFACEDESC desc;
  dx5_mode     *next;
  dx5_mode(dx5_mode *next) : next(next) {}
};

struct dx5_driver
{
  LPGUID lpGuid;
  char   DriverName[128];
  char   DeviceDesc[128];
  dx5_driver *next;
  dx5_driver(dx5_driver *next) : next(next) {}
};

struct dx5_d3d_info
{
  LPGUID lpGuid;
  LPSTR  lpDeviceName;
  D3DDEVICEDESC hw_desc,sw_desc;
};




class dx5_common_class
{

public:
  static IDirectDraw2          *ddraw;
  static IDirectDrawSurface3   *primary_surface, *back_surface, *front_surface;
  static DDPIXELFORMAT          dd_fmt_565, dd_fmt_1555;
  static i4_pixel_format        i4_fmt_565, i4_fmt_1555;

  dx5_common_class();

  IDirectDrawSurface3 *create_surface(dx5_surface_type type,
                                      int width=0, int height=0, // not need for primary
                                      int flags=0,
                                      DDPIXELFORMAT *format=0); // format not need for primary


  i4_bool get_surface_description(IDirectDrawSurface3 *surface, DDSURFACEDESC &surface_desc)
  {
    memset(&surface_desc,0,sizeof(DDSURFACEDESC));
    surface_desc.dwSize = sizeof(DDSURFACEDESC);
    return (i4_bool)(surface->GetSurfaceDesc(&surface_desc)==DD_OK);
  }

  i4_bool get_desc(IDirectDrawSurface3 *surface, DDSURFACEDESC &surface_desc)
  {
    memset(&surface_desc,0,sizeof(DDSURFACEDESC));
    surface_desc.dwSize = sizeof(DDSURFACEDESC);
    return (i4_bool)(surface->GetSurfaceDesc(&surface_desc)==DD_OK);
  }


  dx5_d3d_info *get_driver_hardware_info(IDirectDraw2 *dd);

  dx5_mode *get_mode_list(IDirectDraw2 *dd);
  void free_mode_list(dx5_mode *list);

  dx5_driver *get_driver_list();
  void free_driver_list(dx5_driver *list);
  IDirectDraw2 *initialize_driver(dx5_driver *driver);

  IDirectDrawSurface3 *get_surface(i4_image_class *im);
  i4_image_class *create_image(int w, int h, w32 surface_flags);

  void cleanup();
};


class i4_dx5_image_class : public i4_image_class
{
public:
  IDirectDrawSurface3 *surface;
  i4_dx5_image_class(w16 w, w16 h, w32 surface_flags=DX5_SYSTEM_RAM);
  ~i4_dx5_image_class()
  {
    if (surface)
    {
      surface->PageUnlock(0);
      surface->Release();
    }
  }

  w16 *paddr(int x, int y) { return (w16 *)(((w8 *)data)+y*bpl) + x; }
  i4_color get_pixel(i4_coord x, i4_coord y)
  {
    return i4_pal_man.convert_to_32(*paddr(x,y), pal);
  }
  
  void put_pixel(i4_coord x, i4_coord y, w32 color)
  {
    w16 *addr=paddr(x,y);
    *addr=i4_pal_man.convert_32_to(color, &pal->source);  
  }



  void lock();
  void unlock();
};


extern dx5_common_class dx5_common;


#endif
