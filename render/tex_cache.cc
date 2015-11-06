/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

//WARNING: this code is really nauseating.
#include "tex_cache.hh"
#include "mip_average.hh"
#include "status/status.hh"
#include "file/file.hh"
#include "error/error.hh"
#include "r1_res.hh"

void do_single_file(i4_file_class *dst_file,
                    w32 id,                    
                    sw32 max_texture_dimention,
                    w32 network_file_date,
                    tex_cache_entry_t *t);

struct file_status_sort_struct
{
  w32 id;
  w32 file_status_index;
};

int file_status_struct_compare(const file_status_sort_struct *a, const file_status_sort_struct *b)
{
  if (a->id<b->id)
    return -1;
  else if (a->id>b->id)
    return 1;
  else return 0;
}


void tex_cache_entry_t::write(i4_file_class *f)
{
  if (!f)
  {
    i4_warning("tex_cache_entry_t::write() NULL file");
    return;
  }

  f->write_32(id);
  f->write_32(lowmipoffset);
  f->write_32(average_color);
  f->write_32(last_modified);
    
  f->write_16(base_width);
  f->write_16(base_height);

  f->write_8(flags);
  f->write_8(num_mip_levels);    
}

void tex_cache_entry_t::read(i4_file_class *f)
{
  if (!f)
  {
    i4_warning("tex_cache_entry_t::read() NULL file");
    return;
  }

  id             = f->read_32();
  lowmipoffset   = f->read_32();
  average_color  = f->read_32();
  last_modified  = f->read_32();
    
  base_width     = f->read_16();
  base_height    = f->read_16();

  flags          = f->read_8();
  num_mip_levels = f->read_8();
}



void tex_cache_header_t::write(i4_file_class *f)
{
  if (!f)
  {
    i4_warning("tex_cache_header_t::write() NULL file");
    return;
  }

  regular_format.write(f);
  chroma_format.write(f);
  alpha_format.write(f);
    
  f->write_32(max_mip_dimention);
  f->write_32(num_entries);        
}
  
void tex_cache_header_t::read(i4_file_class *f)
{
  if (!f)
  {
    i4_warning("tex_cache_header_t::read() NULL file");
    return;
  }

  if (f->size() < tex_cache_header_disk_size())
  {
    i4_warning("texture cache file corrupt");
    regular_format.default_format();
    chroma_format.default_format();
    alpha_format.default_format();
    max_mip_dimention = 0;
    num_entries       = 0;
    entries           = 0;
    return;
  }

  //seek to the end and read the last dword
    
  f->seek(f->size()-4);
  sw32 expected_size = f->read_32();

  if (expected_size != f->size())
  {      
    i4_warning("texture cache file corrupt");
    entries = 0;
    num_entries = 0;
    return;
  }    
    
  //seek back to the beginning and start reading the file
  f->seek(0);

  regular_format.read(f);
  chroma_format.read(f);
  alpha_format.read(f);
    
  max_mip_dimention = f->read_32();
  num_entries = f->read_32();
          
  w32 entry_lump_size = tex_cache_entry_disk_size() * num_entries;

  //read in the entries

  if (entry_lump_size)
  {    
    entries = (tex_cache_entry_t *)i4_malloc(sizeof(tex_cache_entry_t) * num_entries,"tex_cache_entries");
    f->seek(f->size() - 4 - entry_lump_size);
  }
  else
  {
    num_entries = 0;
    entries     = 0;
  }

  sw32 i;

  for (i=0; i<num_entries; i++)
  {
    entries[i].read(f);
  }  
}


