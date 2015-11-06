/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include <ddraw.h>
#include <windows.h>
#include "arch.hh"
#include "error/error.hh"

#ifdef DEBUG

i4_bool i4_dx5_check(HRESULT res)
{
  if (res==DD_OK) return 1;

  switch (res)
  {
    case DDERR_ALREADYINITIALIZED :
      i4_warning("The object has already been initialized. ");
      break;
    case DDERR_BLTFASTCANTCLIP :
      i4_warning("A DirectDrawClipper object is attached to a source surface that has "
                 "passed into a call to the IDirectDrawSurface2::BltFast method. ");
      break;
    case DDERR_CANNOTATTACHSURFACE :
      i4_warning("A surface cannot be attached to another requested surface. ");
      break;
    case DDERR_CANNOTDETACHSURFACE :
      i4_warning("A surface cannot be detached from another requested surface. ");
      break;
    case DDERR_CANTCREATEDC :
      i4_warning("Windows cannot create any more device contexts (DCs). ");
      break;
    case DDERR_CANTDUPLICATE :
      i4_warning("Primary and 3-D surfaces, or surfaces that are implicitly created, "
                 "cannot be duplicated. ");
      break;
    case DDERR_CANTLOCKSURFACE :
      i4_warning("Access to this surface is refused because an attempt was made to lock "
                 "the primary surface without DCI support. ");
      break;
    case DDERR_CANTPAGELOCK :
      i4_warning("An attempt to page lock a surface failed. Page lock will not work on a "
                 "display-memory surface or an emulated primary surface. ");
      break;
    case DDERR_CANTPAGEUNLOCK :
      i4_warning("An attempt to page unlock a surface failed. Page unlock will not work "
                 "on a display-memory surface or an emulated primary surface. ");
      break;
    case DDERR_CLIPPERISUSINGHWND :
      i4_warning("An attempt was made to set a clip list for a DirectDrawClipper object "
                 "that is already monitoring a window handle. ");
      break;
    case DDERR_COLORKEYNOTSET :
      i4_warning("No source color key is specified for this operation. ");
      break;
    case DDERR_CURRENTLYNOTAVAIL :
      i4_warning("No support is currently available. ");
      break;
    case DDERR_DCALREADYCREATED :
      i4_warning("A device context (DC) has already been returned for this surface. Only "
                 "one DC can be retrieved for each surface. ");
      break;
    case DDERR_DEVICEDOESNTOWNSURFACE:
      i4_warning("Surfaces created by one direct draw device cannot be used directly by "
                 "another direct draw device.");
      break;
    case DDERR_DIRECTDRAWALREADYCREATED :
      i4_warning("A DirectDraw object representing this driver has already been created "
                 "for this process. ");
      break;
    case DDERR_EXCEPTION :
      i4_warning("An exception was encountered while performing the requested operation. ");
      break;
    case DDERR_EXCLUSIVEMODEALREADYSET :
      i4_warning("An attempt was made to set the cooperative level when it was already set "
                 "to exclusive. ");
      break;
    case DDERR_GENERIC :
      i4_warning("There is an undefined error condition. ");
      break;
    case DDERR_HEIGHTALIGN :
      i4_warning("The height of the provided rectangle is not a multiple of the "
                 "required alignment. ");
      break;
    case DDERR_HWNDALREADYSET :
      i4_warning("The DirectDraw cooperative level window handle has already been set. "
                 "It cannot be reset while the process has surfaces or palettes created. ");
      break;
    case DDERR_HWNDSUBCLASSED :
      i4_warning("DirectDraw is prevented from restoring state because the DirectDraw "
                 "cooperative level window handle has been subclassed. ");
      break;
    case DDERR_IMPLICITLYCREATED :
      i4_warning("The surface cannot be restored because it is an implicitly created surface. ");
      break;
    case DDERR_INCOMPATIBLEPRIMARY :
      i4_warning("The primary surface creation request does not match with the existing "
                 "primary surface. ");
      break;
    case DDERR_INVALIDCAPS :
      i4_warning("One or more of the capability bits passed to the callback function are "
                 "incorrect. ");
      break;
    case DDERR_INVALIDCLIPLIST :
      i4_warning("DirectDraw does not support the provided clip list. ");
      break;
    case DDERR_INVALIDDIRECTDRAWGUID :
      i4_warning("The globally unique identifier (GUID) passed to the DirectDrawCreate "
                 "function is not a valid DirectDraw driver identifier. ");
      break;
    case DDERR_INVALIDMODE :
      i4_warning("DirectDraw does not support the requested mode. ");
      break;
    case DDERR_INVALIDOBJECT :
      i4_warning("DirectDraw received a pointer that was an invalid DirectDraw object. ");
      break;
    case DDERR_INVALIDPARAMS :
      i4_warning("One or more of the parameters passed to the method are incorrect. ");
      break;
    case DDERR_INVALIDPIXELFORMAT :
      i4_warning("The pixel format was invalid as specified. ");
      break;
    case DDERR_INVALIDPOSITION :
      i4_warning("The position of the overlay on the destination is no longer legal. ");
      break;
    case DDERR_INVALIDRECT :
      i4_warning("The provided rectangle was invalid. ");
      break;
    case DDERR_INVALIDSURFACETYPE :
      i4_warning("The requested operation could not be performed because the surface was "
                 "of the wrong type. ");
      break;
    case DDERR_LOCKEDSURFACES :
      i4_warning("One or more surfaces are locked, causing the failure of the requested "
                 "operation. ");
      break;
    case DDERR_MOREDATA:
      i4_warning("There is more data available than the specified buffer size can hold.");
      break;
    case DDERR_NO3D :
      i4_warning("No 3-D hardware or emulation is present. ");
      break;
    case DDERR_NOALPHAHW :
      i4_warning("No alpha acceleration hardware is present or available, causing the "
                 "failure of the requested operation. ");
      break;
    case DDERR_NOBLTHW :
      i4_warning("No blitter hardware is present. ");
      break;
    case DDERR_NOCLIPLIST :
      i4_warning("No clip list is available. ");
      break;
    case DDERR_NOCLIPPERATTACHED :
      i4_warning("No DirectDrawClipper object is attached to the surface object. ");
      break;
    case DDERR_NOCOLORCONVHW :
      i4_warning("The operation cannot be carried out because no color-conversion hardware "
                 "is present or available. ");
      break;
    case DDERR_NOCOLORKEY :
      i4_warning("The surface does not currently have a color key. ");
      break;
    case DDERR_NOCOLORKEYHW :
      i4_warning("The operation cannot be carried out because there is no hardware support "
                 "for the destination color key. ");
      break;
    case DDERR_NOCOOPERATIVELEVELSET :
      i4_warning("A create function is called without the IDirectDraw2::SetCooperativeLevel "
                 "method being called. ");
      break;
    case DDERR_NODC :
      i4_warning("No DC has ever been created for this surface. ");
      break;
    case DDERR_NODDROPSHW :
      i4_warning("No DirectDraw raster operation (ROP) hardware is available. ");
      break;
    case DDERR_NODIRECTDRAWHW :
      i4_warning("Hardware-only DirectDraw object creation is not possible; the driver does "
                 "not support any hardware. ");
      break;
    case DDERR_NODIRECTDRAWSUPPORT :
      i4_warning("DirectDraw support is not possible with the current display driver. ");
      break;
    case DDERR_NOEMULATION :
      i4_warning("Software emulation is not available. ");
      break;
    case DDERR_NOEXCLUSIVEMODE :
      i4_warning("The operation requires the application to have exclusive mode, but the "
                 "application does not have exclusive mode. ");
      break;
    case DDERR_NOFLIPHW :
      i4_warning("Flipping visible surfaces is not supported. ");
      break;
    case DDERR_NOGDI :
      i4_warning("No GDI is present. ");
      break;
    case DDERR_NOHWND :
      i4_warning("Clipper notification requires a window handle, or no window handle has "
                 "been previously set as the cooperative level window handle. ");
      break;
    case DDERR_NOMIPMAPHW :
      i4_warning("The operation cannot be carried out because no mipmap texture mapping "
                 "hardware is present or available. ");
      break;
    case DDERR_NOMIRRORHW :
      i4_warning("The operation cannot be carried out because no mirroring hardware is "
                 "present or available. ");
      break;
    case DDERR_NOOVERLAYDEST :
      i4_warning("The IDirectDrawSurface2::GetOverlayPosition method is called on an "
                 "overlay that the IDirectDrawSurface2::UpdateOverlay method has not "
                 "been called on to establish a destination. ");
      break;
    case DDERR_NOOVERLAYHW :
      i4_warning("The operation cannot be carried out because no overlay hardware is "
                 "present or available. ");
      break;
    case DDERR_NOPALETTEATTACHED :
      i4_warning("No palette object is attached to this surface. ");
      break;
    case DDERR_NOPALETTEHW :
      i4_warning("There is no hardware support for 16- or 256-color palettes. ");
      break;
    case DDERR_NORASTEROPHW :
      i4_warning("The operation cannot be carried out because no appropriate raster "
                 "operation hardware is present or available. ");
      break;
    case DDERR_NOROTATIONHW :
      i4_warning("The operation cannot be carried out because no rotation hardware is "
                 "present or available. ");
      break;
    case DDERR_NOSTRETCHHW :
      i4_warning("The operation cannot be carried out because there is no hardware support "
                 "for stretching. ");
      break;
    case DDERR_NOT4BITCOLOR :
      i4_warning("The DirectDrawSurface object is not using a 4-bit color palette and "
                 "the requested operation requires a 4-bit color palette. ");
      break;
    case DDERR_NOT4BITCOLORINDEX :
      i4_warning("The DirectDrawSurface object is not using a 4-bit color index palette "
                 "and the requested operation requires a 4-bit color index palette. ");
      break;
    case DDERR_NOT8BITCOLOR :
      i4_warning("The DirectDrawSurface object is not using an 8-bit color palette "
                 "and the requested operation requires an 8-bit color palette. ");
      break;
    case DDERR_NOTAOVERLAYSURFACE :
      i4_warning("An overlay component is called for a non-overlay surface. ");
      break;
    case DDERR_NOTEXTUREHW :
      i4_warning("The operation cannot be carried out because no texture-mapping "
                 "hardware is present or available. ");
      break;
    case DDERR_NOTFLIPPABLE :
      i4_warning("An attempt has been made to flip a surface that cannot be flipped. ");
      break;
    case DDERR_NOTFOUND :
      i4_warning("The requested item was not found. ");
      break;
    case DDERR_NOTINITIALIZED :
      i4_warning("An attempt was made to call an interface method of a DirectDraw "
                 "object created by CoCreateInstance before the object was initialized. ");
      break;
    case DDERR_NOTLOCKED :
      i4_warning("An attempt is made to unlock a surface that was not locked. ");
      break;
    case DDERR_NOTPAGELOCKED :
      i4_warning("An attempt is made to page unlock a surface with no outstanding page locks. ");
      break;
    case DDERR_NOTPALETTIZED :
      i4_warning("The surface being used is not a palette-based surface. ");
      break;
    case DDERR_NOVSYNCHW :
      i4_warning("The operation cannot be carried out because there is no hardware support "
                 "for vertical blank synchronized operations. ");
      break;
    case DDERR_NOZBUFFERHW :
      i4_warning("The operation to create a z-buffer in display memory or to perform a "
                 "blit using a z-buffer cannot be carried out because there is no hardware "
                 "support for z-buffers. ");
      break;
    case DDERR_NOZOVERLAYHW :
      i4_warning("The overlay surfaces cannot be z-layered based on the z-order "
                 "because the hardware does not support z-ordering of overlays. ");
      break;
    case DDERR_OUTOFCAPS :
      i4_warning("The hardware needed for the requested operation has already been allocated. ");
      break;
    case DDERR_OUTOFMEMORY :
      i4_warning("DirectDraw does not have enough memory to perform the operation. ");
      break;
    case DDERR_OUTOFVIDEOMEMORY :
      i4_warning("DirectDraw does not have enough display memory to perform the operation. ");
      break;
    case DDERR_OVERLAYCANTCLIP :
      i4_warning("The hardware does not support clipped overlays. ");
      break;
    case DDERR_OVERLAYCOLORKEYONLYONEACTIVE :
      i4_warning("An attempt was made to have more than one color key active on an overlay.  ");
      break;
    case DDERR_OVERLAYNOTVISIBLE :
      i4_warning("The IDirectDrawSurface2::GetOverlayPosition method is called on a "
                 "hidden overlay. ");
      break;
    case DDERR_PALETTEBUSY :
      i4_warning("Access to this palette is refused because the palette is locked by "
                 "another thread. ");
      break;
    case DDERR_PRIMARYSURFACEALREADYEXISTS :
      i4_warning("This process has already created a primary surface. ");
      break;
    case DDERR_REGIONTOOSMALL :
      i4_warning("The region passed to the IDirectDrawClipper::GetClipList method is too small. ");
      break;
    case DDERR_SURFACEALREADYATTACHED :
      i4_warning("An attempt was made to attach a surface to another surface to which it"
                 " is already attached. ");
      break;
    case DDERR_SURFACEALREADYDEPENDENT :
      i4_warning("An attempt was made to make a surface a dependency of another "
                 "surface to which it is already dependent. ");
      break;
    case DDERR_SURFACEBUSY :
      i4_warning("Access to the surface is refused because the surface is "
                 "locked by another thread. ");
      break;
    case DDERR_SURFACEISOBSCURED :
      i4_warning("Access to the surface is refused because the surface is obscured. ");
      break;
    case DDERR_SURFACELOST :
      i4_warning("Access to the surface is refused because the surface memory is gone. "
                 "The DirectDrawSurface object representing this surface should have the "
                 "IDirectDrawSurface2::Restore method called on it. ");
      break;
    case DDERR_SURFACENOTATTACHED :
      i4_warning("The requested surface is not attached. ");
      break;
    case DDERR_TOOBIGHEIGHT :
      i4_warning("The height requested by DirectDraw is too large. ");
      break;
    case DDERR_TOOBIGSIZE :
      i4_warning("The size requested by DirectDraw is too large. "
                 "However, the individual height and width are OK. ");
      break;
    case DDERR_TOOBIGWIDTH :
      i4_warning("The width requested by DirectDraw is too large. ");
      break;
    case DDERR_UNSUPPORTED :
      i4_warning("The operation is not supported. ");
      break;
    case DDERR_UNSUPPORTEDFORMAT :
      i4_warning("The FourCC format requested is not supported by DirectDraw. ");
      break;
    case DDERR_UNSUPPORTEDMASK :
      i4_warning("The bitmask in the pixel format requested is not supported by DirectDraw. ");
      break;
    case DDERR_UNSUPPORTEDMODE :
      i4_warning("The display is currently in an unsupported mode. ");
      break;
    case DDERR_VERTICALBLANKINPROGRESS :
      i4_warning("A vertical blank is in progress. ");
      break;
    case DDERR_VIDEONOTACTIVE:
      i4_warning("The video port is not active.");
      break;
    case DDERR_WASSTILLDRAWING :
      i4_warning("The previous blit operation that is transferring information "
                 "to or from this surface is incomplete. ");
      break;
    case DDERR_WRONGMODE :
      i4_warning("This surface cannot be restored because it was created in a different mode. ");
      break;
    case DDERR_XALIGN :
      i4_warning("The provided rectangle was not horizontally aligned on a required boundary.");
      break;
  }
  return 0;
}

#endif
