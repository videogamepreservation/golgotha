/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "tmanage.hh"
#include "memory/malloc.hh"
#include "error/error.hh"
#include "checksum/checksum.hh"
#include "image/color.hh"
#include "string/str_checksum.hh"
#include "error/alert.hh"
#include "time/profile.hh"
#include "string/string.hh"
#include "status/status.hh"
#include "file/ram_file.hh"
#include "tex_cache.hh"
#include "file/file.hh"
#include "search.hh"
#include "r1_res.hh"
#include "image/image.hh"
#include "image/context.hh"
#include "time/profile.hh"

i4_profile_class pf_register_texture("register_texture");

w32 R1_CHROMA_COLOR = (254<<16) | (2<<8) | (166);

i4_profile_class pf_get_texture("tmanage get texture");

r1_texture_manager_class *r1_texture_manager=0;

r1_texture_manager_class::r1_texture_manager_class(const i4_pal *pal)
  : pal(pal),
    registered_tnames(0,128)
{
  textures_loaded=i4_F;
  square_textures = i4_F;
}

void r1_texture_manager_class::init()
{
  texture_load_toggle = i4_T;
  texture_resolution_changed = i4_F;
  textures_loaded=i4_F;
  
  frame_count       = 0;
  entries           = 0;
  tanims            = 0;

  total_tanims   = 0;
  total_textures = 0;

  registered_tnames.clear();
}

char *r1_texture_manager_class::get_texture_name(r1_texture_handle handle)
{
  if (handle<registered_tnames.size())
    return registered_tnames[handle].name;
  else
    return "invalid";
}


void r1_texture_manager_class::uninit()
{
  sw32 i,j;

  for (i=1; i<total_textures; i++)
  {    
    for (j=0; j<R1_MAX_MIP_LEVELS; j++)
    {
      r1_miplevel_t *m = (*entries)[i].mipmaps[j];
      if (m)
      {
        if(m->vram_handle)
        {
          free_mip(m->vram_handle);
          m->vram_handle = 0;
        }
        delete m;
        m = 0;
      }
    }
//    delete (*entries)[i];
  }
  
  if (entries)
  {
    delete entries;
    entries = 0;
  }
  
  registered_tnames.uninit();

  texture_load_toggle = i4_T;
  texture_resolution_changed=i4_F;

  frame_count = 0;
  tanims      = 0;

  total_tanims   = 0;
  total_textures = 0;
  textures_loaded = i4_F;
}

void r1_texture_manager_class::reset()
{
  uninit();
  init();
}

int entry_compare(const r1_texture_entry_struct *a, const r1_texture_entry_struct *b)
{
  if (a->id<b->id)
    return -1;
  else if (a->id>b->id)
    return 1;
  else return 0;
}

r1_texture_handle r1_texture_manager_class::find_texture(w32 id)
{
  sw32 lo=1,hi=total_textures-1,mid;

  if (!entries)
    return 0;

  r1_texture_entry_struct search_entry;
  search_entry.id = id;

  sw32 location = entries->binary_search(&search_entry,entry_compare);
  
  if (location==-1)
    return 0;
  
  return location;
}


void r1_texture_manager_class::matchup_textures()
{
  sw32 i;
  for (i=0; i<registered_tnames.size(); i++)
  {
    r1_texture_handle h = find_texture(registered_tnames[i].id);
    
    if (h)
      registered_tnames[i].handle = h;
    else
    {
      registered_tnames[i].handle = 0;
      i4_warning("Texture matchup failed: %s",registered_tnames[i].name);
    }
  }

  
}

int name_compare(const r1_texture_matchup_struct *a, const r1_texture_matchup_struct *b)
{
  return strcmp(a->name, b->name);
}


r1_texture_handle r1_texture_manager_class::register_texture(const i4_const_str &tname,
                                                             const i4_const_str &error_string,
                                                             i4_bool *has_been_loaded)
{
  pf_register_texture.start();

  if (textures_loaded)
    i4_error("textures already loaded");

  int found = -1;


  r1_texture_matchup_struct t;
  i4_os_string(tname, t.name, 128);
  t.id     = r1_get_texture_id(tname);
  t.handle = 0;
  t.left   = -1;
  t.right  = -1;

  if (has_been_loaded)
    *has_been_loaded=0;

  // search the tree for the name
  if (registered_tnames.size())
  {
    int root=0, parent=-1;
    while (found<0)
    {
      parent=root;

      int res=strcmp(registered_tnames[root].name, t.name);
      if (res==0)
      {
        if (has_been_loaded)
          *has_been_loaded=1;
        found = root;
      }
      else if (res<0)
        root=registered_tnames[root].left;
      else
        root=registered_tnames[root].right;

      if (root==-1)
      {
        if (res<0)
          found = registered_tnames[parent].left=registered_tnames.add(t);
        else
          found = registered_tnames[parent].right=registered_tnames.add(t);
      }
    }
  }
  else
    found=registered_tnames.add(t);

  pf_register_texture.stop();
  return found;
}