i4_bool r1_texture_manager_class::update_cache_file(i4_array<w32> &update_ids,
                                                    const i4_const_str &network_dir,
                                                    const i4_const_str &local_dir)
{
  sw32 i,j,k;

  i4_array<w32>               network_file_ids(128,128);
  i4_array<tex_cache_entry_t> new_cache_entries(128,128);

  i4_status_class *stat=i4_create_status(r1_gets("checking_times"), I4_STATUS_UNKNOWN_TOTAL);


  i4_directory_struct ds;
  i4_get_directory(network_dir, ds, i4_T, stat);

  delete stat;

  for (i=0; i<ds.tfiles; i++)
    network_file_ids.add(r1_get_file_id(*ds.files[i]));    

  i4_file_class *old_cache_file = i4_open(r1_get_cache_file(), I4_READ | I4_NO_BUFFER);

  w32 csize = old_cache_file->size();

  void *old_cache_file_data = i4_malloc(csize,"old cache file data");
  old_cache_file->read(old_cache_file_data,csize);

  delete old_cache_file;
  
  old_cache_file = new i4_ram_file_class(old_cache_file_data,csize);

  tex_cache_header_t old_cache_header;

  old_cache_header.read(old_cache_file);  

  w32 total_cache_entries = old_cache_header.num_entries;
  w32 old_cache_header_num_entries = old_cache_header.num_entries;

  for (i=0; i < update_ids.size();)
  {
    tex_cache_entry_t *t = find_id_in_tex_cache(old_cache_header.entries,old_cache_header.num_entries,update_ids[i]);
    
    if (t)
    {
      update_ids.remove(i); //pull this out of the list
      t->flags |= R1_MIP_EXTRA_FLAG;
    }
    else
    {
      //new textures going into the cache file
      total_cache_entries++;
      i++;
    }
  }
  
  i4_file_class *new_cache_file = i4_open(r1_get_cache_file(), I4_WRITE | I4_NO_BUFFER);

  old_cache_header.num_entries = total_cache_entries;
  old_cache_header.write(new_cache_file);  
  
  r1_setup_decompression(&regular_format,&chroma_format,&alpha_format,R1_CHROMA_COLOR,square_textures);

  update_ids.sort(w32_compare);  
  
  j = 0;
  i = 0;  

  i4_bool i_done = i4_F;
  i4_bool j_done = i4_F;

  stat = i4_create_status(r1_gets("updating_texture_cache"));

  while (1)
  {  
    if (j>=update_ids.size()) j_done = i4_T;
    
    if (i>=old_cache_header_num_entries) i_done = i4_T;

    if (i_done && j_done) break;

    w32 file_offs = new_cache_file->tell();

    if ((j_done && !i_done) || (!i_done && old_cache_header.entries[i].id < update_ids[j]))
    {
      tex_cache_entry_t *t = &old_cache_header.entries[i];
            
      if (!(t->flags & R1_MIP_EXTRA_FLAG))
      {
        if (t->lowmipoffset != 0xFFFFFFFF)
        {
          old_cache_file->seek(t->lowmipoffset);

          sw32 entry_size = sizeof(sw32) * 2 +
                           t->base_width  / (1<<(t->num_mip_levels-1)) *
                           t->base_height / (1<<(t->num_mip_levels-1)) * 2;
    
          t->lowmipoffset = file_offs;

          w8 copy_buffer[2048];
          while (entry_size)
          {          
            sw32 copy_size = (entry_size < 2048) ? (entry_size) : (2048);

            old_cache_file->read(copy_buffer, copy_size);
            new_cache_file->write(copy_buffer,copy_size);
            
            entry_size -= copy_size;
          }
        }
      }
      else
      {
        t->flags &= (~R1_MIP_EXTRA_FLAG);

        w32 file_date=0xFFFFFFFF;

        //have to update this texture    
        for (k=0; k<network_file_ids.size(); k++)
        {
          if (t->id==network_file_ids[k])
          {
            file_date = ds.file_status[k].last_modified;
            break;
          }
        }

        //dont want files in the cache w/different mip levels.
        do_single_file(new_cache_file,
                       t->id,                       
                       old_cache_header.max_mip_dimention, 
                       file_date,
                       t);        
      }

      new_cache_entries.add(*t);

      i++;
    }    
    else
    if ((i_done && !j_done) || (!j_done && update_ids[j] < old_cache_header.entries[i].id))    
    {
      //have to add this id
      w32 new_id = update_ids[j];

      w32 file_date=0xFFFFFFFF;

      //have to update this texture    
      for (k=0; k<network_file_ids.size(); k++)
      {
        if (new_id==network_file_ids[k])
        {
          file_date = ds.file_status[k].last_modified;
          break;
        }
      }

      tex_cache_entry_t new_entry;

      do_single_file(new_cache_file,
                     new_id,                     
                     old_cache_header.max_mip_dimention,
                     file_date,
                     &new_entry);

      new_cache_entries.add(new_entry);
      j++;  
    }

    if (stat)
      stat->update((new_cache_entries.size()+1) / (float)total_cache_entries);
  }
    
  if (stat)
    delete stat;


  if (old_cache_header.entries)
    i4_free(old_cache_header.entries);

  i4_free(old_cache_file_data);
  delete old_cache_file;
  
  r1_end_decompression();  

  if (total_cache_entries != new_cache_entries.size())
  {
    i4_warning("error updating the texture cache file");
  }

  for (i=0;i<new_cache_entries.size();i++)
  {
    new_cache_entries[i].write(new_cache_file);    
  }

  sw32 cache_size = new_cache_file->tell() + 4;

  new_cache_file->write_32(cache_size);

  delete new_cache_file;
                  
  return i4_T;
}

