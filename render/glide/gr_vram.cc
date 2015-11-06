/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "video/glide/glide.h"
#include "glide/gr_vram.hh"
#include "time/profile.hh"
#include "r1_res.hh"

r1_glide_vram_class *r1_glide_vram_class_instance=0;

i4_profile_class pf_glide_install_vram("glide install vram texture");
i4_profile_class pf_glide_free_vram("glide free vram texture");

r1_glide_vram_class::r1_glide_vram_class(const i4_pal *pal)

  : r1_texture_manager_class(pal),finished_array(16,16)
{    
  tex_heap_man = 0;
  tex_heap_man_2 = 0;

  r1_glide_vram_class_instance = this;

  r1_glide_vram_class::init();

  min_texture_dimention = 1;

  max_texture_dimention = r1_max_texture_size;

  if (max_texture_dimention>256)
    max_texture_dimention=256;     // glide can't go higher than 256
    
  regular_format.red_mask   = 31 << 11;
  regular_format.green_mask = 63 << 5;
  regular_format.blue_mask  = 31;
  regular_format.alpha_mask = 0;
  regular_format.lookup     = 0;
  regular_format.calc_shift();
  regular_format.pixel_depth = I4_16BIT;
  
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
}

void r1_glide_vram_class::init()
{
  r1_texture_manager_class::init();

  int t_vram=grTexMaxAddress(GR_TMU0) - grTexMinAddress(GR_TMU0);
  
  int  heap1_size=t_vram > 2*1024*1024 ?  2*1024*1024 : t_vram;
  
  int heap2_size=t_vram - heap1_size;
  
  heap1_address=grTexMinAddress(GR_TMU0);
  heap2_address=heap1_address + heap1_size;
    
  tex_heap_man = new r1_texture_heap_class(heap1_size,
                                           heap1_address,
                                           sizeof(free_node),
                                           sizeof(used_node),
                                           &frame_count);

  /*  if (heap2_size>0)
    tex_heap_man_2 = new r1_texture_heap_class(heap2_size,
                                               heap2_address,
                                               sizeof(free_node),
                                               sizeof(used_node),
                                               &frame_count);

  */



  
  array_lock.lock();
  array_lock.unlock();

  bytes_loaded = 0;
  textures_loaded = 0;
}

void r1_glide_vram_class::uninit()
{  
  if (tex_heap_man)
  {
    delete tex_heap_man;
    tex_heap_man = 0;
  }
  if (tex_heap_man_2)
  {
    delete tex_heap_man_2;
    tex_heap_man_2 = 0;
  }

  
  r1_texture_manager_class::uninit();
}

i4_bool r1_glide_vram_class::immediate_mip_load(r1_mip_load_info *load_info)
{  
  if (textures_loaded > 0)
  {
    if (load_info->dest_mip->last_frame_used != -1)
    {
      load_info->error = R1_MIP_LOAD_BUSY;
      return i4_F;
    }
  }  
  pf_glide_install_vram.start();

  r1_miplevel_t *mip = load_info->dest_mip;

  sw32 actual_w,actual_h;

  w32  w_lod=GR_LOD_1, h_lod=GR_LOD_1;

  //determine the aspect ratio and level of detail
  for (actual_w=min_texture_dimention; actual_w<mip->width; actual_w*=2)
    w_lod--;

  for (actual_h=min_texture_dimention; actual_h<mip->height; actual_h*=2)
    h_lod--;

  w32 need_size = ((actual_w * actual_h * 2) + 7) & (~7); // sizes must be 8 byte alligned  

  used_node *new_used = (used_node *)tex_heap_man->alloc(need_size);
  if (!new_used && tex_heap_man_2)
    new_used = (used_node *)tex_heap_man_2->alloc(need_size);
    
  if (!new_used)
  {
    pf_glide_install_vram.stop();
    load_info->error = R1_MIP_LOAD_NO_ROOM;
    return i4_F;
  }

  free_node *f = (free_node *)new_used->node;

  //f is the node we'll use
  
  if (load_info->src_file)
  {
    load_info->src_file->read(scratch_texture,mip->width*mip->height*2);
  }
  else
  {

    i4_str *fn = r1_texture_id_to_filename(mip->entry->id, r1_get_decompressed_dir());
    i4_file_class *fp = i4_open(*fn);
    delete fn;
    
    fp->seek(mip->entry->file_offsets[mip->level]+8);
    fp->read(scratch_texture,mip->width*mip->height*2);

    delete fp;
  }      
  
  bytes_loaded += mip->width*mip->height*2;
  textures_loaded++;
  
  //fill these data fields w/the appropriate data
  f->mip = mip;
  mip->vram_handle = new_used;
  
  if (mip->last_frame_used != -1)
    mip->last_frame_used = frame_count;


  // setup the correct lod (level of detail) and aspect variables
  if (actual_w>=actual_h)
  {
    // uv correction for this texture because we made it pow2
    f->smul   = (i4_float)(mip->width)  / (i4_float)actual_w * 256.0;
    f->tmul   = (i4_float)(mip->height) / (i4_float)actual_w * 256.0;
    f->lod    = w_lod;
    f->aspect = GR_ASPECT_1x1 - (h_lod-w_lod);
  }
  else
  {
    f->smul   = (i4_float)(mip->width)  / (i4_float)actual_h * 256.0;
    f->tmul   = (i4_float)(mip->height) / (i4_float)actual_h * 256.0;
    f->lod    = h_lod;
    f->aspect = w_lod-h_lod+GR_ASPECT_1x1;
  }     

  GrTexInfo tex_load_info;

  tex_load_info.smallLod    = f->lod;
  tex_load_info.largeLod    = f->lod;
  tex_load_info.aspectRatio = f->aspect;
  tex_load_info.format      = GR_TEXFMT_RGB_565;
  
  // download the texture to the card
  grTexDownloadMipMapLevel(GR_TMU0,
                           f->start,
                           f->lod,
                           f->lod,
                           f->aspect,
                           GR_TEXFMT_RGB_565,
                           GR_MIPMAPLEVELMASK_BOTH,                           
                           scratch_texture);  

  pf_glide_install_vram.stop();
 
  return i4_T;
}

