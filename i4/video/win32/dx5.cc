/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "video/win32/dx5.hh"
#include "image/image.hh"
#include "time/profile.hh"
#include "main/win_main.hh"
#include "image/context.hh"

#include "video/win32/dx5_util.hh"
#include "video/win32/dx5_error.hh"
#include "threads/threads.hh"


static i4_profile_class pf_dx5_mouse("dx5::mouse draw"),
  pf_dx5_copy("dx5::copy dirty"),
  pf_dx5_flip("dx5::flip"),
  pf_dx5_lock("dx5::lock");

i4_dx5_display_class i4_dx5_display_class_instance;

i4_dx5_display_class *i4_dx5_display=0;

struct gr_buf_status_type
{
  sw8            state;  // -1 = not locked, else  
                         //    I4_FRAME_BUFFER_READ,
                         // or I4_FRAME_BUFFER_WRITE
  i4_image_class *im;


} dx_buf[2];    // I4_FRONT_FRAME_BUFFER=0, I4_BACK_FRAME_BUFFER=1



int dx5_error_function(const char *str)
{
  dx5_common.cleanup();
  i4_dx5_display->error_handler.old_error_handler(str);
  return 0;
}

i4_refresh_type i4_dx5_display_class::update_model()
{
  if (use_page_flip)
    return I4_PAGE_FLIP_REFRESH;
  else
    return I4_BLT_REFRESH;
  
}

i4_image_class *i4_dx5_display_class::lock_frame_buffer(i4_frame_buffer_type type,
                                                  i4_frame_access_type access)
{
  if (access==dx_buf[type].state) 
    return dx_buf[type].im;
  else if (dx_buf[type].state!=-1)
    unlock_frame_buffer(type);

  HRESULT res=0;

  DDSURFACEDESC cur_dx5_lock_info;
  memset(&cur_dx5_lock_info,0,sizeof(DDSURFACEDESC));
  cur_dx5_lock_info.dwSize = sizeof(DDSURFACEDESC);

  IDirectDrawSurface3 *surf=type==I4_FRONT_FRAME_BUFFER ? 
    dx5_common.front_surface : dx5_common.back_surface;

  int flags=access==I4_FRAME_BUFFER_READ ? DDLOCK_READONLY :
    DDLOCK_WRITEONLY;
          
  if (!surf)
    return 0;

  res = surf->Lock(NULL,&cur_dx5_lock_info, flags | DDLOCK_WAIT,0);

  if (res == DD_OK)
  {
#ifdef DEBUG
    surf->Unlock(NULL); 
#endif

    dx_buf[type].im->data=cur_dx5_lock_info.lpSurface;
    dx_buf[type].im->bpl=cur_dx5_lock_info.lPitch;
    dx_buf[type].state=access;
  }

  return dx_buf[type].im;
}


void i4_dx5_display_class::unlock_frame_buffer(i4_frame_buffer_type type)
{
  IDirectDrawSurface3 *surf=type==I4_FRONT_FRAME_BUFFER ? 
    dx5_common.front_surface : dx5_common.back_surface;
          
#ifndef DEBUG
  if (surf)
    surf->Unlock(NULL); 
#endif

  dx_buf[type].im->data=0;
  dx_buf[type].state=-1;
}



// i4_image_class *i4_dx5_display_class::grab_framebuffer()
// { 
//   if (!fake_screen) return 0;

//   i4_image_class *im=i4_create_image(width(), height(), fake_screen->get_pal());

//   void *mem=dx5_lock(DX5_LOCK_READ);


//   for (int y=0; y<height(); y++)
//   {
//     w16 *p=(w16 *)((w8 *)im->data + y*im->bpl);

//     for (int x=0; x<width(); x++)
//     {      
//       p[x]=*(((w16 *)mem)+x);
//     }

//     mem=(w8 *)mem + cur_dx5_lock_info.lPitch;
//   }

//   dx5_unlock();

//   return im;
// }

i4_dx5_display_class::i4_dx5_display_class():input(0)
{
  mode_list=0;
  i4_dx5_display=this;
  mouse=0;
  fake_screen=0;
  context=0;  

#ifdef DEBUG
  use_exclusive_mode = i4_F;
  use_page_flip      = i4_F;
#else
  use_exclusive_mode = i4_T;
  use_page_flip      = i4_F;
#endif
}


