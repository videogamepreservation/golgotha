/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifdef _WINDOWS
//#define INITGUID

#include "video/win32/directx.hh"
#include "device/event.hh"
#include <d3d.h>
#include <d3drm.h>
//#include "video/win32/dx_cursor.hh"
#endif

// this should be defined in main/win_main
extern HINSTANCE my_instance;
extern int       my_nCmdShow;

extern sw32 win32_mouse_x, win32_mouse_y;

static directx_display_class directx_display_instance;

char* D3DAppErrorToString(HRESULT error);

void dx_error(sw32 result)
{
  i4_error(D3DAppErrorToString(result));
}

// called when windows gives us a WM_MOVE message, this tells everyone else interested
void directx_display_class::move_screen(i4_coord x, i4_coord y)   
{

  if (back_buffer)  // make sure the screen has been created so far
  {
    // if the location is not word alligned move the 
    // window and exit (we will get another message later when this takes effect)
    if (x&3)   
    {
      SetWindowPos(input.get_window_handle(),
                   HWND_TOP,
                   x&(~3),y,
                   cur_mode.xres,
                   cur_mode.yres,
          0);
      return ;
    }
  }
}




i4_bool directx_display_class::set_mouse_shape(i4_cursor_class *c)

{
  return i4_T;
}


directx_display_class::directx_display_class()
{
  mouse_pict=0;
  mouse_save=0;
  mouse_trans=0;

  context=0;
  direct_draw_driver=0;
  primary_surface=0;
  exclusive_mode=i4_F;
  full_screen=i4_F;
  back_buffer=0;
  back_surface=0;
}



HRESULT __stdcall dx_mode_callback(LPDDSURFACEDESC desc, LPVOID context)
{
  directx_display_class *dx=(directx_display_class *)context;

  if (((desc->ddpfPixelFormat.dwRGBBitCount+1)&(~1)) == I4_BYTES_PER_PIXEL*8)
  {
    if (dx->last_found && 
        dx->last_found->xres==desc->dwWidth &&        
        dx->last_found->yres==desc->dwHeight)
      dx->saw_last=i4_T;
    else if (!dx->last_found || (dx->saw_last))                            
    {
      dx->saw_last=i4_F;

      dx->last_found=&dx->amode;
      dx->last_found->xres=desc->dwWidth;
      dx->last_found->yres=desc->dwHeight;
      dx->last_found->flags = i4_display_class::mode::PAGE_FLIPPED;
      dx->last_found->bits_per_pixel = desc->ddpfPixelFormat.dwRGBBitCount;
      dx->last_found->assoc=dx;

      w32 rbits=0, gbits=0, bbits=0,m,tbits, depth;
  
      m = desc->ddpfPixelFormat.dwRBitMask;
      if (!m) return D3DENUMRET_OK;
      dx->last_found->red_mask=m;

      for (; !(m & 1);  m >>= 1);
      for (; m&1; m>>=1) rbits++;

      m = desc->ddpfPixelFormat.dwGBitMask;
      if (!m) return D3DENUMRET_OK;
      dx->last_found->green_mask=m;
      for (; !(m & 1);  m >>= 1);
      for (; m&1; m>>=1) gbits++;

      m = desc->ddpfPixelFormat.dwBBitMask;
      if (!m) return D3DENUMRET_OK;
      dx->last_found->blue_mask=m;
      for (; !(m & 1);  m >>= 1);
      for (; m&1; m>>=1) bbits++;

      sprintf(dx->last_found->name,"Page flipped %d x %d x %d (%d%d%d)",
              desc->dwWidth,
              desc->dwHeight,
              desc->ddpfPixelFormat.dwRGBBitCount,
              rbits, gbits, bbits);

      i4_warning("Mode : %s\n",dx->last_found->name);

    }

  }
  return  D3DENUMRET_OK;
}