int w32_compare(const w32 *a, const w32 *b)
{
  if (*a < *b)
    return -1;
  else
  if (*a > *b)
    return 1;
  else
    return 0;  
}

w32 r1_get_file_id(const i4_const_str &fname)
{
  int x;
  char st[30], *s;
  s=st;
  
  i4_const_str::iterator l=fname.begin();
  while (l!=fname.end() && l.get().ascii_value()!='.')
  {
    *(s++)=l.get().ascii_value();
    ++l;
  }

  *s=0;

  if (sscanf(st,"%x",&x))
    return x;
  else return 0;   
}

void r1_texture_manager_class::toggle_texture_loading()
{
  texture_load_toggle = (i4_bool)(!texture_load_toggle);
}

r1_miplevel_t *r1_texture_manager_class::get_texture(r1_texture_handle _handle,
                                                     w32 frame_counter,
                                                     sw32 desired_width,
                                                     sw32 &ret_w, sw32 &ret_h)
{
  pf_get_texture.start();  

  if (desired_width > max_texture_dimention)
    desired_width = max_texture_dimention;

  r1_texture_entry_struct *e;
  
  r1_texture_handle handle = registered_tnames[_handle].handle;
  if (handle==0)
    i4_error("get_texture called with invalid handle");

  if (handle>=total_textures)
    i4_error("asking for bad texture");

  if (handle<0)  // this is an animation
  {
    r1_texture_animation_entry_struct *ta=tanims+(-handle-1);
    handle=ta->frames[frame_counter%ta->total_frames];
    if (!handle)
    {
      pf_get_texture.stop();
      return 0;
    }
  }
  
  e = &(*entries)[handle];
  
  sw32 i = 0;  
  sw32 highest_resident = -1;
  sw32 need_to_use = 0;
  
  r1_miplevel_t *t = 0;

  //find the highest resident mip. can be precalculated and updated ?
  for (i=0; i<R1_MAX_MIP_LEVELS; i++)
  {
    t = e->mipmaps[i];
    
    if (t)
    {
      if (t->width >= desired_width)
        need_to_use = i;

      if (t->vram_handle)    
      {        
        if (t->width < desired_width)
        {
          //if there were no more higher than this,
          //this is the best we can do
          if (highest_resident==-1)
            highest_resident = i;
          break;
        }
        else
          highest_resident = i;
      }            
    }
    else
    {
      break;
    }  
  }  
  
  if (highest_resident==-1)
  {
    i4_warning("No textures resident for handle %d", handle);
    return 0;    
  }  
  
  r1_miplevel_t *return_mip = e->mipmaps[highest_resident];
  
  if (!return_mip)
    i4_error("catastophic error - r1_texture_manager_class::return_mip is 0");

  if (!return_mip->vram_handle)
    i4_warning("returned a mip w/no vram handle");      

  //this is to prevent the load below from deleting this miplevel from vram
  sw32 old_last_frame = return_mip->last_frame_used;
  return_mip->last_frame_used = frame_count;
  
  //loading information 
  r1_mip_load_info load_info;

  //i4_warning("texture loading is OFF");
  if (texture_load_toggle && (highest_resident != need_to_use) && e->mipmaps[need_to_use])
  {
    load_info.src_file = 0;
    load_info.dest_mip = e->mipmaps[need_to_use];      
    //should be asynchronous
     
    //prevents from trying to load this same mip level twice at the same time
    if (!(load_info.dest_mip->flags & R1_MIPLEVEL_IS_LOADING))
    {
      //if there is no room for this mip, load up as high a level as possible
      while (!async_mip_load(&load_info))
      {
        if (load_info.error==R1_MIP_LOAD_NO_ROOM)
        {
          //try to load a lower res
          need_to_use++;
          if (need_to_use==highest_resident) break;
          if (!e->mipmaps[need_to_use]) break;
          if (e->mipmaps[need_to_use]->flags & R1_MIPLEVEL_IS_LOADING) break;
          load_info.dest_mip = e->mipmaps[need_to_use];
        }
        else
          break;
      }
    }
    //else
    //{
    //  i4_warning("texture still loading, wont queue");
    //}
  }  
      
  pf_get_texture.stop();
  
  //store the old information back.
  return_mip->last_frame_used = old_last_frame;

  return return_mip;
}

