/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "map_man.hh"
#include "map_cell.hh"
#include "g1_object.hh"
#include "tile.hh"
#include "map.hh"
#include "compress/rle.hh"
#include "loaders/dir_save.hh"
#include "loaders/dir_load.hh"
#include "saver_id.hh"
#include "map_vert.hh"

g1_object_chain_class *g1_map_cell_class::get_solid_list()
{
  if (!object_list || !object_list->object->get_flag(g1_object_class::BLOCKING))
    return 0;
  else return object_list;
}


g1_object_chain_class *g1_map_cell_class::get_non_solid_list()
{
  g1_object_chain_class *p=object_list;

  while (p && p->object->get_flag(g1_object_class::BLOCKING))
    p=p->next;
  
  return p;
}


void g1_map_cell_class::add_object(g1_object_chain_class &o) 
{ 
  if (!object_list ||
      o.object->get_flag(g1_object_class::BLOCKING))  // add solids to front of list
  {
    o.next=object_list; 
    object_list=&o; 
  }
  else
  {
    // add non solids after all solids
    g1_object_chain_class *last=object_list, *p=object_list->next;
    while (p && p->object->get_flag(g1_object_class::BLOCKING))
    {
      last=p;
      p=p->next;
    }
    o.next=last->next;
    last->next=&o;
  }
}

i4_bool g1_map_cell_class::remove_object(g1_object_chain_class &o)
{
  if (&o==object_list)
  {
    object_list=o.next;
    return i4_T;
  }
  else
  {
    g1_object_chain_class *c=object_list, *last;
    while (c && c!=&o)
    {
      last=c;
      c=c->next;      
    }
    if (!c) 
    {
      i4_warning("map_cell::remove_object, object not in cell");
      return i4_F;
    }

    last->next=o.next;
    return i4_T;
  }
}


void g1_map_cell_class::init(w16 _type, g1_rotation_type rot,
                             i4_bool mirrored)
{
  object_list=0;
  type=_type;
  flags=FOGGED;
  top_left_normal=0x8000;
  bottom_right_normal=0x8000;
  

  g1_tile_man.get(_type)->apply_to_cell(*this);  
  set_rotation(rot);
  if (mirrored)
    flags|=MIRRORED;
}


void g1_map_cell_class::recalc_top_left(int cx, int cy)
{
  int mw=g1_get_map()->width(), mh=g1_get_map()->height();

  I4_ASSERT(cx<=mw && cy<=mh, "recalc normal : vert out of range");

  if (cx==mw-1 || cy==mh-1)
    top_left_normal=(31) | (15<<5) | (15<<10);     // normal = 0,0,1
  else
  {
    g1_map_vertex_class *v=g1_get_map()->vertex(cx,cy);

    float h0=v->get_height(), h2=v[mw+1].get_height(), h3=v[mw+1+1].get_height();
 
    i4_3d_vector v1(1, 0, h3-h2), v2(0, -1, h0-h2);

    i4_3d_vector normal;
    normal.cross(v2,v1);
    normal.normalize();
   
    top_left_normal=g1_normal_to_16(normal);   
  }

}


void g1_map_cell_class::recalc_bottom_right(int cx, int cy)
{
  int mw=g1_get_map()->width(), mh=g1_get_map()->height();
  if (cx>=mw || cy>=mh)
    bottom_right_normal=(31) | (15<<5) | (15<<10);     // normal = 0,0,1
  else
  {
    g1_map_vertex_class *v=g1_get_map()->vertex(cx,cy);
    float h0=v->get_height(), h1=v[1].get_height(), h3=v[mw+1+1].get_height();
 
    i4_3d_vector v1(-1, 0, h0-h1), v2(0, 1, h3-h1);

    i4_3d_vector normal;
    normal.cross(v2,v1);
    normal.normalize();
       
    bottom_right_normal=g1_normal_to_16(normal);   
  }

}


void g1_save_map_cells(g1_map_cell_class *list, 
                       int lsize, i4_saver_class *fp)
{
  int i,j;
  i4_rle_class<w16> fp16(fp);

  fp->mark_section("golgotha cell type v1");
  for (i=0; i<lsize; i++)
    fp16.write(list[i].type);
  fp16.flush();


  fp->mark_section("golgotha cell flags v2");
  for (i=0; i<lsize; i++)
    fp16.write(list[i].flags & g1_map_cell_class::SAVED_FLAGS);
  fp16.flush();


  fp->mark_section("golgotha cell normals v1");
  for (i=0; i<lsize; i++)
    fp16.write(list[i].top_left_normal);
  for (i=0; i<lsize; i++)
    fp16.write(list[i].bottom_right_normal);
  fp16.flush(); 
  
}
       