i4_image_class *i4_dx5_display_class::get_screen()
{      
  return fake_screen;
}



// this will add all the drivers that directx find into the display list
void i4_dx5_display_class::init()
{
  int id=0;
  dx5_driver *list=dx5_common.get_driver_list();
  char *n=name_buffer;

  for (dx5_driver *d=list; d; d=d->next, id++)
  {    
    IDirectDraw2 *ddraw=dx5_common.initialize_driver(d);
    if (ddraw)
    {

      // no hardware acceleration for this demo
      if (dx5_common.get_driver_hardware_info(ddraw))
      {
        sprintf(n, "%s : 3d Accelerated", d->DriverName);
        i4_display_list_struct *s=new i4_display_list_struct;
        s->add_to_list(n, id | 0x8000, this, i4_display_list);
        n+=strlen(n)+1;
      }
    
      //only want software rendering stuffs for the primary display driver to be listed
      if (!stricmp(d->DriverName,"display"))
      {
        sprintf(n, "%s : Software Rendered", d->DriverName);
        i4_display_list_struct *s=new i4_display_list_struct;
        s->add_to_list(n, id, this, i4_display_list);
        n+=strlen(n)+1;
      }

      ddraw->Release();
    }
  }

  dx5_common.free_driver_list(list);
}

// find the driver, then get it's modes and return the first one
i4_display_class::mode *i4_dx5_display_class::get_first_mode(int driver_id)
{
  int find_id = driver_id & (~0x8000);    // top bit indicates hardware accelerated

  if (mode_list)
  {
    dx5_common.free_mode_list(mode_list);
    mode_list=0;
  }

  dx5_driver *driver_list=dx5_common.get_driver_list(), *d;
  for (d=driver_list; find_id && d; d=d->next, find_id--);
  IDirectDraw2 *ddraw=dx5_common.initialize_driver(d); 
  dx5_common.free_driver_list(driver_list);

  if (!ddraw) return 0;

  mode_list=dx5_common.get_mode_list(ddraw);
  ddraw->Release();
  
  amode.dx5=mode_list;
  amode.driver_id=driver_id;


  return get_next_mode();
}


i4_dx5_display_class::~i4_dx5_display_class()
{
  ;
}

i4_display_class::mode *i4_dx5_display_class::get_next_mode()
{
  if (!amode.dx5) 
    return 0;


  DDSURFACEDESC *desc=&amode.dx5->desc;

  sprintf(amode.name, "%d X %d 16bit", desc->dwWidth, desc->dwHeight);
  amode.xres=desc->dwWidth;
  amode.yres=desc->dwHeight;
  amode.bits_per_pixel=16;
  amode.red_mask    = desc->ddpfPixelFormat.dwRBitMask;
  amode.green_mask  = desc->ddpfPixelFormat.dwGBitMask;
  amode.blue_mask   = desc->ddpfPixelFormat.dwBBitMask;  
  amode.flags =0;


  last_mode=amode;
  amode.dx5=amode.dx5->next;
  return &amode;
}


