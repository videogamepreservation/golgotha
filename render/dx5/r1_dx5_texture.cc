/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "dx5/r1_dx5_texture.hh"

#include "video/win32/dx5.hh"
#include "video/win32/dx5_error.hh"
#include "time/profile.hh"
#include "r1_res.hh"

i4_profile_class pf_dx5_install_vram("dx5 install vram texture");
i4_profile_class pf_dx5_free_vram("dx5 free vram texture");
i4_profile_class pf_dx5_select_texture("dx5 select_texture");
i4_profile_class pf_dx5_texture_load("dx5 texture Load()");

DDPIXELFORMAT dd_fmt_565;
DDPIXELFORMAT dd_fmt_1555;
DDPIXELFORMAT dd_fmt_4444;

r1_dx5_texture_class *r1_dx5_texture_class_instance=0;

LPDDSURFACEDESC first_tex_format=0;

HRESULT WINAPI texture_enum(LPDDSURFACEDESC lpddsd, LPVOID _regular_format)
{
  if ((lpddsd->ddpfPixelFormat.dwFlags & DDPF_ALPHAPIXELS)==0 &&
       lpddsd->ddpfPixelFormat.dwRGBBitCount==16)
  {
    //find the best non-alpha surface format
    i4_pixel_format *regular_format = (i4_pixel_format *)_regular_format;

    sw32 total_new_bits = lpddsd->ddpfPixelFormat.dwRBitMask +
                          lpddsd->ddpfPixelFormat.dwGBitMask +
                          lpddsd->ddpfPixelFormat.dwBBitMask;

    sw32 total_reg_bits = regular_format->red_mask   + 
                          regular_format->green_mask +
                          regular_format->blue_mask;

    if (total_new_bits > total_reg_bits)
    {
      regular_format->red_mask   = lpddsd->ddpfPixelFormat.dwRBitMask;
      regular_format->green_mask = lpddsd->ddpfPixelFormat.dwGBitMask;
      regular_format->blue_mask  = lpddsd->ddpfPixelFormat.dwBBitMask;
      regular_format->alpha_mask = 0;
      regular_format->lookup     = 0;
      regular_format->calc_shift();
      regular_format->pixel_depth = I4_16BIT;
    }
  }

  if (first_tex_format)
  {
    if (lpddsd==first_tex_format) 
      return D3DENUMRET_CANCEL;
  }
  else
    first_tex_format = lpddsd;

  return D3DENUMRET_OK;
}

r1_dx5_texture_class::r1_dx5_texture_class(const i4_pal *pal)
: r1_texture_manager_class(pal),finished_array(16,16)
{
  r1_dx5_texture_class_instance = this;
  
  tex_no_heap = 0;

  //find out how much texture memory we have and show it to anyone debugging
  DDSCAPS tcaps;
  tcaps.dwCaps = DDSCAPS_TEXTURE;  
  w32 total_free, texture_free;
  dx5_common.ddraw->GetAvailableVidMem(&tcaps, &total_free, &texture_free);  
  i4_warning("Total Display Memory Free: %d",total_free);
  i4_warning("Total Display Texture Memory Free: %d",texture_free);
    
  memset(&regular_format,0,sizeof(i4_pixel_format));

  if (r1_dx5_class_instance.d3d_device->EnumTextureFormats(texture_enum,(LPVOID)&regular_format) != D3D_OK)
    i4_error("Couldn't determine Direct3D Texture Formats");
  
  chroma_format.red_mask   = 31 << 10;
  chroma_format.green_mask = 31 << 5;
  chroma_format.blue_mask  = 31;
  chroma_format.alpha_mask = 1 << 15;
  chroma_format.lookup     = 0;  
  chroma_format.calc_shift();
  chroma_format.pixel_depth = I4_16BIT;
  
  alpha_format.red_mask   = 15 << 8;
  alpha_format.green_mask = 15 << 4;
  alpha_format.blue_mask  = 15;
  alpha_format.alpha_mask = 15 << 12;
  alpha_format.lookup     = 0;  
  alpha_format.calc_shift();
  alpha_format.pixel_depth = I4_16BIT;

  memset(&dd_fmt_565,0,sizeof(DDPIXELFORMAT));
  dd_fmt_565.dwSize = sizeof(DDPIXELFORMAT);
  dd_fmt_565.dwFlags = DDPF_RGB;
  dd_fmt_565.dwRGBBitCount = 16;
  dd_fmt_565.dwRBitMask = regular_format.red_mask;
  dd_fmt_565.dwGBitMask = regular_format.green_mask;
  dd_fmt_565.dwBBitMask = regular_format.blue_mask;

  memset(&dd_fmt_1555,0,sizeof(DDPIXELFORMAT));
  dd_fmt_1555.dwSize = sizeof(DDPIXELFORMAT);
  dd_fmt_1555.dwFlags = DDPF_RGB | DDPF_ALPHAPIXELS;
  dd_fmt_1555.dwRGBBitCount     = 16;    
  dd_fmt_1555.dwRBitMask        = 31 << 10;
  dd_fmt_1555.dwGBitMask        = 31 << 5;
  dd_fmt_1555.dwBBitMask        = 31;
  dd_fmt_1555.dwRGBAlphaBitMask = 1 << 15;

  memset(&dd_fmt_4444,0,sizeof(DDPIXELFORMAT));
  dd_fmt_4444.dwSize = sizeof(DDPIXELFORMAT);
  dd_fmt_4444.dwFlags = DDPF_RGB | DDPF_ALPHAPIXELS;
  dd_fmt_4444.dwRGBBitCount     = 16;    
  dd_fmt_4444.dwRBitMask        = 15 << 8;
  dd_fmt_4444.dwGBitMask        = 15 << 4;
  dd_fmt_4444.dwBBitMask        = 15;
  dd_fmt_4444.dwRGBAlphaBitMask = 15 << 12;

  min_texture_dimention = 0;

  max_texture_dimention = r1_max_texture_size;

  if (max_texture_dimention>256)
    max_texture_dimention=256;     // don't allow d3d to go above 256

  square_textures = r1_dx5_class_instance.needs_square_textures;

  init();
}