w32 r1_texture_manager_class::average_texture_color(r1_texture_handle _handle, w32 frame_num)
{
  r1_texture_handle handle = registered_tnames[_handle].handle;

  r1_texture_entry_struct *e;
  
  if (handle>=total_textures)
    i4_error("asking for bad texture");

  if (handle<0)  // this is an animation
  {
    r1_texture_animation_entry_struct *ta=tanims+(-handle-1);
    handle=ta->frames[frame_num%ta->total_frames];
    if (!handle)
    {
      pf_get_texture.stop();
      return 0;
    }
  }
  
  e = &(*entries)[handle];

  return e->average_color;
}


void r1_texture_manager_class::next_frame()
{ 
  frame_count++;
  //eventually (after a long fucking time) this could
  //wrap around. just make sure its always > 0
  if (frame_count<0) frame_count=0;
}

int tex_entry_compare(const tex_cache_entry_t *a, const tex_cache_entry_t *b)
{
  if (a->id > b->id)
    return 1;
  else
  if (a->id < b->id)
    return -1;
  else
    return 0;
}

tex_cache_entry_t *find_id_in_tex_cache(tex_cache_entry_t *entries, w32 num_entries, w32 search_id)
{
  if (!entries || num_entries==0)
    return 0;

  w32 res;
  
  tex_cache_entry_t search;
  search.id = search_id;

  if (!i4_base_bsearch(&search,res,entries,
                       sizeof(tex_cache_entry_t),
                       num_entries,
                       (i4_bsearch_compare_function_type)tex_entry_compare))
    return 0;

  return entries+res;
}

i4_bool palettes_are_same(i4_pixel_format *a, i4_pixel_format *b)
{
  sw32 i;
  
  w8 *compare_a = (w8 *)a;
  w8 *compare_b = (w8 *)b;

  for (i=0;i<sizeof(i4_pixel_format); i++)
  {
    if (*compare_a != *compare_b)
      return i4_F;

    compare_a++;
    compare_b++;
  }
  
  return i4_T;
}

#include "tex_cache.cc"
#include "tex_heap.cc"

void r1_texture_manager_class::keep_resident(const i4_const_str &tname, sw32 desired_width)
{
  w32 id = r1_get_texture_id(tname);
  sw16 handle = find_texture(id);
  if (!handle) return;
    
  r1_texture_entry_struct *t = &(*entries)[handle];

  r1_mip_load_info load_info;

  //load all levels <= desired_width
  load_info.src_file = 0;
  
  sw32 j;
  for (j=0; j<R1_MAX_MIP_LEVELS; j++)
  {          
    r1_miplevel_t *mip = t->mipmaps[j];
    
    if (mip && (mip->width <= desired_width))
    {      
      mip->last_frame_used = -1;

      if ((mip->vram_handle==0) && (!(mip->flags & R1_MIPLEVEL_IS_LOADING)))
      {
        load_info.dest_mip = mip;      
        if (!immediate_mip_load(&load_info))
        {
          //these absolutely must work
          i4_error("keep_resident::could not load texture, cannot continue");        
        }
      }
    }
  }
}