i4_bool i4_dx5_display_class::initialize_mode()
{
  if (!i4_win32_startup_options.fullscreen)
    use_page_flip = i4_F;
  
  if (!input.create_window(0,0, 
                           last_mode.xres, 
                           last_mode.yres, this, i4_win32_startup_options.fullscreen))
    return i4_F;

  
  int x,y,w,h;

  if (!i4_win32_startup_options.fullscreen)
    input.get_window_area(x,y,w,h);
  else
  {
    w=last_mode.xres;
    h=last_mode.yres;
  }

  last_mode.xres=w;
  last_mode.yres=h;

  // find the driver and initialize it
  int find_id = last_mode.driver_id & (~0x8000);

  dx5_driver *driver_list=dx5_common.get_driver_list(), *d;
  for (d=driver_list; find_id && d; d=d->next, find_id--);
  dx5_common.ddraw=dx5_common.initialize_driver(d); 
  dx5_common.free_driver_list(driver_list);
    
  if (!dx5_common.ddraw) 
    return i4_F;


  if (i4_win32_startup_options.fullscreen && use_exclusive_mode)
  {
    // change the error handling function so that the screen is restored before
    // a dialog box is displayed, or they won't be able to read the message!
    if (i4_dx5_check(dx5_common.ddraw->SetCooperativeLevel(input.get_window_handle(),
                                                           DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN)))
    {
      error_handler.old_error_handler=i4_get_error_function();
      error_handler.need_restore_old=i4_T;
      i4_set_error_function(dx5_error_function);
    }
    else return i4_F;
  }
  else if (!i4_dx5_check(dx5_common.ddraw->SetCooperativeLevel(input.get_window_handle(),
                                                               DDSCL_NORMAL)))
    return i4_F;

  //win95 function to hide the mouse cursor. our display uses our own cursor

  if (i4_win32_startup_options.fullscreen && 
      !i4_dx5_check(dx5_common.ddraw->SetDisplayMode(last_mode.xres, last_mode.yres,16,0,0)))
      return i4_F;


  if (use_page_flip)
  {
    if (!dx5_common.create_surface(DX5_PAGE_FLIPPED_PRIMARY_SURFACE, 
                                   last_mode.xres, last_mode.yres))
      return i4_F;
  }
  else if (!dx5_common.create_surface(DX5_BACKBUFFERED_PRIMARY_SURFACE, 
                                      last_mode.xres, last_mode.yres))
    return i4_F;
  

  // find the size of the primary surface
  DDSURFACEDESC ddsd;
  dx5_common.get_surface_description(dx5_common.primary_surface, ddsd); 
  fake_screen=new i4_dx5_image_class(last_mode.xres, last_mode.yres);
  fake_screen->surface->PageLock(0);
    
  


  i4_pixel_format fmt;
  fmt.pixel_depth = I4_16BIT;  
  fmt.red_mask    = ddsd.ddpfPixelFormat.dwRBitMask;
  fmt.green_mask  = ddsd.ddpfPixelFormat.dwGBitMask;
  fmt.blue_mask   = ddsd.ddpfPixelFormat.dwBBitMask;
  fmt.alpha_mask  = 0;
  fmt.calc_shift();

  pal = i4_pal_man.register_pal(&fmt);

  dx_buf[0].im=i4_create_image(last_mode.xres, last_mode.yres, pal, 0, 0);
  dx_buf[0].state=-1;

  dx_buf[1].im=i4_create_image(last_mode.xres, last_mode.yres, pal, 0, 0);
  dx_buf[1].state=-1;


  context             = new i4_draw_context_class(0,0, last_mode.xres-1, last_mode.yres-1);
  context->both_dirty = new i4_rect_list_class;

  memcpy(&cur_mode, &last_mode, sizeof(last_mode));

  if (1) //!i4_win32_startup_options.fullscreen)
    mouse = new dx5_mouse_class(use_page_flip);
  else
  {
    thread_mouse = new ddraw_thread_cursor_class(pal,input.get_window_handle(),
                                                 0,0,last_mode.xres,last_mode.yres);

    input.set_async_mouse(thread_mouse);
  }

  return i4_T;
}



i4_bool i4_dx5_display_class::close()
{
  if (mouse)
  {
    delete mouse;
    mouse = 0;
  }

  input.set_async_mouse(0);

  if (thread_mouse)
  {
    delete thread_mouse;
    thread_mouse = 0;
  }

  if (error_handler.need_restore_old)
  {
    i4_set_error_function(error_handler.old_error_handler);
    error_handler.need_restore_old=i4_F;
  }

  input.destroy_window();

  for (int i=0; i<2; i++)
  {
    if (dx_buf[i].im)
    {
      delete dx_buf[i].im;
      dx_buf[i].im=0;
    }
  }

  if (fake_screen)
  {
    delete fake_screen;
    fake_screen=0;
  }

  if (context)
  {
    delete context;
    context=0;
  }

  if (mode_list)
  {
    dx5_common.free_mode_list(mode_list);
    mode_list=0;
  }

  next_frame_copy.delete_list();

  dx5_common.cleanup();

  return i4_T;
}