void r1_dx5_texture_class::init()
{  
  r1_texture_manager_class::init();
  
  tex_no_heap  = new r1_texture_no_heap_class(this,sizeof(used_node),(w32 *)&frame_count);
  
  array_lock.lock();
  array_lock.unlock();

  bytes_loaded = 0;
  textures_loaded = 0;
}

void r1_dx5_texture_class::uninit()
{  
  if (tex_no_heap)
  {
    delete tex_no_heap;
    tex_no_heap = 0;
  }

  r1_texture_manager_class::uninit();
}


#define R1_DX5_SURFACE_HOLY         1
#define R1_DX5_SURFACE_ALPHATEXTURE 2
#define R1_DX5_SURFACE_VRAM         4


IDirectDrawSurface3 *dx5_make_surface(sw32 w, sw32 h, w8 flags)
{
  DDSURFACEDESC ddsd;
  memset(&ddsd,0,sizeof(DDSURFACEDESC));
  ddsd.dwSize = sizeof(DDSURFACEDESC);  
  ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
  ddsd.dwWidth  = w;
  ddsd.dwHeight = h;

  if (flags & R1_DX5_SURFACE_VRAM)
  {
    ddsd.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_ALLOCONLOAD;
  
    if (r1_dx5_class_instance.hardware_tmapping)
      ddsd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
    else
      ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
  }
  else
    ddsd.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_SYSTEMMEMORY;

  if (flags & R1_DX5_SURFACE_HOLY)
    ddsd.ddpfPixelFormat = dd_fmt_1555;
  else
  if (flags & R1_DX5_SURFACE_ALPHATEXTURE)
    ddsd.ddpfPixelFormat = dd_fmt_4444;
  else
    ddsd.ddpfPixelFormat = dd_fmt_565;
  
  IDirectDrawSurface  *return_surface  = 0;
  IDirectDrawSurface3 *return_surface3 = 0;
  
  HRESULT res = dx5_common.ddraw->CreateSurface(&ddsd,&return_surface,0);

  if (!i4_dx5_check(res))
  {
#ifdef DEBUG
    i4_warning("r1_dx5_texture::dx5_make_surface failed.");
#endif
    return_surface = 0;
  }
  else
  {    
    res = return_surface->QueryInterface(IID_IDirectDrawSurface3,(void **)&return_surface3);    

    if (!i4_dx5_check(res))
    {
#ifdef DEBUG
      i4_warning("r1_dx5_texture::dx5_make_surface failed.");
#endif
      return_surface3 = 0;
    }
    
    return_surface->Release();
  }

  return return_surface3;
}