i4_bool r1_texture_manager_class::build_cache_file(i4_array<w32> &texture_file_ids,
                                                   const i4_const_str &network_dir,
                                                   const i4_const_str &local_dir)
{
  //creates a tex_cache.dat "directory" w/lowest mip levels and mipheader_t info (tex_cache_entry_t, actually)
  //decompresses others to local .gtx files

  i4_array<w32> network_file_ids(128,128);  

  sw32 i,j;  


  r1_setup_decompression(&regular_format,&chroma_format,&alpha_format,R1_CHROMA_COLOR,square_textures);

  i4_file_class *dst_file = i4_open(r1_get_cache_file(),I4_WRITE | I4_NO_BUFFER);

  tex_cache_header_t tex_cache_header;

  memcpy(&tex_cache_header.regular_format,&regular_format,sizeof(i4_pixel_format));
  memcpy(&tex_cache_header.chroma_format ,&chroma_format, sizeof(i4_pixel_format));
  memcpy(&tex_cache_header.alpha_format  ,&alpha_format,  sizeof(i4_pixel_format));

  tex_cache_header.max_mip_dimention = max_texture_dimention;
  tex_cache_header.num_entries       = texture_file_ids.size();

  tex_cache_header.write(dst_file);  
  
  tex_cache_entry_t *tex_cache_entries = 0;

  if (texture_file_ids.size())
  {
    tex_cache_entries = (tex_cache_entry_t *)
      i4_malloc(sizeof(tex_cache_entry_t) * texture_file_ids.size(),"tex_cache_entries");
  }

  //the entries will go at the end of the file. to get to the start of the entry list,
  //seek to:   filesize()-(num_entries*sizeof(tex_cache_entry_t))


  

  i4_status_class *stat=i4_create_status(r1_gets("checking_times"), I4_STATUS_UNKNOWN_TOTAL);

  i4_directory_struct ds;
  i4_get_directory(network_dir, ds, i4_T, stat);
  delete stat;

  for (i=0; i<ds.tfiles; i++)
    network_file_ids.add(r1_get_file_id(*ds.files[i]));    

  stat = i4_create_status(r1_gets("building_texture_cache"));
  
  if (network_file_ids.size() != ds.tfiles)
    i4_error("something bad happened building the texture cache file");
  
  for (i=0; i<texture_file_ids.size(); i++)
  {    
    w32 id = texture_file_ids[i];    
    
    w32 file_date = 0xFFFFFFFF;
    
    for (j=0; j<network_file_ids.size(); j++)
    {
      if (id==network_file_ids[j])
      {
        file_date = ds.file_status[j].last_modified;
        break;
      }
    }        

    //we need to decompress (for now, just copy)
    //the network (or cd image) texture to our hard drive
    do_single_file(dst_file,
                   id,                   
                   max_texture_dimention,
                   file_date,
                   &tex_cache_entries[i]);
        
    
    if (stat)
      stat->update((float)(i+1) / (float)texture_file_ids.size());
  }

  if (stat)
    delete stat;


  if (tex_cache_entries)    
  {
    for (i=0; i<tex_cache_header.num_entries; i++)
    {
      tex_cache_entries[i].write(dst_file);
    }    
    i4_free(tex_cache_entries);
  }

  sw32 cache_size = dst_file->tell() + 4;

  dst_file->write_32(cache_size);

  delete dst_file;  

  r1_end_decompression();  
  


  return i4_T;
}