i4_bool r1_texture_manager_class::load_textures()
{  
  sw32 i,j;

  if (textures_loaded)
    return i4_F;

   r1_texture_ref *p=r1_texture_ref::first;
  for (; p; p=p->next)
    p->texture_handle=register_texture(p->name, "code reference");
  
  //create an array of texture id's from names[] and sort it
  i4_array<w32> texture_file_ids(128,128);  

  for (i=0; i<registered_tnames.size(); i++)
    texture_file_ids.add(registered_tnames[i].id);


  //sort the list
  texture_file_ids.sort(w32_compare);

  //update / build / rebuild the cache file
  keep_cache_current(&texture_file_ids);

  i4_file_class *cache_file = i4_open(r1_get_cache_file(), I4_READ | I4_NO_BUFFER);  

  tex_cache_header_t tex_cache_header;

  if (cache_file)
  {      
    //read in the header for further processing
    tex_cache_header.read(cache_file);    
  }
  else
  {
    i4_warning("Couldnt locate texture cache file.");
    tex_cache_header.num_entries = 0;
    tex_cache_header.entries     = 0;
  }
  

  //OK FINALLY. process this crap. load information for all
  //requested textures (their ids are currently in texture_file_ids)
  
  //dynamic array of loaded texture entries    
  i4_array<r1_texture_entry_struct> new_texture_entries(128,128);

  i4_status_class *stat = i4_create_status(r1_gets("loading_textures"));  

  for (i=0; i<texture_file_ids.size(); i++)
  {
    w32 id = texture_file_ids[i];
    
    tex_cache_entry_t *t = find_id_in_tex_cache(tex_cache_header.entries,
                                                tex_cache_header.num_entries,id);

    if (!t || (t->lowmipoffset==0xFFFFFFFF))
    {
      for (int k=0; k<registered_tnames.size(); k++)
        if (registered_tnames[k].id==id)
        {
          i4_warning("Texture: %s not found in texture cache, run maxtool 'Update textures'", registered_tnames[k].name);
        }
    }
    else    
    {
      //texture is in the cache. need to load up some info, load the lowest mip, etc
      //add it to the list of valid textures
      r1_texture_entry_struct *new_entry = new_texture_entries.add();

      //the last one should always be null, hence the R1_MAX_MIP_LEVELS+1
      memset(new_entry->mipmaps,0,sizeof(r1_miplevel_t *) * (R1_MAX_MIP_LEVELS+1));
      
      new_entry->flags         = t->flags;
      new_entry->id            = t->id;
      new_entry->average_color = t->average_color;                            
      
      generate_mip_offsets(t->base_width,t->base_height,t->num_mip_levels,(sw32 *)new_entry->file_offsets,2);

      //fill in this structure. information on mip levels
      for (j=0; j<t->num_mip_levels; j++)
      {
        new_entry->mipmaps[j] = new r1_miplevel_t;
      
        r1_miplevel_t *mip = new_entry->mipmaps[j];

        mip->level  = j;
        mip->width  = t->base_width /(1<<j);
        mip->height = t->base_height/(1<<j);
        mip->entry  = new_entry;
                
        mip->flags = 0;
      }

      //seek to the low mip offset (stored IN the cache file)
      cache_file->seek(t->lowmipoffset+8);

      r1_mip_load_info load_info;
    
      //the dst_mip is the very last one    
      load_info.src_file = cache_file;
      load_info.dest_mip = new_entry->mipmaps[t->num_mip_levels-1];  
        
      //dont want these to ever be thrown out of texture memory
      load_info.dest_mip->last_frame_used   = -1;      

      //load that low mip level
      if (!immediate_mip_load(&load_info))
      {
        //check the error field in load_info
        i4_error("Could not load lowest miplevel of a texture, cannot continue");
      }
    }

    if (stat)
      stat->update((float)(i+1) / (float)texture_file_ids.size());
  }
        
  if (stat)
    delete stat;
  
  if (cache_file)
    delete cache_file;

  if (tex_cache_header.entries)
    i4_free(tex_cache_header.entries);
  else
  {
    //there were no texture cache entries? just get rid of the file. 
    i4_unlink(r1_get_cache_file());
  }
  

  //theres 1 additional texture, which will be entries[0], the default texture
  total_textures = new_texture_entries.size() + 1;

  entries = new i4_array<r1_texture_entry_struct>(total_textures,8);
  
  r1_texture_entry_struct blank_entry;
  memset(&blank_entry,0,sizeof(r1_texture_entry_struct));
  blank_entry.average_color = 0x00FFFFFF;

  entries->add(blank_entry);

  //should be sorted already but go ahead, sort again just in case
  if (new_texture_entries.size())
    new_texture_entries.sort(entry_compare);  

  for (i=0; i<total_textures-1; i++)
  {    
    entries->add();
    memset(&(*entries)[i+1],0,sizeof(r1_texture_entry_struct));
    
    (*entries)[i+1] = new_texture_entries[i];
    
    //crap. have to update the entry pointers since the mip's ->entry
    //references are in new_texture_entries (instead of entries[])
    for (j=0; j<R1_MAX_MIP_LEVELS; j++)
    {
      if ((*entries)[i+1].mipmaps[j])
      {
        (*entries)[i+1].mipmaps[j]->entry = &(*entries)[i+1];
      }
    }
  }    

  matchup_textures();

  /*
  i4_array<r1_texture_animation_entry_struct> anim_a(128,128);
  for (i=0; i<names.size(); i++)
  {
    w32 id=r1_get_texture_id(*names[i]);
    i4_str *fn=r1_animation_id_to_filename(id);
    i4_file_class *fp=opener(*fn);
    if (fp)
    {
      r1_texture_animation_entry_struct *a=anim_a.add();
      a->id=fp->read_32();
      a->total_frames=fp->read_16();
      a->frames=(r1_texture_handle *)i4_malloc(sizeof(r1_texture_handle)*a->total_frames,
                                                "animation frames");    
      for (j=0; j<a->total_frames; j++)
      {
        w32 id=fp->read_32();
        a->frames[j]=find_texture(id);
      }
    }

    delete fn;
  }  

  total_tanims=anim_a.size(); 
  if (total_tanims)
  {
    tanims=(r1_texture_animation_entry_struct *)i4_malloc(sizeof(r1_texture_animation_entry_struct)
                                                          * total_tanims, "animations");  
    for (i=0; i<anim_a.size(); i++)
      tanims[i]=anim_a[i];
  }
  else
    tanims=0;

  */
  return i4_T;
}