r1_dx5_texture_class::used_node *r1_dx5_texture_class::make_surfaces_for_load(
                                                     IDirectDrawSurface3 *&vram_surface3,
                                                     IDirectDrawSurface3 *&system_surface3,
                                                     r1_mip_load_info *&load_info,
                                                     sw32 actual_w, sw32 actual_h,
                                                     w8 node_alloc_flags)
{
  r1_miplevel_t *mip = load_info->dest_mip;

  sw32 need_size = actual_w*actual_h*2;

  w8 create_flags = 0;
  
  if (mip->entry->is_transparent())
    create_flags = R1_DX5_SURFACE_HOLY;
  else
  if (mip->entry->is_alphatexture())
    create_flags = R1_DX5_SURFACE_ALPHATEXTURE;
  
  system_surface3 = dx5_make_surface(actual_w, actual_h, create_flags);
    
  if (!system_surface3)
  {    
    load_info->error = R1_MIP_LOAD_NO_ROOM;    
    return 0;
  }

  create_flags |= R1_DX5_SURFACE_VRAM;

  vram_surface3 = dx5_make_surface(actual_w, actual_h, create_flags);

  used_node *new_used = 0;

  if (vram_surface3)
  {
    new_used = (used_node *)tex_no_heap->alloc(node_alloc_flags);
  }
  else  
  {
    used_node *u = (used_node *)tex_no_heap->alloc_from_used(mip->width*mip->height*2,node_alloc_flags);
    
    if (!u)
    {
      system_surface3->Release();
      system_surface3 = 0;

      load_info->error = R1_MIP_LOAD_NO_ROOM;      
      return 0;
    }
        
    vram_surface3 = dx5_make_surface(actual_w, actual_h, create_flags);
    
    if (!vram_surface3)
    {
      system_surface3->Release();
      system_surface3 = 0;
      
      tex_no_heap->missed_one(mip->width*mip->height*2);
      
      load_info->error = R1_MIP_LOAD_NO_ROOM;      
      return 0;
    }

    new_used = u;
  }
  
  if (!new_used)
  {
    i4_warning("couldn't alloc used_node");
    if (system_surface3)
      system_surface3->Release();
    if (vram_surface3)
      vram_surface3->Release();
    
    system_surface3 = 0;
    vram_surface3   = 0;

    load_info->error = R1_MIP_LOAD_NO_ROOM;    
    return 0;
  }  

  return new_used;
}


i4_bool r1_dx5_texture_class::immediate_mip_load(r1_mip_load_info *load_info)
{  
  if (textures_loaded > 0)
  {
    if (load_info->dest_mip->last_frame_used!=-1)
    {
      load_info->error = R1_MIP_LOAD_BUSY;
      return i4_F;
    }
  }

  pf_dx5_install_vram.start();

  IDirectDrawSurface3 *vram_surface   = 0;
  IDirectDrawSurface3 *system_surface = 0;

  sw32 i,j,actual_w,actual_h;
    
  r1_miplevel_t *mip = load_info->dest_mip;

  for (actual_w = 1; actual_w < mip->width;  actual_w*=2); // these need to be power of 2
  for (actual_h = 1; actual_h < mip->height; actual_h*=2);
    
  used_node *new_used = make_surfaces_for_load(vram_surface,system_surface,load_info,actual_w,actual_h);
  
  if (!new_used)
  {
    pf_dx5_install_vram.stop();
    return i4_F;
  }

  DDSURFACEDESC ddsd;
  memset(&ddsd,0,sizeof(DDSURFACEDESC));
  ddsd.dwSize = sizeof(DDSURFACEDESC);

  // Lock the surface so we can get the memory address where it is
  system_surface->Lock(0, &ddsd, DDLOCK_WAIT | DDLOCK_WRITEONLY | DDLOCK_NOSYSLOCK, 0);  

  w8   *texture_ptr = (w8 *)ddsd.lpSurface;
  sw32  bpl         = mip->width*2;

  i4_bool segmented_load = i4_F;
    
  if (load_info->src_file)
  {
    if (!segmented_load)
      load_info->src_file->read(texture_ptr,mip->width*mip->height*2);
    else
    {
      for (i=0; i<mip->height; i++)
      {
        load_info->src_file->read(texture_ptr,bpl);
        texture_ptr += ddsd.lPitch;
      }
    }
  }
  else
  {
    i4_str *fn = r1_texture_id_to_filename(mip->entry->id,r1_get_decompressed_dir());    
    
    i4_file_class *fp = i4_open(*fn,I4_READ | I4_NO_BUFFER);

    delete fn;
    
    fp->seek(mip->entry->file_offsets[mip->level]+8);
    
    if (!segmented_load)
      fp->read(texture_ptr,mip->width*mip->height*2);
    else
    {
      for (i=0; i<mip->height; i++)
      {
        fp->read(texture_ptr,bpl);
        texture_ptr += ddsd.lPitch;
      }
    }

    delete fp;
  }   

  system_surface->Unlock(0);

  mip->vram_handle = new_used;

  if (mip->last_frame_used != -1)
    mip->last_frame_used = frame_count;  

  new_used->vram_surface   = vram_surface;
  new_used->system_surface = 0;
  new_used->mip            = mip;
  new_used->async_fp       = 0;
  new_used->data           = 0;

  IDirect3DTexture2 *system_texture, *vram_texture;
  
  system_surface->QueryInterface(IID_IDirect3DTexture2,(void **)&system_texture);
  vram_surface->QueryInterface(IID_IDirect3DTexture2,(void **)&vram_texture);

  pf_dx5_texture_load.start();
  if (vram_texture->Load(system_texture) != D3D_OK)
    i4_warning("r1_dx5_texture:: Load() failed");
  pf_dx5_texture_load.stop();
  
  vram_texture->GetHandle(r1_dx5_class_instance.d3d_device, &new_used->texture_handle);
   
  vram_texture->Release();
  system_texture->Release();

  system_surface->Release();
  
  bytes_loaded += mip->width*mip->height*2;
  textures_loaded++;

  pf_dx5_install_vram.stop();
  return i4_T;
}

