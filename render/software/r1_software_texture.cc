/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "software/r1_software_texture.hh"
#include "video/win32/dx5.hh"
#include "time/profile.hh"
#include "r1_res.hh"

i4_profile_class pf_software_install_vram("software install vram texture");
i4_profile_class pf_software_free_vram("software free vram texture");

r1_software_texture_class *r1_software_texture_class_instance=0;

r1_software_texture_class::r1_software_texture_class(const i4_pal *pal)
: r1_texture_manager_class(pal),finished_array(16,16)
{  
  r1_software_texture_class_instance = this;

  tex_heap_man = 0;
  texture_heap = 0;

  init();  
  
  min_texture_dimention = 1;
  max_texture_dimention = r1_max_texture_size;

  //setup the texture formats
  regular_format = pal->source;
  chroma_format  = regular_format;    
  
  alpha_format.red_mask   = 15 << 8;
  alpha_format.green_mask = 15 << 4;
  alpha_format.blue_mask  = 15;
  alpha_format.alpha_mask = 15 << 12;
  alpha_format.lookup     = 0;  
  alpha_format.calc_shift();
  alpha_format.pixel_depth = I4_16BIT;
}


void r1_software_texture_class::init()
{
  r1_texture_manager_class::init();
  
  int tex_mem_size = 1024*1024*8;
  if (i4_available()/4 < tex_mem_size)
    tex_mem_size=i4_available()/4;
  
  i4_warning("Software texture memory heap allocated: %d bytes", tex_mem_size);
  
  texture_heap = (w8 *)i4_malloc(tex_mem_size,"software texture memory heap");

  tex_heap_man = new r1_texture_heap_class(tex_mem_size,
                                           (w32)texture_heap,
                                           sizeof(free_node),
                                           sizeof(used_node),
                                           &frame_count);
  
  array_lock.lock();
  array_lock.unlock();
  
  bytes_loaded = 0;
  textures_loaded = 0;
}

void r1_software_texture_class::uninit()
{  
  if (tex_heap_man)
  {
    delete tex_heap_man;  
    tex_heap_man = 0;
  }

  if (texture_heap)
  {
    i4_warning("Software texture memory heap deallocated: 8 megs");

    i4_free(texture_heap);
    texture_heap = 0;
  }
  
  r1_texture_manager_class::uninit();
}

i4_bool r1_software_texture_class::immediate_mip_load(r1_mip_load_info *load_info)
{  
  if (textures_loaded > 0)
  {
    if (load_info->dest_mip->last_frame_used != -1)
    {
      load_info->error = R1_MIP_LOAD_BUSY;
      return i4_F;
    }
  }  
  pf_software_install_vram.start();  

  r1_miplevel_t *mip = load_info->dest_mip;  

  sw32 i,need_size;  
  
  need_size = mip->width * mip->height * 2;

  used_node *new_used = (used_node *)tex_heap_man->alloc(need_size);
  
  if (!new_used)
  {
    pf_software_install_vram.stop();
    load_info->error = R1_MIP_LOAD_NO_ROOM;
    return i4_F;
  }
    
  free_node *f = (free_node *)new_used->node;
  
  //f is the node we'll use  
  
  if (load_info->src_file)
  {
    load_info->src_file->read((w16 *)f->start,mip->width*mip->height*2);
  }
  else
  {
    i4_str *fn = r1_texture_id_to_filename(mip->entry->id, r1_get_decompressed_dir());    
    
    i4_file_class *fp = i4_open(*fn);

    delete fn;
    
    fp->seek(mip->entry->file_offsets[mip->level]+8);    
    fp->read((w16 *)f->start,mip->width*mip->height*2);        

    delete fp;
  }
  
  bytes_loaded += mip->width*mip->height*2;
  textures_loaded++;

  //load the texture
  f->mip = mip;
  mip->vram_handle = new_used;

  if (mip->last_frame_used != -1)
    mip->last_frame_used = frame_count;

  pf_software_install_vram.stop();

  return i4_T;
}

static i4_critical_section_class num_async_pending_lock;
static int num_async_pending=0;

r1_software_texture_class::~r1_software_texture_class()
{
  while (num_async_pending!=0)
    i4_sleep(0);


  uninit();
}
//typedef void (*async_callback)(w32 count, void *context);

void software_async_callback(w32 count, void *context)
{  
  r1_software_texture_class::used_node *u = (r1_software_texture_class::used_node *)context;
  r1_software_texture_class::free_node *f = (r1_software_texture_class::free_node *)u->node;
  
  if (f->async_fp)
    delete f->async_fp;

  f->async_fp = 0;  

  if (count != f->mip->width*f->mip->height*2)
    i4_warning("async texture read failure");

  r1_software_texture_class_instance->async_load_finished(u);  

  num_async_pending_lock.lock();
  num_async_pending--;
  num_async_pending_lock.unlock();
}

