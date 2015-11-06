/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include <windows.h>
#include <ddraw.h>
#include <math.h>
#include <memory.h>

#include "r1_clip.hh"
#include "r1_win.hh"

#include "device/processor.hh"

#include "software/r1_software.hh"
#include "software/r1_software_texture.hh"
#include "software/r1_software_globals.hh"
#include "software/mappers.hh"
#include "software/span_buffer.hh"
#include "software/inline_fpu.hh"
#include "software/win32_specific.hh"

#include "video/win32/dx5.hh"
#include "video/win32/dx5_util.hh"

#include "time/profile.hh"

static i4_profile_class pf_software_flush_spans("software::flush_spans");
static i4_profile_class pf_software_draw("software::draw()");
static i4_profile_class pf_software_blt("software::blt()");

void r1_software_lock_render_buffer()
{  
  if (r1_software_render_buffer_is_locked) return;
  
  DDSURFACEDESC ddsd;
  memset(&ddsd,0,sizeof(DDSURFACEDESC));
  ddsd.dwSize = sizeof(DDSURFACEDESC);


  r1_software_render_surface->Lock(NULL,&ddsd,DDLOCK_WAIT | DDLOCK_NOSYSLOCK,0);
  
  r1_software_render_buffer_ptr       = (w16 *)ddsd.lpSurface;
  r1_software_render_buffer_bpl       = ddsd.lPitch;
  r1_software_render_buffer_wpl       = ddsd.lPitch>>1;    
  
  if (r1_software_render_expand_type==R1_COPY_1x1_SCANLINE_SKIP)
    r1_software_render_buffer_wpl *= 2;
  
  r1_software_render_buffer_height    = ddsd.dwHeight;  
  r1_software_render_buffer_is_locked = i4_T;
}

void r1_software_unlock_render_buffer()
{
  if (!r1_software_render_buffer_is_locked) return;
  r1_software_render_surface->Unlock(NULL);
  r1_software_render_buffer_is_locked = i4_F;  
}

i4_bool r1_software_class::init(i4_display_class *display)
{
  if (display!=i4_dx5_display || i4_dx5_display->using_accelerated_driver())
    return i4_F;
  
  fmt = display->get_palette()->source;
  
  init_span_buffer();

  init_color_tints();

  tmanager = new r1_software_texture_class(display->get_palette());
  
  texture_mode = 0;
  
  write_mask = R1_WRITE_W | R1_COMPARE_W | R1_WRITE_COLOR;
  set_write_mode(write_mask);

  shade_mode = R1_COLORED_SHADING;
  set_shading_mode(shade_mode);

  alpha_mode = R1_ALPHA_DISABLED;
  set_alpha_mode(alpha_mode);

  inverse_leftover_lookup_init(); //init the 1/<blah> table
  initialize_function_pointers(i4_F); //init the function pointer tables, default to intel functions

  return i4_T;
}


i4_bool r1_software_class::expand_type_supported(r1_expand_type type)
{ 
  if (type==R1_COPY_2x2)
  {
    DDCAPS card_caps, HELcaps;
    memset(&card_caps, 0, sizeof(card_caps));
    card_caps.dwSize=sizeof(DDCAPS);  
    memset(&HELcaps, 0, sizeof(HELcaps));
    HELcaps.dwSize=sizeof(DDCAPS);  
    dx5_common.ddraw->GetCaps(&card_caps, &HELcaps);
    
    if (card_caps.dwCaps & DDCAPS_BLTSTRETCH)
      return i4_T;
    else
      return i4_F;
  }

  if (type==R1_COPY_1x1 || type==R1_COPY_1x1_SCANLINE_SKIP)
    return i4_T;

  return i4_F;
}





r1_software_render_window_class::r1_software_render_window_class(
                               w16 w, w16 h,r1_expand_type expand_type,
                               r1_render_api_class *api)
                               :r1_render_window_class(w,h,expand_type,api)
{
  sw32 wi,hi;
  w32  flags;

  if (expand_type==R1_COPY_2x2)      // if stretching determine if driver support bltstretch
  {
    DDCAPS card_caps, HELcaps;
    memset(&card_caps, 0, sizeof(card_caps));
    card_caps.dwSize=sizeof(DDCAPS);  
    memset(&HELcaps, 0, sizeof(HELcaps));
    HELcaps.dwSize=sizeof(DDCAPS);  
    dx5_common.ddraw->GetCaps(&card_caps, &HELcaps);
    
    if (card_caps.dwCaps & DDCAPS_BLTSTRETCH)
    {
      flags = DX5_VRAM;
      wi = w>>1;
      hi = h>>1;
    }
    else
    {      
      flags = DX5_SYSTEM_RAM;
      wi = w>>1;
      hi = h>>1;
    }        
  }
  else
  {
    wi = w;
    hi = h;
    flags = DX5_VRAM;
  }
  
  surface = dx5_common.create_surface(DX5_SURFACE, wi,hi, flags);  
  
  if (surface==0)
    i4_error("Unable to create surface for r1_software_render_window_class");

  DDBLTFX fx;
  memset(&fx,0,sizeof(DDBLTFX));
  fx.dwSize = sizeof(DDBLTFX);
    
  fx.dwFillColor = 0x0000;
  surface->Blt(NULL,NULL,NULL,DDBLT_WAIT | DDBLT_COLORFILL,&fx);    
}