void dx5_async_callback(w32 count, void *context)
{
  r1_dx5_texture_class::used_node *u = (r1_dx5_texture_class::used_node *)context;

  if (u->async_fp)
  {
    delete u->async_fp;
    u->async_fp = 0;
  }

  r1_dx5_texture_class_instance->async_load_finished(u);
}

void r1_dx5_texture_class::async_load_finished(used_node *u)
{
  array_lock.lock();
  
  finished_array.add(u);
  
  array_lock.unlock();
}

i4_bool r1_dx5_texture_class::async_mip_load(r1_mip_load_info *load_info)
{  
  if (bytes_loaded > 32768 || textures_loaded > 16)
  {
    if (load_info->dest_mip->last_frame_used!=-1)
    {
      load_info->error = R1_MIP_LOAD_BUSY;
      return i4_F;
    }
  }

  pf_dx5_install_vram.start();  

  IDirectDrawSurface3 *vram_surface   = 0;
  IDirectDrawSurface3 *system_surface = 0;

  sw32 i,j,actual_w,actual_h;
    
  r1_miplevel_t *mip = load_info->dest_mip;

  //for (actual_w = 1; actual_w < mip->width;  actual_w*=2); // these need to be power of 2
  //for (actual_h = 1; actual_h < mip->height; actual_h*=2);
  
  //all textures are power of 2 now
  actual_w = mip->width;
  actual_h = mip->height;
    
  used_node *new_used = make_surfaces_for_load(vram_surface,system_surface,load_info,actual_w,actual_h,R1_TEX_NO_HEAP_DONT_LIST);
  
  if (!new_used)
  {
    pf_dx5_install_vram.stop();
    return i4_F;
  }

  new_used->mip      = mip;
  new_used->data     = (w8 *)i4_malloc(mip->width*mip->height*2,"dx5 async load alloc");
  new_used->async_fp = 0;  
  
  new_used->vram_surface   = vram_surface;
  new_used->system_surface = system_surface;    
  new_used->texture_handle = 0;

  mip->vram_handle  = 0;
  mip->flags       |= R1_MIPLEVEL_IS_LOADING;

  i4_bool async_worked = i4_F;
  
  if (load_info->src_file)
  {
    async_worked = load_info->src_file->async_read(new_used->data,
                                                   mip->width*mip->height*2,
                                                   dx5_async_callback,
                                                   new_used);
  }
  else
  {
    i4_str *fn = r1_texture_id_to_filename(mip->entry->id,r1_get_decompressed_dir());    
    
    i4_file_class *fp = i4_open(*fn,I4_READ | I4_NO_BUFFER);

    delete fn;
    
    new_used->async_fp = fp;

    fp->seek(mip->entry->file_offsets[mip->level]+8);
    
    async_worked = fp->async_read(new_used->data,
                                  mip->width*mip->height*2,
                                  dx5_async_callback,
                                  new_used);
  }
  
  if (!async_worked)
  {
    mip->flags &= (~R1_MIPLEVEL_IS_LOADING);
    free_mip(new_used);    
    
    load_info->error = R1_MIP_LOAD_BUSY;    
    pf_dx5_install_vram.stop();
    return i4_F;
  }  
  
  bytes_loaded += mip->width*mip->height*2;
  textures_loaded++;

  pf_dx5_install_vram.stop();
  return i4_T;
}