//typedef void (*async_callback)(w32 count, void *context);

void glide_async_callback(w32 count, void *context)
{  
  r1_glide_vram_class::used_node *u = (r1_glide_vram_class::used_node *)context;
  r1_glide_vram_class::free_node *f = (r1_glide_vram_class::free_node *)u->node;
  
  if (f->async_fp)
    delete f->async_fp;

  f->async_fp = 0;

  r1_glide_vram_class_instance->async_load_finished(u);  
}

void r1_glide_vram_class::async_load_finished(used_node *u)
{
  array_lock.lock();
  
  finished_array.add(u);  
  
  array_lock.unlock();
}

i4_bool r1_glide_vram_class::async_mip_load(r1_mip_load_info *load_info)
{
  //return immediate_mip_load(load_info);

  if (bytes_loaded > 32768 || textures_loaded > 16)
  {
    if (load_info->dest_mip->last_frame_used != -1)
    {
      load_info->error = R1_MIP_LOAD_BUSY;
      return i4_F;
    }
  }  
  
  pf_glide_install_vram.start();

  r1_miplevel_t *mip = load_info->dest_mip;

  sw32 actual_w,actual_h;

  w32  w_lod=GR_LOD_1, h_lod=GR_LOD_1;

  //determine the aspect ratio and level of detail
  for (actual_w=min_texture_dimention; actual_w<mip->width; actual_w*=2)
    w_lod--;

  for (actual_h=min_texture_dimention; actual_h<mip->height; actual_h*=2)
    h_lod--;

  w32 need_size = ((actual_w * actual_h * 2) + 7) & (~7); // sizes must be 8 byte alligned  

  used_node *new_used = (used_node *)tex_heap_man->alloc(need_size,R1_TEX_HEAP_DONT_LIST);
  if (!new_used && tex_heap_man_2)
    new_used = (used_node *)tex_heap_man_2->alloc(need_size,R1_TEX_HEAP_DONT_LIST);
  
  if (!new_used)
  {
    pf_glide_install_vram.stop();
    load_info->error = R1_MIP_LOAD_NO_ROOM;
    return i4_F;
  }

  //f is the node we'll use
  free_node *f = (free_node *)new_used->node;  
  
  //fill these data fields w/the appropriate data
  f->mip      = mip;
  f->data     = (w8 *)i4_malloc(need_size,"temp glide texture");
  f->async_fp = 0;

  mip->flags |= R1_MIPLEVEL_IS_LOADING;
  
  // setup the correct lod (level of detail) and aspect variables
  if (actual_w>=actual_h)
  {
    // uv correction for this texture because we made it pow2
    f->smul   = (i4_float)(mip->width)  / (i4_float)actual_w * 256.0;
    f->tmul   = (i4_float)(mip->height) / (i4_float)actual_w * 256.0;
    f->lod    = w_lod;
    f->aspect = GR_ASPECT_1x1 - (h_lod-w_lod);
  }
  else
  {
    f->smul   = (i4_float)(mip->width)  / (i4_float)actual_h * 256.0;
    f->tmul   = (i4_float)(mip->height) / (i4_float)actual_h * 256.0;
    f->lod    = h_lod;
    f->aspect = w_lod-h_lod+GR_ASPECT_1x1;
  }

  i4_bool async_worked;

  if (load_info->src_file)
  {
    texture_resolution_changed=i4_T;
    async_worked = load_info->src_file->async_read(f->data,mip->width*mip->height*2,glide_async_callback,new_used);  
  }
  else
  {
    i4_str *fn = r1_texture_id_to_filename(mip->entry->id, r1_get_decompressed_dir());
    
    i4_file_class *fp = i4_open(*fn,I4_READ | I4_NO_BUFFER | I4_SUPPORT_ASYNC);

    delete fn;
        
    if (!fp)    
      async_worked = i4_F;    
    else
    {
      f->async_fp = fp;

      fp->seek(mip->entry->file_offsets[mip->level]+8);

      texture_resolution_changed=i4_T;
      async_worked = fp->async_read(f->data,mip->width*mip->height*2,glide_async_callback,new_used);    
    }
  }    

  if (!async_worked)
  {
    if (f->async_fp)
      delete f->async_fp;

    if (f->data)
      i4_free(f->data);

    mip->flags &= (~R1_MIPLEVEL_IS_LOADING);

    if (new_used->node->start>=heap2_address)
      tex_heap_man_2->free((r1_tex_heap_used_node *)new_used);
    else
      tex_heap_man->free((r1_tex_heap_used_node *)new_used);
    
    load_info->error = R1_MIP_LOAD_BUSY;    
    pf_glide_install_vram.stop();
    return i4_F;
  }

  bytes_loaded += mip->width*mip->height*2;
  textures_loaded++;

  pf_glide_install_vram.stop(); 
  return i4_T;  
}