r1_render_window_class *r1_software_class::create_render_window(int w, int h,
                                             r1_expand_type expand_type)
{
  r1_software_render_window_class *new_window;  
  if (w & 1) w++;
  if (h & 1) h++;
  new_window = new r1_software_render_window_class(w,h, expand_type, this);  
  return new_window;
}

void r1_software_render_window_class::receive_event(i4_event *ev)
{
  if (expand_type==R1_COPY_2x2 && ev->type()==i4_event::MOUSE_MOVE)
  {
    CAST_PTR(mev, i4_mouse_move_event_class, ev);

    int o_lx=mev->lx, o_ly=mev->ly, o_x=mev->x, o_y=mev->y;

    mev->lx/=2;
    mev->ly/=2;
    mev->x/=2;
    mev->y/=2;

    i4_parent_window_class::receive_event(ev);

    mev->lx=o_lx;
    mev->ly=o_ly;
    mev->x=o_x;
    mev->y=o_y;
  } 
  else i4_parent_window_class::receive_event(ev);
}


void r1_software_class::copy_part(i4_image_class *im,
                                  int x, int y,             // position on screen
                                  int x1, int y1,           // area of image to copy 
                                  int x2, int y2)
{
  //this better be true


  RECT srcRect;
  srcRect.top     = y1;
  srcRect.bottom  = y2+1;
  srcRect.left    = x1;
  srcRect.right   = x2+1;

  i4_bool lock_check = r1_software_render_buffer_is_locked;
  
  IDirectDrawSurface3 *temp_surf=0;
  r1_software_render_surface->QueryInterface(IID_IDirectDrawSurface3,(void **)&temp_surf);

  switch (r1_software_render_expand_type)
  {    
    case R1_COPY_1x1:      
    case R1_COPY_2x2:      
    case R1_COPY_1x1_SCANLINE_SKIP:
    if (lock_check) r1_software_unlock_render_buffer();    
    

    temp_surf->BltFast(x,
                       y,
                       dx5_common.get_surface(im),
                       &srcRect,
                       DDBLTFAST_NOCOLORKEY);
    
    if (lock_check) r1_software_lock_render_buffer();
    break;
  }

  temp_surf->Release();
}


void r1_software_render_window_class::draw(i4_draw_context_class &context)
{
  if (GetAsyncKeyState('L') & 1)
  {
    do_amd3d = !do_amd3d;
    
    i4_cpu_info_struct s;
    i4_get_cpu_info(&s);

    if ((s.cpu_flags & i4_cpu_info_struct::AMD3D)==0)
      do_amd3d = i4_F;      
    
    r1_software_class_instance.initialize_function_pointers(do_amd3d); //amd3d functions

    if (do_amd3d)
      i4_warning("amd3d optimizations on");
    else
      i4_warning("amd3d optimizations off");
  }

  pf_software_draw.start();

  r1_software_render_surface = surface;    
  r1_software_render_expand_type = expand_type;      

#ifdef DEBUG
  r1_software_class_instance.lock_cheat = i4_F;
#endif

  r1_software_lock_render_buffer();  

  if (r1_software_class_instance.lock_cheat)
    r1_software_unlock_render_buffer();

  r1_software_class_instance.width_compare  = width();
  r1_software_class_instance.height_compare = height();

  r1_software_class_instance.modify_features(R1_PERSPECTIVE_CORRECT, 1);
  r1_software_class_instance.modify_features(R1_SPANS, 1);

  clear_spans();

  r1_software_class_instance.context = &context;  
  r1_render_window_class::draw(context);  
  r1_software_class_instance.context = 0;

  //set this to false so that set_color_tint() actuall does work
  
  i4_bool spans_still_on = r1_software_class_instance.is_feature_on(R1_SPANS);

  r1_software_class_instance.modify_features(R1_SPANS, 0);

  pf_software_flush_spans.start();  
  flush_spans();  
  pf_software_flush_spans.stop();
  
  //clear this so that no funny business happens with leftover
  //color tint settings
  r1_software_class_instance.set_color_tint(0);

  //restore whatever state it was in
  r1_software_class_instance.modify_features(R1_SPANS, spans_still_on);

  //flush lines
  if (num_buffered_lines)
  {
    sw32 i;
    for (i=0;i<num_buffered_lines;i++)
    {  
      r1_software_class_instance.draw_line(software_lines[i].x0,
                                           software_lines[i].y0,
                                           software_lines[i].x1,
                                           software_lines[i].y1,
                                           software_lines[i].start_color);
    }
    num_buffered_lines=0;
  }
  
  if (!r1_software_class_instance.lock_cheat)
    r1_software_unlock_render_buffer();

  RECT		srcRect;
  RECT		destRect;

  pf_software_blt.start();

  
  IDirectDrawSurface3 *surface3=0;
  surface->QueryInterface(IID_IDirectDrawSurface3,(void **)&surface3);

  i4_rect_list_class::area_iter cl;
  i4_rect_list_class *clip=&context.clip;
  
  if (surface3)
  switch (expand_type)
  {
    case R1_COPY_1x1_SCANLINE_SKIP:
    case R1_COPY_1x1:      
      for (cl = clip->list.begin(); cl !=  clip->list.end(); ++cl)
      {
        srcRect.left   = cl->x1;
        srcRect.right  = cl->x2+1;
        srcRect.top    = cl->y1;
        srcRect.bottom = cl->y2+1;
        
        dx5_common.back_surface->BltFast(context.xoff+cl->x1,context.yoff+cl->y1,
                                         surface3,&srcRect,
                                         DDBLTFAST_NOCOLORKEY);
      }
      break;


    case R1_COPY_2x2:      
      for (cl = clip->list.begin(); cl !=  clip->list.end(); ++cl)
      {
        srcRect.left   = cl->x1/2;
        srcRect.right  = (cl->x2+2)/2;
        srcRect.top    = cl->y1/2;
        srcRect.bottom = (cl->y2+2)/2;
          
        destRect.left   = context.xoff+cl->x1;
        destRect.top    = context.yoff+cl->y1;
        destRect.right  = (context.xoff+cl->x2+1)&(~1);
        destRect.bottom = (context.yoff+cl->y2+1)&(~1);
              
        dx5_common.back_surface->Blt(&destRect, surface3, &srcRect, DDBLT_ASYNC | DDBLT_WAIT, NULL);      
      }
      break;
  }
  surface3->Release();
  
  pf_software_blt.stop();

  r1_software_render_surface = 0;
  pf_software_draw.stop();
}