i4_display_class::mode *directx_display_class::get_next_mode(i4_display_class::mode *last_mode)
{
  sw32 last_w=last_mode->xres,
       last_h=last_mode->yres;

  if (!full_screen)
    return 0;
  else
  {
    saw_last = i4_F;
    last_found = (directx_display_class::directx_mode *)last_mode;

    if (!exclusive_mode)
    {
      direct_draw_driver->SetCooperativeLevel(input.get_window_handle(),
                                              DDSCL_ALLOWREBOOT |
                                              DDSCL_EXCLUSIVE |
                                              DDSCL_FULLSCREEN );

      direct_draw_driver->EnumDisplayModes(0, 0, this,  dx_mode_callback);

      direct_draw_driver->SetCooperativeLevel(input.get_window_handle(),
                                              DDSCL_NORMAL);

    }
    else
      direct_draw_driver->EnumDisplayModes(0, 0, this,  dx_mode_callback);
    
    if (last_found->xres==last_w && last_found->yres==last_h)
      return 0;
    else
      return last_found;
  }
}

i4_display_class::mode *directx_display_class::get_first_mode()
{
  if (!direct_draw_driver)
  {
    if (!create_direct_draw())    // initialize the direct draw driver if not already
      return 0;
  }

  if (full_screen)
  {    
    saw_last = i4_F;
    last_found = 0;


    if (!exclusive_mode)
    {
      direct_draw_driver->SetCooperativeLevel(input.get_window_handle(),
                                              DDSCL_ALLOWREBOOT |
                                              DDSCL_EXCLUSIVE |
                                              DDSCL_FULLSCREEN );

      direct_draw_driver->EnumDisplayModes(0, 0, this,  dx_mode_callback);

      direct_draw_driver->SetCooperativeLevel(input.get_window_handle(),
                                              DDSCL_NORMAL);
    }
    else
      direct_draw_driver->EnumDisplayModes(0, 0, this,  dx_mode_callback);




    return last_found;
  }
  else
  {
    // get the current video mode that windows is running in
    DDSURFACEDESC surface_description; 
    // just for luck, clear this out
    memset(&surface_description,0,sizeof(surface_description));    
    // need to tell win how big this structure is
    surface_description.dwSize=sizeof(surface_description);
  

    // ask windows to fill in the struct
    direct_draw_driver->GetDisplayMode(&surface_description);      


    // since we only support one bit depth per
    // executable make sure windows is running in the bit depth
    // this executable was made for
    if (surface_description.ddpfPixelFormat.dwRGBBitCount!=I4_BYTES_PER_PIXEL*8)
      return 0;
  
  
  // fill in one of our own mode strucutres to report to the caller
 
    memset(&amode,0,sizeof(amode));
    strcpy(amode.name,"Win32 direct screen window");
    amode.flags=i4_display_class::mode::RESOLUTION_DETERMINED_ON_OPEN;
    amode.bits_per_pixel=I4_SCREEN_DEPTH;

    amode.xres=surface_description.dwWidth;
    amode.yres=surface_description.dwHeight;

    amode.red_mask=surface_description.ddpfPixelFormat.dwRBitMask;
    amode.green_mask=surface_description.ddpfPixelFormat.dwGBitMask;
    amode.blue_mask=surface_description.ddpfPixelFormat.dwBBitMask;


    amode.assoc=this;          // so we know in 'initialize_mode' that we created this mode

  }

  return &amode;
}




i4_bool directx_display_class::create_direct_draw()
{
  if (direct_draw_driver)  // is it already created?
    return i4_T;

  // try to create it
  HRESULT ret = DirectDrawCreate( NULL, &direct_draw_driver, NULL );

  // failed ?
  if ( ret != DD_OK )
    return i4_F;
 
  return i4_T;
}

void directx_display_class::destroy_direct_draw()
{
  if (direct_draw_driver)
  {
    direct_draw_driver->Release();
    direct_draw_driver=0;
  }
}



i4_bool directx_display_class::close()
{  
  if (context)
  {
    delete context;
    context=0;
  }

  if (exclusive_mode && direct_draw_driver)
  {
    direct_draw_driver->SetCooperativeLevel(input.get_window_handle(), DDSCL_NORMAL );
    direct_draw_driver->RestoreDisplayMode();
    exclusive_mode=i4_F;
  }

  if (primary_surface)
  {
    primary_surface->Release();
    primary_surface=0;
  }

  if (back_buffer)
  {
    delete back_buffer;
    back_buffer=0;
  }

  input.destroy_window();
  return i4_T;
}