void r1_glide_vram_class::next_frame()
{
  r1_texture_manager_class::next_frame();

  sw32 i;    

  array_lock.lock();      

  for (i=0; i<finished_array.size(); i++)
  {
    used_node *u = finished_array[i];
    free_node *f = (free_node *)u->node;
    
    //this officially puts it in vram
    f->mip->vram_handle      = u;
    f->mip->flags           &= (~R1_MIPLEVEL_IS_LOADING);
    
    if (f->mip->last_frame_used != -1)
      f->mip->last_frame_used  = frame_count;

    if (u->node->start>=heap2_address)
      tex_heap_man_2->update_usage((r1_tex_heap_used_node *)u);
    else
      //this adds it into the list of used nodes
      tex_heap_man->update_usage((r1_tex_heap_used_node *)u);
    
    GrTexInfo tex_load_info;

    tex_load_info.smallLod    = f->lod;
    tex_load_info.largeLod    = f->lod;
    tex_load_info.aspectRatio = f->aspect;
    tex_load_info.format      = GR_TEXFMT_RGB_565;
  
    // download the texture to the card
    grTexDownloadMipMapLevel(GR_TMU0,
                             f->start,
                             f->lod,
                             f->lod,
                             f->aspect,
                             GR_TEXFMT_RGB_565,
                             GR_MIPMAPLEVELMASK_BOTH,
                             f->data);
                            
    i4_free(f->data);
    f->data = 0;
  }
  
  finished_array.clear();

  array_lock.unlock();

  if (tex_heap_man->needs_cleanup)
    tex_heap_man->free_really_old();

  if (tex_heap_man_2 && tex_heap_man_2->needs_cleanup)
    tex_heap_man_2->free_really_old();

  
  bytes_loaded = 0;
  textures_loaded = 0;
}

void r1_glide_vram_class::free_mip(r1_vram_handle_type vram_handle)
{
  pf_glide_free_vram.start();
  r1_tex_heap_used_node *n=(r1_tex_heap_used_node *)vram_handle;
  
  if (n->node->start>=heap2_address)
    tex_heap_man_2->free(n);
  else
    tex_heap_man->free(n);
  
  pf_glide_free_vram.stop();
}

void r1_glide_vram_class::select_texture(r1_vram_handle_type handle,
                                         float &smul, float &tmul,
                                         i4_bool holy, i4_bool alpha)
{  
  free_node *n = (free_node *)((used_node *)handle)->node;

  GrTexInfo tex_source_info;
  
  tex_source_info.smallLod    = n->lod;
  tex_source_info.largeLod    = n->lod;
  tex_source_info.aspectRatio = n->aspect;
  
  if (alpha)
    tex_source_info.format = GR_TEXFMT_ARGB_4444;
  else
  if (holy)
    tex_source_info.format = GR_TEXFMT_ARGB_1555;
  else
    tex_source_info.format = GR_TEXFMT_RGB_565;  
  
  smul = n->smul; 
  tmul = n->tmul;

  grTexSource(GR_TMU0,
              n->start,
              GR_MIPMAPLEVELMASK_BOTH,
              &tex_source_info);
}

r1_miplevel_t *r1_glide_vram_class::get_texture(r1_texture_handle handle,
                                                w32 frame_counter,
                                                sw32 desired_width,
                                                sw32 &w, sw32 &h)
{
  r1_miplevel_t *mip = r1_texture_manager_class::get_texture(handle,frame_counter,desired_width,w,h);

  if (mip)
  {
    r1_tex_heap_used_node *u = (r1_tex_heap_used_node *)mip->vram_handle;

    if (u)
    {
      if (u->node->start>=heap2_address)
        tex_heap_man_2->update_usage(u);
      else
        tex_heap_man->update_usage(u);
    }
  }

  return mip;    
}