void r1_dx5_texture_class::free_mip(void *vram_handle)
{
  pf_dx5_free_vram.start();
  used_node *u = (used_node *)vram_handle;

  if (u->system_surface)
  {
    u->system_surface->Release();
    u->system_surface = 0;
  }
    
  if (u->vram_surface)
  {
    u->vram_surface->Release();
    u->vram_surface = 0;
  }
    
  if (u->async_fp)
  {
    delete u->async_fp;
    u->async_fp = 0;
  }
  
  if (u->data)
  {
    i4_free(u->data);
    u->data = 0;
  }    

  tex_no_heap->free((r1_tex_no_heap_used_node *)u);
  
  pf_dx5_free_vram.stop();
}

void r1_dx5_texture_class::select_texture(r1_local_texture_handle_type handle,
                                          float &smul, float &tmul)
{
  pf_dx5_select_texture.start();

  used_node *u = (used_node *)handle;
  
  if (r1_dx5_class_instance.d3d_device->SetRenderState(D3DRENDERSTATE_TEXTUREHANDLE,
                                                   u->texture_handle) != D3D_OK)
  {
    i4_warning("select_texture failed");
  }

  smul = 1.0;//n->smul;
  tmul = 1.0;//n->tmul; 

  pf_dx5_select_texture.stop();
}

r1_miplevel_t *r1_dx5_texture_class::get_texture(r1_texture_handle handle,
                                                 w32 frame_counter,
                                                 sw32 desired_width,
                                                 sw32 &w, sw32 &h)
{
  r1_miplevel_t *mip = r1_texture_manager_class::get_texture(handle,frame_counter,desired_width,w,h);
  
  used_node *u = (used_node *)mip->vram_handle;  

  tex_no_heap->update_usage((r1_tex_no_heap_used_node *)u);
  
  return mip;
}

void r1_dx5_texture_class::next_frame()
{
  r1_texture_manager_class::next_frame();
  
  sw32 i,j;
  array_lock.lock();

  for (i=0; i<finished_array.size(); i++)
  {
    used_node *u = finished_array[i];    
    
    //this officially puts it in vram
    u->mip->vram_handle  = u;
    u->mip->flags       &= (~R1_MIPLEVEL_IS_LOADING);
    
    if (u->mip->last_frame_used != -1)
      u->mip->last_frame_used  = frame_count;
    
    //this adds it into the list of used nodes
    tex_no_heap->update_usage((r1_tex_no_heap_used_node *)u);
                                    
    DDSURFACEDESC ddsd;
    memset(&ddsd,0,sizeof(DDSURFACEDESC));
    ddsd.dwSize = sizeof(DDSURFACEDESC);

    u->system_surface->Lock(0, &ddsd, DDLOCK_WAIT | DDLOCK_WRITEONLY | DDLOCK_NOSYSLOCK, 0);    
    
    sw32 bpl = u->mip->width*2;

    w8 *texture_ptr = (w8 *)ddsd.lpSurface;

    if (0)//ddsd.lPitch > bpl)
    {      
      for (j=0; j<u->mip->height; j++)
      {
        memcpy(texture_ptr,u->data,bpl);
        texture_ptr += ddsd.lPitch;
      }
    }
    else
      memcpy(texture_ptr,u->data,u->mip->width*u->mip->height*2);    

    u->system_surface->Unlock(0);

    i4_free(u->data);
    u->data = 0;    

    IDirect3DTexture2  *system_texture, *vram_texture;        

    u->system_surface->QueryInterface(IID_IDirect3DTexture2,(void **)&system_texture);
    u->vram_surface->QueryInterface(IID_IDirect3DTexture2,(void **)&vram_texture);

    pf_dx5_texture_load.start();
    if (vram_texture->Load(system_texture) != D3D_OK)
      i4_warning("d3d:load texture failed");
    pf_dx5_texture_load.stop();

    vram_texture->GetHandle(r1_dx5_class_instance.d3d_device, &u->texture_handle);

    vram_texture->Release();
    system_texture->Release();    
    
    u->system_surface->Release();
    u->system_surface = 0;    
  }
  finished_array.clear();

  array_lock.unlock();
  
  if (tex_no_heap->needs_cleanup)
  {
    tex_no_heap->free_really_old();
  }

  bytes_loaded = 0;
  textures_loaded = 0;
}