i4_bool i4_dx5_display_class::set_mouse_shape(i4_cursor_class *cursor)
{
  if (mouse)
  {
    mouse->set_cursor(cursor);    
    return i4_T;
  }
  else
  if (thread_mouse)
  {
    thread_mouse->set_cursor(cursor);
    return i4_T;
  }

  return i4_F;
}

void i4_dx5_display_class::flush()
{    
  RECT src;

  if (!input.get_active())
    {
      context->both_dirty->delete_list();
      context->both_dirty->add_area(0,0,width()-1, height()-1);
      i4_thread_yield();
      return ;
    }

  //start of copy stuff
  pf_dx5_copy.start();
  context->both_dirty->intersect_area(0,0,width()-1,height()-1);

  // Step 1 : copy middle buffer information to the back-buffer

  i4_rect_list_class::area_iter a,b;
  i4_rect_list_class *use_list;

  // if page flipped we need to make add the current dirty to the stuff left over from last frame
  if (use_page_flip)
  {
    a=context->both_dirty->list.begin();
    for (;a!=context->both_dirty->list.end();++a)
      next_frame_copy.add_area(a->x1, a->y1, a->x2, a->y2);
    use_list=&next_frame_copy;
  }
  else
    use_list=context->both_dirty;


  int wx,wy,ww,wh;
  input.get_window_area(wx,wy,ww,wh);

  for (a=use_list->list.begin(); a!=use_list->list.end(); ++a)
  {        
    src.left   = a->x1;
    src.top    = a->y1;
    src.right  = a->x2+1;
    src.bottom = a->y2+1;

    if (!i4_dx5_check(dx5_common.back_surface->BltFast(a->x1, a->y1, fake_screen->surface,
                                                       &src,DDBLTFAST_NOCOLORKEY | 
                                                       DDBLTFAST_WAIT)))
      i4_error("dx5 blt failed on area %d %d %d %d ", a->x1, a->y1,a->x2, a->y2);

  }    
  pf_dx5_copy.stop();
  //end of copy stuff

  //start of mouse stuff
  pf_dx5_mouse.start(); 

  mouse_x = input.mouse_x;
  mouse_y = input.mouse_y;

  if (mouse)
    mouse->save_and_draw(mouse_x, mouse_y);

  pf_dx5_mouse.stop();  


  //start of flip stuff
  pf_dx5_flip.start();  
  
  // now flip or copy the backbuffer to video memory
  if (use_page_flip)
  {
//     if (thread_mouse)
//     {
//       thread_mouse->draw_lock.lock();
//       thread_mouse->use_backbuffer(i4_F);
//       thread_mouse->remove(); //remove from front buffer
//       thread_mouse->use_backbuffer(i4_T);      
//       thread_mouse->display();//display on back buffer
//     }

    dx5_common.primary_surface->Flip(NULL,DDFLIP_WAIT);

//     if (thread_mouse)
//     {      
//       thread_mouse->use_backbuffer(i4_F);
//       thread_mouse->draw_lock.unlock();
//     }
  }
  else
  {
//     if (thread_mouse)
//     {
//       thread_mouse->draw_lock.lock();      
//       thread_mouse->use_backbuffer(i4_T);
//       thread_mouse->display();      
//     }

    if (!i4_dx5_check(dx5_common.primary_surface->BltFast(0,0,dx5_common.back_surface, NULL,
                                                          DDBLTFAST_NOCOLORKEY |
                                                          DDBLTFAST_WAIT)))
      i4_error("dx5 blt failed");
    
//     if (thread_mouse)
//     {      
//       thread_mouse->remove();
//       thread_mouse->set_visible(i4_T);        
//       thread_mouse->use_backbuffer(i4_F);
//       thread_mouse->draw_lock.unlock();
//     }
  }
  
  pf_dx5_flip.stop();
  //end of flip stuff


  pf_dx5_mouse.start();
  if (mouse)
    mouse->restore();
  pf_dx5_mouse.stop();  


  if (use_page_flip)
    next_frame_copy.swap(context->both_dirty);

  context->both_dirty->delete_list();
}