void do_single_file(i4_file_class *dst_file,
                    w32 id,                    
                    sw32 max_texture_dimention,
                    w32 network_file_date,
                    tex_cache_entry_t *t)
{

  i4_str *local_fname   = r1_texture_id_to_filename(id, r1_get_decompressed_dir());
  i4_str *network_fname = r1_texture_id_to_filename(id, r1_get_compressed_dir());

  char local_tex_file[256];
  i4_os_string(*local_fname,local_tex_file,256);
  delete local_fname;
    
  char network_tex_file[256];
  i4_os_string(*network_fname,network_tex_file,256);    
  delete network_fname;

  i4_bool loaded = i4_F;
      
  w32 file_offs = dst_file->tell();

  //open the mip file from the network to get information on the mip levels    
  mipheader_t mipheader;

  i4_file_class *src_file = i4_open(i4_const_str(network_tex_file),I4_READ | I4_NO_BUFFER);
  if (!src_file)
    i4_warning("Couldn't open compressed texture: %s.", network_tex_file);
    
  void *file_buf = 0;
  w32   file_size;

  if (src_file)
  {
    file_size = src_file->size();
    file_buf  = i4_malloc(file_size,"file buffer");
  
    src_file->read(file_buf,file_size);
    delete src_file;

    src_file = new i4_ram_file_class(file_buf,file_size);
  }    

  if (src_file)
  {      
    mipheader.read(src_file);

    loaded = r1_decompress_to_local_mip(src_file,
                                        dst_file,
                                        network_tex_file,
                                        local_tex_file,
                                        &mipheader,
                                        max_texture_dimention);
      
    if (file_buf)        
      i4_free(file_buf);

    delete src_file;
  }        

  if (loaded)    
  {      
    t->id             = id;      
    t->lowmipoffset   = file_offs;
    t->last_modified  = network_file_date;
    t->average_color  = mipheader.average_color;
    t->base_width     = mipheader.base_width;
    t->base_height    = mipheader.base_height;
    t->flags          = mipheader.flags;
    t->num_mip_levels = mipheader.num_mip_levels;
  }
  else
  {
    i4_warning("Failed to decompress %s. Try updating textures w/the maxtool.",network_tex_file);
    
    //be sure to set lowmipoffset to 0xFFFFFFFF and last_modified to 0 since the load failed
    t->id             = id;      
    t->lowmipoffset   = 0xFFFFFFFF;
    t->last_modified  = 0;
    t->average_color  = 0;
    t->base_width     = 0;
    t->base_height    = 0;
    t->flags          = 0;
    t->num_mip_levels = 0;
  }  
}