r1_software_render_window_class::~r1_software_render_window_class()
{
  if (surface)
    surface->Release();

  surface=0;
}

void r1_software_class::clear_area(int x1, int y1, int x2, int y2, w32 color, float z)
{  
  if ((write_mask & R1_WRITE_COLOR) == 0) return;

  if (write_mask & R1_COMPARE_W)   // need to use spans to do this
  {
    w32             old_const_color = get_constant_color();  
    r1_alpha_type   old_alpha_mode  = get_alpha_mode();
    r1_shading_type old_shade_mode  = get_shade_mode();
  
    set_shading_mode(R1_CONSTANT_SHADING);
    set_alpha_mode(R1_ALPHA_DISABLED);  
  
    set_constant_color(color);

    disable_texture();

    r1_vert v[4];

    z -= 0.5;
    if (z<r1_near_clip_z)
      z = r1_near_clip_z;

    v[3].px=x1;   v[3].py=y1;    v[3].v.z = z; v[3].w = 1/z;
    v[2].px=x2+1; v[2].py=y1;    v[2].v.z = z; v[2].w = 1/z;
    v[1].px=x2+1; v[1].py=y2+1;  v[1].v.z = z; v[1].w = 1/z;
    v[0].px=x1;   v[0].py=y2+1;  v[0].v.z = z; v[0].w = 1/z;
  
    render_sprite(v);
  
    set_shading_mode(old_shade_mode);
    set_constant_color(old_const_color);
    set_alpha_mode(old_alpha_mode);
  }
  else
  {
    RECT dst;
    dst.left=x1;// + context->xoff;
    dst.right=x2/* + context->xoff*/+1;
    dst.top=y1;// + context->xoff;
    dst.bottom=y2/* + context->yoff*/+1;

    DDBLTFX fx;
    memset(&fx,0,sizeof(DDBLTFX));
    fx.dwSize = sizeof(DDBLTFX);
    
    fx.dwFillColor = color;
    
    i4_bool lock_check = r1_software_render_buffer_is_locked;

    if (lock_check) r1_software_unlock_render_buffer();
    
    r1_software_render_surface->Blt(&dst,NULL,NULL,DDBLT_WAIT | DDBLT_COLORFILL,&fx);
    
    if (lock_check) r1_software_lock_render_buffer();
  }

}

i4_image_class *r1_software_class::create_compatible_image(w16 w, w16 h)
{  
  i4_image_class *a = dx5_common.create_image(w,h, DX5_VRAM);
  if (!dx5_common.get_surface(a))
  {
    delete a;
    a = dx5_common.create_image(w,h, DX5_SYSTEM_RAM);
    if (!dx5_common.get_surface(a))
    {
      delete a;
      return NULL;
    }
  }
  
  return a;
}