i4_bool directx_display_class::initialize_mode(i4_display_class::mode *which_one)
{
  // make sure this is a mode we listed, and not some other driver
  if ((((directx_mode *)which_one)->assoc)==this)  
  {
    memcpy(&cur_mode,which_one,sizeof(cur_mode));

    close();

    w32 width=which_one->xres, height=which_one->yres;

    if (!direct_draw_driver)
    {
      if (!create_direct_draw())    // initialize the direct draw driver if not already
        return i4_F;
    }  


    input.create_window(0,0,width, height, this);

    HRESULT ddrval;

    if (!full_screen)
      ddrval = direct_draw_driver->SetCooperativeLevel(input.get_window_handle(), 
                                                       DDSCL_NORMAL );
    else if (!exclusive_mode)
    {
      ddrval = direct_draw_driver->SetCooperativeLevel( input.get_window_handle(), 
                                                       DDSCL_ALLOWREBOOT |
                                                       DDSCL_EXCLUSIVE |
                                                       DDSCL_FULLSCREEN );
      exclusive_mode=i4_T;
      if (ddrval!=DD_OK)
        dx_error(ddrval);

      /*
        SetWindowPos(win32_display_instance.window_handle, HWND_BOTTOM,
        0,600, 320, 100, SWP_NOACTIVATE | SWP_NOZORDER);
        */
  
    }

    if (full_screen)
      direct_draw_driver->SetDisplayMode(which_one->xres, 
                                         which_one->yres, 
                                         I4_BYTES_PER_PIXEL*8
                                         );


    if (ddrval != DD_OK)
    {
      input.destroy_window();
      return i4_F;
    }


    if (!create_surfaces(&cur_mode))
    {
      primary_surface=0;
      input.destroy_window();
      return i4_F;
    }

    if (context)
      delete context;

    context=new i4_draw_context_class(0,0,width-1,height-1);
    context->both_dirty=new i4_rect_list_class;
    context->both_dirty->add_area(0,0,width-1,height-1);
    context->single_dirty=new i4_rect_list_class;

    return i4_T;

  } else return i4_F; 
}



void directx_display_class::flush() 
{
  // add the single dirty area into the both dirty list
  i4_rect_list_class::area_iter a;
  for (a=context->single_dirty->list.begin(); 
       a!=context->single_dirty->list.end();
       ++a)
    context->both_dirty->add_area(a->x1, a->y1, a->x2, a->y2);

  context->single_dirty->delete_list();

  if (context->both_dirty->empty())
      return ;
  
  context->both_dirty->delete_list();  
}


w32 directx_display_class::priority()
{
  if (create_direct_draw())
  {
    destroy_direct_draw();
    return 2;
  } 
  else 
    return 0;
}


i4_bool directx_display_class::create_surfaces(directx_mode *mode)
{
  // Create the primary surface
  DDSURFACEDESC       ddsd;
  ddsd.dwSize = sizeof( ddsd );
  ddsd.dwFlags = DDSD_CAPS;
  ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
  HRESULT ddrval = direct_draw_driver->CreateSurface( &ddsd, &primary_surface,0);
  if (ddrval != DD_OK)
  {
    dx_error(ddrval);
    return i4_F;
  }
    
  create_back_surface(mode);


  win32_mouse_x=-1;  // the mouse thread will wait till this is not -1 before it
  // starts tracking the mouse (then we will know where the system mouse is)


  return i4_T;
}


i4_bool directx_display_class::create_back_surface(directx_mode *mode)
{
  DDSURFACEDESC   ddsd;

  ddsd.dwSize = sizeof(ddsd);
  ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
  ddsd.dwWidth = mode->xres;
  ddsd.dwHeight = mode->yres;
  ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN; // | DDSCAPS_SYSTEMMEMORY;

  // create a new back buffer
  HRESULT er = direct_draw_driver->CreateSurface(&ddsd, &back_surface, 0);
  if (er != DD_OK)
  {
    dx_error(er);
    return i4_F;
  }


  i4_pixel_format fmt;
  fmt.pixel_depth=I4_16BIT;
  fmt.red_mask=mode->red_mask;
  fmt.green_mask=mode->green_mask;
  fmt.blue_mask=mode->blue_mask;
  fmt.alpha_mask=0;
  fmt.calc_shift();

  pal = i4_pal_man.register_pal(&fmt);

  // create the i4_image_class which points to the surface memory
  er=back_surface->Lock(0, &ddsd, DDLOCK_WAIT, 0);
  if (er == DD_OK)
  {
    w8 *v_addr=(w8 *)ddsd.lpSurface;                 // this is the address
    back_surface->Unlock(0);   // Now that we have the video address, unlock so we can debug!
    if (back_surface)
      delete back_buffer;                                   // delete the old screen

    // create the image that maps to screen memory
    back_buffer=new I4_SCREEN_TYPE(cur_mode.xres,
                                   cur_mode.yres,  
                                   pal,
                                   ddsd.lPitch,
                                   v_addr);
    if (context)
      delete context;

    context=new i4_draw_context_class(0,0,
                                      back_buffer->width()-1,
                                      back_buffer->height()-1);


    context->both_dirty=new i4_rect_list_class;
    context->single_dirty=new i4_rect_list_class;
  }
  else
  {
    dx_error(er);
    return i4_F;
  }
  return i4_T;
}