i4_bool g1_load_map_cells(g1_map_cell_class *list, int lsize, 
                          w16 *tile_remap, i4_loader_class *fp)
{
  int i;
  i4_rle_class<w16> fp16(fp);

  //////////////////////////////////////////////////////////
  // support for old formats
  if (!fp->goto_section("golgotha cell type v1"))
  {
    g1_get_map()->mark_for_recalc(G1_RECALC_STATIC_LIGHT);

    if (fp->goto_section(OLD_G1_SECTION_CELL_V6))
    {
      for (i=0; i<lsize; i++)
        list[i].load_v6(fp, tile_remap);
    } else if (fp->goto_section(OLD_G1_SECTION_CELL_V5))
    {
      for (i=0; i<lsize; i++)
        list[i].load_v5(fp, tile_remap);
    }
    else if (fp->goto_section(OLD_G1_SECTION_CELL_V4))
    {
      for (i=0; i<lsize; i++)
        list[i].load_v4(fp, tile_remap);
    } else if (fp->goto_section(OLD_G1_SECTION_CELL_V3))
    {
      for (i=0; i<lsize; i++)
        list[i].load_v3(fp, tile_remap);
    } else if (fp->goto_section(OLD_G1_SECTION_CELL_V2))
    {
      for (i=0; i<lsize; i++)
        list[i].load_v2(fp, tile_remap);
    }
    else
      return i4_F;
  }
  else /////////////////////// new format : stored in rle arrays rather than structs
  {    
    for (i=0; i<lsize; i++)
      list[i].type=tile_remap[fp16.read()];
  }



  if (fp->goto_section("golgotha cell flags v2"))
  {
    for (i=0; i<lsize; i++)
      list[i].flags = fp16.read() & g1_map_cell_class::SAVED_FLAGS;
  }
  else if (fp->goto_section("golgotha cell flags v1"))
  {
    for (i=0; i<lsize; i++)
    {
      list[i].flags = (fp16.read() & g1_map_cell_class::SAVED_FLAGS) | g1_map_cell_class::FOGGED;
      list[i].clear_restore_bits();
    }
  } 

  for (i=0; i<lsize; i++)
    g1_tile_man.get(list[i].type)->apply_to_cell(list[i]);  


  if (fp->goto_section("golgotha cell normals v1"))
  {
    for (i=0; i<lsize; i++)
      list[i].top_left_normal=fp16.read();

    for (i=0; i<lsize; i++)
      list[i].bottom_right_normal=fp16.read();
  }

  return i4_T;
}





///////////////////// OLD LOAD CODE //////////////////////////


void g1_map_cell_class::load_v2(i4_file_class *fp, w16 *tile_remap)
{
  object_list=0;
  //  function_block=0xffff;

  type=tile_remap[fp->read_16()];
  flags=fp->read_8();
  fp->read_8();
  fp->read_16();            // nearest node gone

  top_left_normal=0x8000;
  bottom_right_normal=0x8000;
}

void g1_map_cell_class::load_v3(i4_file_class *fp, w16 *tile_remap)
{
  object_list=0;

  type=tile_remap[fp->read_16()];
  flags=fp->read_16();

  fp->read_8();  // height not used anymore

  fp->read_16();            // nearest node gone

  top_left_normal=0x8000;
  bottom_right_normal=0x8000;
}

void g1_map_cell_class::load_v4(i4_file_class *fp, w16 *tile_remap)
{
  object_list=0;

  type=tile_remap[fp->read_16()];
  flags=fp->read_16();

  fp->read_16();            // nearest node gone

  if (type==0)
    flags |= IS_GROUND;

  top_left_normal=0x8000;
  bottom_right_normal=0x8000;
}

void g1_map_cell_class::load_v5(i4_file_class *fp, w16 *tile_remap)
{
  object_list=0;
        
  type     = tile_remap[fp->read_16()];
  fp->read_8();


  flags=fp->read_16();

  fp->read_16();              // nearest_node gone

  if (type==0)
    flags |= IS_GROUND;  

  top_left_normal=0x8000;
  bottom_right_normal=0x8000;
}

void g1_map_cell_class::load_v6(i4_file_class *fp, w16 *tile_remap)
{
  object_list=0;
  type     = tile_remap[fp->read_16()];
  fp->read_8();

  w32 blah = fp->read_32();     // height info not used anymoe
  if (blah)
  {
    fp->read_8();
    fp->read_16();
    fp->read_float();
    fp->read_float();
  }


  flags=fp->read_16();

  fp->read_16();              // nearest_node gone

  if (type==0)
    flags |= IS_GROUND;  

  top_left_normal=0x8000;
  bottom_right_normal=0x8000;
}



void g1_map_cell_class::unfog(int x, int y)
{
  flags&=~FOGGED;
  g1_map_vertex_class *v=g1_verts+x+y*(g1_map_width+1);
  v->set_flag(g1_map_vertex_class::FOGGED,0);
  v++;
  v->set_flag(g1_map_vertex_class::FOGGED,0);
  v+=g1_map_width;
  v->set_flag(g1_map_vertex_class::FOGGED,0);
  v++;
  v->set_flag(g1_map_vertex_class::FOGGED,0);
}
