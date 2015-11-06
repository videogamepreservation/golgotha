/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "tmanage.hh"

void async_read_callback(w32 count, void *context)
{
  ((r1_texture_manager_class *)context)->async_read_finish(count);
}

sw32 r1_expect_size;

void r1_texture_manager_class::async_read_finish(sw32 count)
{
  /*
  r1_texture_node_struct *thi=current_load.new_texture;
  r1_texture_node_struct *tlo=thi->lower;
  r1_texture_entry_struct *e=thi->entry;

  // make sure this are in local byte order
  thi->w=s_to_lsb(thi->w);
  thi->h=s_to_lsb(thi->h);
  thi->next_size=l_to_lsb(thi->next_size);

  tlo->higher=thi;

  int han=thi->entry - entries;


  //  tlo->entry=0;
  e->highest=thi;    // at this point the texture is visible to the main process

  delete async_file;
  async_file=0;

   // is there another texture to load?
  if (next_load.in_use)
  {
    current_load=next_load;
    next_load.in_use=i4_F;
    load_current();
  }
  else current_load.in_use=i4_F;
  */

}

void r1_texture_manager_class::load_current()
{
  /*
  r1_texture_entry_struct *e=current_load.entry;
  if (e)
  {
    r1_texture_node_struct *lower=e->highest, *tn;

    sw32 size=lower->next_size;
    tn=alloc(size);


    // this shouldn't fail because we checked before for space
    if (!tn)
      i4_error("load current alloc failed");
    
    tn->entry=e;
    tn->higher=0;
    tn->lower=lower;
    tn->last_used=frame_count;

    current_load.new_texture=tn;
                        
    sw32 offset=e->next_texture_file_offset();

      //e->offset + r1_texture_disk_space(lower->width, lower->height);


    i4_str *texture_file=r1_texture_id_to_filename(e->id, *texture_dir);
    async_file=i4_open(*texture_file);
    delete texture_file;
    
    if (async_file)
    {
      async_file->seek(offset);
      async_file->async_read(&tn->w, size + 8, async_read_callback,  this);
    }
    else
    {
      i4_warning("texture id %d missing",e->id);
      current_load.in_use=i4_F;
    }
  }
  else current_load.in_use=i4_F;
  */
}


/*
i4_bool r1_texture_manager_class::increase_texture_detail(r1_texture_node_struct *tn)
{
  sw32 load_size=tn->next_size;
  i4_bool ret;

  // if this is a bigger texture than the one we are currently planning to load
  // plan on loading this one instead if it will fit in memory
  if ((!next_load.in_use || next_load.load_size<load_size) &&
      (!current_load.in_use || tn->entry!=current_load.entry))
  {
    if (can_alloc(load_size))  // is there enough space for this texture?
    {
      next_load.entry=tn->entry;
      next_load.load_size=load_size;
      next_load.in_use=i4_T;
      ret=i4_T;
    }
    else 
    {
      smallest_sys_failure=load_size;
      need_gc=i4_T;  // need to do garbage collection
      ret=i4_F;
    }
  }
  else ret=i4_F;

  // if we are not currently loading a texture then we should move next_load to current_load
  // and load it if next_load has been suggested
  if (!current_load.in_use)
  {
    if (next_load.in_use)
    {
      current_load=next_load;
      next_load.in_use=i4_F;
      load_current();
    }
  }

  was_a_miss=i4_T;
  return ret; 
  return i4_F;
}
*/