i4_bool r1_software_texture_class::async_mip_load(r1_mip_load_info *load_info)
{
  if (bytes_loaded > 32768 || textures_loaded > 16)
  {
    if (load_info->dest_mip->last_frame_used != -1)
    {    
      load_info->error = R1_MIP_LOAD_BUSY;
      return i4_F;
    }
  }

  pf_software_install_vram.start();

  sw32 i;

  r1_miplevel_t *mip = load_info->dest_mip;  
  
  w32 need_size = mip->width * mip->height * 2;

  used_node *new_used = (used_node *)tex_heap_man->alloc(need_size,R1_TEX_HEAP_DONT_LIST);
  
  if (!new_used)
  {
    pf_software_install_vram.stop();
    load_info->error = R1_MIP_LOAD_NO_ROOM;
    return i4_F;
  }
  
  //f is the node we'll use  
  free_node *f = (free_node *)new_used->node;    
  
  f->mip         = mip;
  f->async_fp    = 0;

  mip->flags |= R1_MIPLEVEL_IS_LOADING;

  i4_bool async_worked;

  num_async_pending_lock.lock();
  num_async_pending++;
  num_async_pending_lock.unlock();

  if (load_info->src_file)
  {
    async_worked = load_info->src_file->async_read((w16 *)f->start,mip->width*mip->height*2,software_async_callback,new_used);    
  }
  else
  {
    i4_str *fn = r1_texture_id_to_filename(mip->entry->id,r1_get_decompressed_dir());    
    
    i4_file_class *fp = i4_open(*fn,I4_READ | I4_NO_BUFFER | I4_SUPPORT_ASYNC);

    delete fn;
        
    if (!fp)
      async_worked = i4_F;
    else
    {
      f->async_fp = fp;

      fp->seek(mip->entry->file_offsets[mip->level]+8);

      async_worked = fp->async_read((w16 *)f->start,mip->width*mip->height*2,software_async_callback,new_used);
    }
  }
  
  if (!async_worked)
  {  
    if (f->async_fp)
      delete f->async_fp;
    
    mip->flags &= (~R1_MIPLEVEL_IS_LOADING);    
    tex_heap_man->free((r1_tex_heap_used_node *)new_used);
    
    load_info->error = R1_MIP_LOAD_BUSY;    
    pf_software_install_vram.stop();

    num_async_pending_lock.lock();
    num_async_pending--;
    num_async_pending_lock.unlock();

    return i4_F;
  }

  bytes_loaded += mip->width*mip->height*2;
  textures_loaded++;

  pf_software_install_vram.stop();

  return i4_T;
}

void r1_software_texture_class::async_load_finished(used_node *u)
{
  array_lock.lock();
  
  finished_array.add(u);  
  
  array_lock.unlock();
}

void r1_software_texture_class::next_frame()
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

    //this adds it into the list of used nodes
    tex_heap_man->update_usage((r1_tex_heap_used_node *)u);
  }  

  finished_array.clear();

  array_lock.unlock();

  if (tex_heap_man->needs_cleanup)
  {
    tex_heap_man->free_really_old();
  }

  bytes_loaded = 0;
  textures_loaded = 0;
}


void r1_software_texture_class::free_mip(r1_vram_handle_type vram_handle)
{
  pf_software_free_vram.start();

  tex_heap_man->free((r1_tex_heap_used_node *)vram_handle);
  
  pf_software_free_vram.stop();
}

void r1_software_texture_class::select_texture(r1_vram_handle_type handle,
                                         float &smul, float &tmul)
{
  r1_tex_heap_used_node *u = (r1_tex_heap_used_node *)handle;

  free_node *f = (free_node *)u->node;

  smul = f->mip->width;
  tmul = f->mip->height;  
}

r1_miplevel_t *r1_software_texture_class::get_texture(r1_texture_handle handle,
                                                      w32 frame_counter,
                                                      sw32 desired_width,
                                                      sw32 &w, sw32 &h)
{
  r1_miplevel_t *mip = r1_texture_manager_class::get_texture(handle,frame_counter,desired_width,w,h);
  if (!mip)
    return 0;

  r1_tex_heap_used_node *u = (r1_tex_heap_used_node *)mip->vram_handle;  
  
  if (u)
    tex_heap_man->update_usage(u);

  return mip;
}
