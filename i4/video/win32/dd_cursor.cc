/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "video/win32/dd_cursor.hh"
#include "video/win32/dx5_util.hh"
#include "video/win32/dx5_error.hh"
#include "image/context.hh"

ddraw_thread_cursor_class::ddraw_thread_cursor_class(const i4_pal *screen_pal,
                                                     HWND window_handle,
                                                     sw32 clip_x1, sw32 clip_y1,
                                                     sw32 clip_x2, sw32 clip_y2)
: dx_threaded_mouse_class(screen_pal,window_handle,clip_x1,clip_y1,clip_x2,clip_y2)
{
  cursor.pict = 0;  
  mouse_save  = 0;

  current_mouse_x = 0;
  current_mouse_y = 0;
  
  save_mouse_x    = 0;
  save_mouse_y    = 0;

  visible = i4_F;
  
  use_back_buffer = i4_F;
}

void ddraw_thread_cursor_class::set_cursor(i4_cursor_class *c)
{  
  draw_lock.lock();

  remove();

  if (cursor.pict)
  {
    delete cursor.pict;
    cursor.pict = 0;
  }

  if (mouse_save)
  {
    delete mouse_save;
    mouse_save = 0;
  }

  if (c && c->pict)
  {
    int cw=c->pict->width(), ch=c->pict->height();

    i4_draw_context_class context(0,0, cw-1, ch-1);
    
    i4_dx5_image_class *dx5_image = new i4_dx5_image_class(cw, ch, DX5_VRAM);

    //setup room for the mouse save
    mouse_save = new i4_dx5_image_class(cw,ch,DX5_VRAM);

    //setup the color-key value for the mouse (black)
    DDCOLORKEY colorkey;
    memset(&colorkey,0,sizeof(DDCOLORKEY));

    colorkey.dwColorSpaceLowValue  = c->trans;
    colorkey.dwColorSpaceHighValue = c->trans;

    dx5_image->surface->SetColorKey(DDCKEY_SRCBLT,&colorkey);

    cursor      = *c;
    cursor.pict = dx5_image;

    dx5_image->lock();
    c->pict->put_image(cursor.pict, 0,0, context);
    dx5_image->unlock();    
    
    display();
  }    

  draw_lock.unlock();
}

void ddraw_thread_cursor_class::save()
{  
  if (!mouse_save)
  {    
    return;  
  }

  RECT r;
  
  r.left   = save_mouse_x;
  r.top    = save_mouse_y;
  r.right  = r.left + mouse_save->width();
  r.bottom = r.top  + mouse_save->height();
                              
  

  IDirectDrawSurface3 *framebuffer = (use_back_buffer)?(dx5_common.back_surface):(dx5_common.primary_surface);

  HRESULT res =
  mouse_save->surface->BltFast(0,
                               0,
                               framebuffer,
                               &r,
                               DDBLTFAST_NOCOLORKEY | 
                               DDBLTFAST_WAIT);
  i4_dx5_check(res);

}

void ddraw_thread_cursor_class::remove()
{
  if (!visible || !mouse_save || !mouse_save->surface)
  {
    return;
  }


  IDirectDrawSurface3 *framebuffer = (use_back_buffer)?(dx5_common.back_surface):(dx5_common.primary_surface);
  HRESULT res =
  framebuffer->BltFast(save_mouse_x,
                       save_mouse_y,
                       mouse_save->surface,
                       0,
                       DDBLTFAST_NOCOLORKEY | 
                       DDBLTFAST_WAIT);
  i4_dx5_check(res);

  visible = i4_F;

}

void ddraw_thread_cursor_class::display()
{  
  if (!cursor.pict || !mouse_save || !mouse_save->surface)
  {
    return;
  }


  
  save_mouse_x = current_mouse_x;
  save_mouse_y = current_mouse_y;  
  
  RECT r;
  
  r.left   = save_mouse_x;
  r.top    = save_mouse_y;
  r.right  = r.left + mouse_save->width();
  r.bottom = r.top + mouse_save->height();


  IDirectDrawSurface3 *framebuffer = (use_back_buffer)?(dx5_common.back_surface):(dx5_common.primary_surface);
  HRESULT res =
  mouse_save->surface->BltFast(0,
                               0,
                               framebuffer,
                               &r,
                               DDBLTFAST_NOCOLORKEY | 
                               DDBLTFAST_WAIT);
  i4_dx5_check(res);

  IDirectDrawSurface3 *mouse_draw_surface = ((i4_dx5_image_class *)cursor.pict)->surface;
  
  res =
  framebuffer->BltFast(save_mouse_x,
                       save_mouse_y,
                       mouse_draw_surface,
                       0,
                       DDBLTFAST_SRCCOLORKEY | 
                       DDBLTFAST_WAIT);
  
  i4_dx5_check(res);  

  
  visible = i4_T;  
}

void ddraw_thread_cursor_class::use_backbuffer(i4_bool use)
{
  use_back_buffer = use;
}