r1_texture_handle r1_texture_manager_class::register_image(i4_image_class *image)
{
  if (!entries || !entries->size())
    load_textures();

  //check to make sure image is power of 2
  sw32 i,j,new_width,new_height;

  for (i=1; i < image->width();  i = i<<1);
  new_width  = i;

  for (i=1; i < image->height(); i = i<<1);
  new_height = i;

  
  r1_texture_handle return_handle = 0;

  i4_draw_context_class context(0,0,image->width()-1,image->height()-1);


  r1_texture_entry_struct new_entry;
  memset(&new_entry,0,sizeof(r1_texture_entry_struct));


  const i4_pal *put_pal=pal;
  if (image->pal->source.alpha_bits)
  {
    put_pal=i4_pal_man.register_pal(&alpha_format);
    new_entry.flags|=R1_MIP_IS_ALPHATEXTURE;
  }


  i4_image_class *temp_image    = i4_create_image(image->width(),image->height(), put_pal);
  i4_image_class *texture_image = temp_image;
  
  image->put_image(temp_image,0,0,context);
  
  if (new_width != temp_image->width() || new_height != temp_image->height())
  {
    texture_image = i4_create_image(new_width,new_height, pal);
    
    sw32 old_width  = temp_image->width();
    sw32 old_height = temp_image->height();

    w16 *old_tex = (w16 *)temp_image->data;
    w16 *new_tex = (w16 *)texture_image->data;
    w16 *dst     = new_tex;
  
    float width_ratio  = (float)old_width  / (float)new_width;
    float height_ratio = (float)old_height / (float)new_height;
   
    //now scale the old to fit the new   

    for (j=0; j<new_height; j++)
    for (i=0; i<new_width;  i++, dst++)
    {    
      *dst = old_tex[(sw32)((double)j * height_ratio)*old_width + (sw32)((double)i * width_ratio)];
    }
    
    delete temp_image;
  }    

  i4_ram_file_class *fake_file = new i4_ram_file_class(texture_image->data,
                                                       texture_image->width() *
                                                       texture_image->height() * 2);


  new_entry.id            = (*entries)[entries->size()-1].id + 1; //maintains the sorted order of the array
  new_entry.average_color = 0xFFFFFFFF;
  new_entry.mipmaps[0]    = new r1_miplevel_t;
  
  r1_miplevel_t *mip = new_entry.mipmaps[0];

  mip->level  = 0;
  mip->width  = texture_image->width();
  mip->height = texture_image->height();
  
  mip->last_frame_used = -1;
  mip->vram_handle     = 0;
  mip->flags = R1_MIP_IS_ALPHATEXTURE;

  entries->add(new_entry);
  total_textures++;

  //update entry pointers for all miplevels  
  for (i=0; i<total_textures-1; i++)
  {
    //references are in new_texture_entries (instead of entries[])
    for (j=0; j<R1_MAX_MIP_LEVELS; j++)
    {
      if ((*entries)[i+1].mipmaps[j])
      {
        (*entries)[i+1].mipmaps[j]->entry = &(*entries)[i+1];
      }
    }
  }
    
  r1_mip_load_info load_info;
  load_info.dest_mip = mip;
  load_info.src_file = fake_file;

  if (!immediate_mip_load(&load_info))
  {
    i4_warning("tmanager:: register_image failed.");
  }
  else
  {
    return_handle = total_textures-1;  
  }
  
  delete fake_file;
  delete texture_image;  

  char name[256];
  sprintf(name, "memory_image_%d", return_handle);
  r1_texture_handle han=register_texture(name, name);
  registered_tnames[han].handle=return_handle;

  return han;
}