void directx_display_class::get_window_pos(sw32 &x, sw32 &y)
{
  POINT p;
  p.x=0;
  p.y=0;
  ClientToScreen(input.get_window_handle(), &p);
  x=p.x;
  y=p.y;
}

void directx_display_class::active(i4_bool act)
{
  /*  if (cursor)
    cursor->active(act);  */
}

i4_bool directx_display_class::available()
{
  int need_destroy=direct_draw_driver != NULL;
  
  if (create_direct_draw())
  {
    if (need_destroy)
      destroy_direct_draw();
    return i4_T;
  }
  else return i4_F;
}


i4_image_class *directx_display_class::get_screen()
{
  return back_buffer;
}

char* D3DAppErrorToString(HRESULT error)
{
  switch(error) {
    case DD_OK:
      return "No error.\0";
    case DDERR_ALREADYINITIALIZED:
      return "This object is already initialized.\0";
    case DDERR_BLTFASTCANTCLIP:
      return "Return if a clipper object is attached to the source surface passed into a BltFast call.\0";
    case DDERR_CANNOTATTACHSURFACE:
      return "This surface can not be attached to the requested surface.\0";
    case DDERR_CANNOTDETACHSURFACE:
      return "This surface can not be detached from the requested surface.\0";
    case DDERR_CANTCREATEDC:
      return "Windows can not create any more DCs.\0";
    case DDERR_CANTDUPLICATE:
      return "Can't duplicate primary & 3D surfaces, or surfaces that are implicitly created.\0";
    case DDERR_CLIPPERISUSINGHWND:
      return "An attempt was made to set a cliplist for a clipper object that is already monitoring an hwnd.\0";
    case DDERR_COLORKEYNOTSET:
      return "No src color key specified for this operation.\0";
    case DDERR_CURRENTLYNOTAVAIL:
      return "Support is currently not available.\0";
    case DDERR_DIRECTDRAWALREADYCREATED:
      return "A DirectDraw object representing this driver has already been created for this process.\0";
    case DDERR_EXCEPTION:
      return "An exception was encountered while performing the requested operation.\0";
    case DDERR_EXCLUSIVEMODEALREADYSET:
      return "An attempt was made to set the cooperative level when it was already set to exclusive.\0";
    case DDERR_GENERIC:
      return "Generic failure.\0";
    case DDERR_HEIGHTALIGN:
      return "Height of rectangle provided is not a multiple of reqd alignment.\0";
    case DDERR_HWNDALREADYSET:
      return "The CooperativeLevel HWND has already been set. It can not be reset while the process has surfaces or palettes created.\0";
    case DDERR_HWNDSUBCLASSED:
      return "HWND used by DirectDraw CooperativeLevel has been subclassed, this prevents DirectDraw from restoring state.\0";
    case DDERR_IMPLICITLYCREATED:
      return "This surface can not be restored because it is an implicitly created surface.\0";
    case DDERR_INCOMPATIBLEPRIMARY:
      return "Unable to match primary surface creation request with existing primary surface.\0";
    case DDERR_INVALIDCAPS:
      return "One or more of the caps bits passed to the callback are incorrect.\0";
    case DDERR_INVALIDCLIPLIST:
      return "DirectDraw does not support the provided cliplist.\0";
    case DDERR_INVALIDDIRECTDRAWGUID:
      return "The GUID passed to DirectDrawCreate is not a valid DirectDraw driver identifier.\0";
    case DDERR_INVALIDMODE:
      return "DirectDraw does not support the requested mode.\0";
    case DDERR_INVALIDOBJECT:
      return "DirectDraw received a pointer that was an invalid DIRECTDRAW object.\0";
    case DDERR_INVALIDPARAMS:
      return "One or more of the parameters passed to the function are incorrect.\0";
    case DDERR_INVALIDPIXELFORMAT:
      return "The pixel format was invalid as specified.\0";
    case DDERR_INVALIDPOSITION:
      return "Returned when the position of the overlay on the destination is no longer legal for that destination.\0";
    case DDERR_INVALIDRECT:
      return "Rectangle provided was invalid.\0";
    case DDERR_LOCKEDSURFACES:
      return "Operation could not be carried out because one or more surfaces are locked.\0";
    case DDERR_NO3D:
      return "There is no 3D present.\0";
    case DDERR_NOALPHAHW:
      return "Operation could not be carried out because there is no alpha accleration hardware present or available.\0";
    case DDERR_NOBLTHW:
      return "No blitter hardware present.\0";
    case DDERR_NOCLIPLIST:
      return "No cliplist available.\0";
    case DDERR_NOCLIPPERATTACHED:
      return "No clipper object attached to surface object.\0";
    case DDERR_NOCOLORCONVHW:
      return "Operation could not be carried out because there is no color conversion hardware present or available.\0";
    case DDERR_NOCOLORKEY:
      return "Surface doesn't currently have a color key\0";
    case DDERR_NOCOLORKEYHW:
      return "Operation could not be carried out because there is no hardware support of the destination color key.\0";
    case DDERR_NOCOOPERATIVELEVELSET:
      return "Create function called without DirectDraw object method SetCooperativeLevel being called.\0";
    case DDERR_NODC:
      return "No DC was ever created for this surface.\0";
    case DDERR_NODDROPSHW:
      return "No DirectDraw ROP hardware.\0";
    case DDERR_NODIRECTDRAWHW:
      return "A hardware-only DirectDraw object creation was attempted but the driver did not support any hardware.\0";
    case DDERR_NOEMULATION:
      return "Software emulation not available.\0";
    case DDERR_NOEXCLUSIVEMODE:
      return "Operation requires the application to have exclusive mode but the application does not have exclusive mode.\0";
    case DDERR_NOFLIPHW:
      return "Flipping visible surfaces is not supported.\0";
    case DDERR_NOGDI:
      return "There is no GDI present.\0";
    case DDERR_NOHWND:
      return "Clipper notification requires an HWND or no HWND has previously been set as the CooperativeLevel HWND.\0";
    case DDERR_NOMIRRORHW:
      return "Operation could not be carried out because there is no hardware present or available.\0";
    case DDERR_NOOVERLAYDEST:
      return "Returned when GetOverlayPosition is called on an overlay that UpdateOverlay has never been called on to establish a destination.\0";
    case DDERR_NOOVERLAYHW:
      return "Operation could not be carried out because there is no overlay hardware present or available.\0";
    case DDERR_NOPALETTEATTACHED:
      return "No palette object attached to this surface.\0";
    case DDERR_NOPALETTEHW:
      return "No hardware support for 16 or 256 color palettes.\0";
    case DDERR_NORASTEROPHW:
      return "Operation could not be carried out because there is no appropriate raster op hardware present or available.\0";
    case DDERR_NOROTATIONHW:
      return "Operation could not be carried out because there is no rotation hardware present or available.\0";
    case DDERR_NOSTRETCHHW:
      return "Operation could not be carried out because there is no hardware support for stretching.\0";
    case DDERR_NOT4BITCOLOR:
      return "DirectDrawSurface is not in 4 bit color palette and the requested operation requires 4 bit color palette.\0";
    case DDERR_NOT4BITCOLORINDEX:
      return "DirectDrawSurface is not in 4 bit color index palette and the requested operation requires 4 bit color index palette.\0";
    case DDERR_NOT8BITCOLOR:
      return "DirectDrawSurface is not in 8 bit color mode and the requested operation requires 8 bit color.\0";
    case DDERR_NOTAOVERLAYSURFACE:
      return "Returned when an overlay member is called for a non-overlay surface.\0";
    case DDERR_NOTEXTUREHW:
      return "Operation could not be carried out because there is no texture mapping hardware present or available.\0";
    case DDERR_NOTFLIPPABLE:
      return "An attempt has been made to flip a surface that is not flippable.\0";
    case DDERR_NOTFOUND:
      return "Requested item was not found.\0";
    case DDERR_NOTLOCKED:
      return "Surface was not locked.  An attempt to unlock a surface that was not locked at all, or by this process, has been attempted.\0";
    case DDERR_NOTPALETTIZED:
      return "The surface being used is not a palette-based surface.\0";
    case DDERR_NOVSYNCHW:
      return "Operation could not be carried out because there is no hardware support for vertical blank synchronized operations.\0";
    case DDERR_NOZBUFFERHW:
      return "Operation could not be carried out because there is no hardware support for zbuffer blitting.\0";
    case DDERR_NOZOVERLAYHW:
      return "Overlay surfaces could not be z layered based on their BltOrder because the hardware does not support z layering of overlays.\0";
    case DDERR_OUTOFCAPS:
      return "The hardware needed for the requested operation has already been allocated.\0";
    case DDERR_OUTOFMEMORY:
      return "DirectDraw does not have enough memory to perform the operation.\0";
    case DDERR_OUTOFVIDEOMEMORY:
      return "DirectDraw does not have enough memory to perform the operation.\0";
    case DDERR_OVERLAYCANTCLIP:
      return "The hardware does not support clipped overlays.\0";
    case DDERR_OVERLAYCOLORKEYONLYONEACTIVE:
      return "Can only have ony color key active at one time for overlays.\0";
    case DDERR_OVERLAYNOTVISIBLE:
      return "Returned when GetOverlayPosition is called on a hidden overlay.\0";
    case DDERR_PALETTEBUSY:
      return "Access to this palette is being refused because the palette is already locked by another thread.\0";
    case DDERR_PRIMARYSURFACEALREADYEXISTS:
      return "This process already has created a primary surface.\0";
    case DDERR_REGIONTOOSMALL:
      return "Region passed to Clipper::GetClipList is too small.\0";
    case DDERR_SURFACEALREADYATTACHED:
      return "This surface is already attached to the surface it is being attached to.\0";
    case DDERR_SURFACEALREADYDEPENDENT:
      return "This surface is already a dependency of the surface it is being made a dependency of.\0";
    case DDERR_SURFACEBUSY:
      return "Access to this surface is being refused because the surface is already locked by another thread.\0";
    case DDERR_SURFACEISOBSCURED:
      return "Access to surface refused because the surface is obscured.\0";
    case DDERR_SURFACELOST:
      return "Access to this surface is being refused because the surface memory is gone. The DirectDrawSurface object representing this surface should have Restore called on it.\0";
    case DDERR_SURFACENOTATTACHED:
      return "The requested surface is not attached.\0";
    case DDERR_TOOBIGHEIGHT:
      return "Height requested by DirectDraw is too large.\0";
    case DDERR_TOOBIGSIZE:
      return "Size requested by DirectDraw is too large, but the individual height and width are OK.\0";
    case DDERR_TOOBIGWIDTH:
      return "Width requested by DirectDraw is too large.\0";
    case DDERR_UNSUPPORTED:
      return "Action not supported.\0";
    case DDERR_UNSUPPORTEDFORMAT:
      return "FOURCC format requested is unsupported by DirectDraw.\0";
    case DDERR_UNSUPPORTEDMASK:
      return "Bitmask in the pixel format requested is unsupported by DirectDraw.\0";
    case DDERR_VERTICALBLANKINPROGRESS:
      return "Vertical blank is in progress.\0";
    case DDERR_WASSTILLDRAWING:
      return "Informs DirectDraw that the previous Blt which is transfering information to or from this Surface is incomplete.\0";
    case DDERR_WRONGMODE:
      return "This surface can not be restored because it was created in a different mode.\0";
    case DDERR_XALIGN:
      return "Rectangle provided was not horizontally aligned on required boundary.\0";
    case D3DERR_BADMAJORVERSION:
      return "D3DERR_BADMAJORVERSION\0";
    case D3DERR_BADMINORVERSION:
      return "D3DERR_BADMINORVERSION\0";
    case D3DERR_EXECUTE_LOCKED:
      return "D3DERR_EXECUTE_LOCKED\0";
    case D3DERR_EXECUTE_NOT_LOCKED:
      return "D3DERR_EXECUTE_NOT_LOCKED\0";
    case D3DERR_EXECUTE_CREATE_FAILED:
      return "D3DERR_EXECUTE_CREATE_FAILED\0";
    case D3DERR_EXECUTE_DESTROY_FAILED:
      return "D3DERR_EXECUTE_DESTROY_FAILED\0";
    case D3DERR_EXECUTE_LOCK_FAILED:
      return "D3DERR_EXECUTE_LOCK_FAILED\0";
    case D3DERR_EXECUTE_UNLOCK_FAILED:
      return "D3DERR_EXECUTE_UNLOCK_FAILED\0";
    case D3DERR_EXECUTE_FAILED:
      return "D3DERR_EXECUTE_FAILED\0";
    case D3DERR_EXECUTE_CLIPPED_FAILED:
      return "D3DERR_EXECUTE_CLIPPED_FAILED\0";
    case D3DERR_TEXTURE_NO_SUPPORT:
      return "D3DERR_TEXTURE_NO_SUPPORT\0";
    case D3DERR_TEXTURE_NOT_LOCKED:
      return "D3DERR_TEXTURE_NOT_LOCKED\0";
    case D3DERR_TEXTURE_LOCKED:
      return "D3DERR_TEXTURELOCKED\0";
    case D3DERR_TEXTURE_CREATE_FAILED:
      return "D3DERR_TEXTURE_CREATE_FAILED\0";
    case D3DERR_TEXTURE_DESTROY_FAILED:
      return "D3DERR_TEXTURE_DESTROY_FAILED\0";
    case D3DERR_TEXTURE_LOCK_FAILED:
      return "D3DERR_TEXTURE_LOCK_FAILED\0";
    case D3DERR_TEXTURE_UNLOCK_FAILED:
      return "D3DERR_TEXTURE_UNLOCK_FAILED\0";
    case D3DERR_TEXTURE_LOAD_FAILED:
      return "D3DERR_TEXTURE_LOAD_FAILED\0";
    case D3DERR_MATRIX_CREATE_FAILED:
      return "D3DERR_MATRIX_CREATE_FAILED\0";
    case D3DERR_MATRIX_DESTROY_FAILED:
      return "D3DERR_MATRIX_DESTROY_FAILED\0";
    case D3DERR_MATRIX_SETDATA_FAILED:
      return "D3DERR_MATRIX_SETDATA_FAILED\0";
    case D3DERR_SETVIEWPORTDATA_FAILED:
      return "D3DERR_SETVIEWPORTDATA_FAILED\0";
    case D3DERR_MATERIAL_CREATE_FAILED:
      return "D3DERR_MATERIAL_CREATE_FAILED\0";
    case D3DERR_MATERIAL_DESTROY_FAILED:
      return "D3DERR_MATERIAL_DESTROY_FAILED\0";
    case D3DERR_MATERIAL_SETDATA_FAILED:
      return "D3DERR_MATERIAL_SETDATA_FAILED\0";
    case D3DERR_LIGHT_SET_FAILED:
      return "D3DERR_LIGHT_SET_FAILED\0";
    case D3DRMERR_BADOBJECT:
      return "D3DRMERR_BADOBJECT\0";
    case D3DRMERR_BADTYPE:
      return "D3DRMERR_BADTYPE\0";
    case D3DRMERR_BADALLOC:
      return "D3DRMERR_BADALLOC\0";
    case D3DRMERR_FACEUSED:
      return "D3DRMERR_FACEUSED\0";
    case D3DRMERR_NOTFOUND:
      return "D3DRMERR_NOTFOUND\0";
    case D3DRMERR_NOTDONEYET:
      return "D3DRMERR_NOTDONEYET\0";
    case D3DRMERR_FILENOTFOUND:
      return "The file was not found.\0";
    case D3DRMERR_BADFILE:
      return "D3DRMERR_BADFILE\0";
    case D3DRMERR_BADDEVICE:
      return "D3DRMERR_BADDEVICE\0";
    case D3DRMERR_BADVALUE:
      return "D3DRMERR_BADVALUE\0";
    case D3DRMERR_BADMAJORVERSION:
      return "D3DRMERR_BADMAJORVERSION\0";
    case D3DRMERR_BADMINORVERSION:
      return "D3DRMERR_BADMINORVERSION\0";
    case D3DRMERR_UNABLETOEXECUTE:
      return "D3DRMERR_UNABLETOEXECUTE\0";
    default:
      return "Unrecognized error value.\0";
  }
}