void r1_texture_manager_class::keep_cache_current(i4_array<w32> *file_ids)
{
  tex_cache_header_t tex_cache_header;

  i4_file_class *cache_file = i4_open(r1_get_cache_file(),I4_READ | I4_NO_BUFFER);  

  //does the cache file exist?
  if (!cache_file)
  {
    //need to build a cache file
    if (file_ids && file_ids->size())
      build_cache_file(*file_ids, r1_get_compressed_dir(), r1_get_decompressed_dir());
  }
  else
  {
    //cache file exists, make sure its in the proper format
    //if anything doesnt match, gotta rebuild
    tex_cache_header.read(cache_file);    
    
    if (
        !palettes_are_same(&tex_cache_header.regular_format,&regular_format) ||        
        !palettes_are_same(&tex_cache_header.chroma_format,&chroma_format)   ||
        !palettes_are_same(&tex_cache_header.alpha_format,&alpha_format)     ||
        (tex_cache_header.max_mip_dimention < max_texture_dimention)
       )
    {
      //close the old cache file
      if (tex_cache_header.entries)
        i4_free(tex_cache_header.entries);

      delete cache_file;
      cache_file = 0;

      //build a new one
      if (file_ids && file_ids->size())
        build_cache_file(*file_ids,
                         r1_get_compressed_dir(), 
                         r1_get_decompressed_dir());

    }
    else
    if (file_ids && file_ids->size())
    {
      //cache file was there (and in a compatible format) but we need to make sure it is up to date           
  
      //yes. get directory information from the source compressed texture directory
      i4_directory_struct ds;
      i4_get_directory(r1_get_compressed_dir(), ds, i4_T);

      i4_array<file_status_sort_struct> sorted_status_indices(128,128);
      
      sw32 i;

      for (i=0; i<ds.tfiles; i++)
      {
        file_status_sort_struct s;
        s.id = r1_get_file_id(*(ds.files[i]));
        s.file_status_index = i;
        
        sorted_status_indices.add(s);
      }
      
      sorted_status_indices.sort(file_status_struct_compare);

      i4_array<w32> ids_needing_update(128,128);

      //determine which textures need to be updated,
      //and also determine if there are any NEW ones
      //that need to be added to the cache

      for (i=0; i<file_ids->size(); i++)
      {
        w32 id = (*file_ids)[i];//r1_get_file_id(*ds.files[i]);

        tex_cache_entry_t *t = find_id_in_tex_cache(tex_cache_header.entries,
                                                    tex_cache_header.num_entries,id);
        
        if (!t || (t->lowmipoffset==0xFFFFFFFF))
        {
          if (t && t->lowmipoffset==0xFFFFFFFF)
          {
            //its not in the cache right now because the file didnt exist before
            //if it exists now, add it to the update list. if not, well dont even try
            //(this avoids creating an "update textures" wait everytime if a bunch of texture files
            // just dont exist)
            file_status_sort_struct s;
            s.id = id;

            sw32 location = sorted_status_indices.binary_search(&s,file_status_struct_compare);
            if (location != -1)
            {
              //yeah, its in the directory, add it to the update list
              ids_needing_update.add(id);
            }
          }
          else
          {
            //its not in the cache. add it
            ids_needing_update.add(id);
          }
        }
        else
        {
          //it is in the cache. make sure its current. if the file does not exist, dont
          //add it

          file_status_sort_struct s;
          s.id = id;

          if (id==0x0f6dbf86)
          {
            int a;
            a=0;
          }

          sw32 location = sorted_status_indices.binary_search(&s,file_status_struct_compare);

          //it exists in the directory. if the directory file is more recent than ours, update it.
          //if it doesnt exist, well then obviously dont update it.
          if (location != -1)
          {
            if (ds.file_status[sorted_status_indices[location].file_status_index].last_modified > t->last_modified)
            {
              //must update this texture
              ids_needing_update.add(id);
            }
          }
        }
      }
    

      if (tex_cache_header.entries)
        i4_free(tex_cache_header.entries);

      delete cache_file;
        cache_file = 0;

      //does anything need updating?
      if (ids_needing_update.size())
      {
        //yes. delete this old data and re-read it after we've updated the cache file      
        update_cache_file(ids_needing_update,
                          r1_get_compressed_dir(), 
                          r1_get_decompressed_dir());
      }
    }
    else
    {
      if (tex_cache_header.entries)
        i4_free(tex_cache_header.entries);

      delete cache_file;
      cache_file = 0;
    }
  }

  if (cache_file)
  {
    i4_warning("unhandled case: keep_cache_current");
  }
}
