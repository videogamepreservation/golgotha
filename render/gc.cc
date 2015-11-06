/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "tmanage.hh"
#include "time/profile.hh"

i4_profile_class pf_gc("texture gc");

// returns total bytes available after garbage collection
// expired_time is the time in which all older textures are thrown out at
w32 r1_texture_manager_class::gc(sw32 system_ram_expire_time,
                                 sw32 vram_expire_time)
{
  /*
  pf_gc.start();

  sw32 move_dist=0;
  free_node *current=(free_node *)start_collectable;
  r1_texture_node_struct *a;
  sw32 free_size=0;
  sw32 move_size, new_location;
  i4_bool free_it=i4_F;

  next_load.in_use=i4_F;  // since texture locations may change, invalid the 'next_load' hint

  //  check_heap(system_ram_expire_time, vram_expire_time);

  while (current!=end)
  {

    if (!isa_free_node(current))
    {
      a=(r1_texture_node_struct *)current;

      current=(free_node *)(((w8 *)a)+a->size()+sizeof(r1_texture_node_struct));
      move_size=a->size()+sizeof(r1_texture_node_struct);

      if (a->is_loaded_in_vram() && a->last_used<vram_expire_time && a->lower)
        free_vram_texture(a);
        
      // only free highlest level textures
      if (!a->higher && a->last_used<system_ram_expire_time && a->lower && a->width()>16)
      {
      //i4_warning("freeing %d %d  %d %d\n", a->width, a->height, a->size(), move_dist+move_size);
      // for (r1_texture_node_struct *c=a->lower; c; c=c->lower)
      // i4_warning("  child %d %d  %d\n", c->width, c->height, c->size());

        if (a->is_loaded_in_vram())
          free_vram_texture(a);

        r1_texture_entry_struct *te = a->entry;

        te->highest=a->lower;
        a->lower->entry=te;
        a->lower->higher=0;

        move_dist+=move_size;
        free_size+=move_size;
      }
      else if (move_dist)
      {
        //        w32 pre_check, post_check;
        //        pre_check=i4_check_sum32(a+1, a->width * a->height);

        r1_texture_node_struct *new_spot=(r1_texture_node_struct *)(((w8 *)a)-move_dist);

        // are we the highest mip level texture?        
        if (!a->higher)
          a->entry->highest=new_spot;
        else
          a->higher->lower=new_spot;

        if (a->lower)
          a->lower->higher=new_spot;

        //the killer memmove. this has a nasty habit of consuming 20% of our processing time.
        memmove(new_spot, a, move_size);
        //the killer memmove. this has a nasty habit of consuming 20% of our processing time.

        //        post_check=i4_check_sum32(new_spot+1, new_spot->width * new_spot->height);

        //        i4_warning("%d %d\n", pre_check, post_check);
      }
      //      else if (!a->lower)
      //        start_collectable=a;
    }
    else
    {
      move_size=current->size+sizeof(free_node);
      free_size+=move_size;
      move_dist+=move_size;
      current=(free_node *)((w8 *)current+current->size+sizeof(free_node));
    }
  }

  if (!free_size)
    first_free=0;
  else
  {  
    current=(free_node *)((w8 *)end-free_size);
    current->size=(sw32)free_size-(sw32)sizeof(free_node);
    current->next=(free_node *)end;
    first_free=current;  
  }


  //  check_heap(system_ram_expire_time, vram_expire_time);

  //  i4_warning("gc : free = %d\n",free_size);

  pf_gc.stop();

  */
  return 0;//free_size;
}



void r1_texture_manager_class::do_collection()
{
  return ;

  // can't do gc while textures are being loaded in
  if (loading_textures)
  {
    if (need_gc)
    {
      if (!current_load.in_use)
      {
        enum { SYS=40, VRAM=20 };
        sw32 system_ram_expire_time=frame_count>SYS ? frame_count-SYS : 0,
            vram_expire_time=frame_count>VRAM ? frame_count-VRAM : 0;
    
        if (gc(system_ram_expire_time,
              vram_expire_time))
          need_gc=i4_F;    
      }
    }
  }
}
