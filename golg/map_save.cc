/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "map.hh"
#include "saver.hh"
#include "tile.hh"       
#include "g1_object.hh"
#include "path.hh"
#include "load3d.hh"
#include "m_flow.hh"
#include "player.hh"
#include "light.hh"
#include "remove_man.hh"
#include "g1_render.hh"
#include "saver_id.hh"
// #include "critical_graph.hh"
// #include "solvemap_astar.hh"

#include "objs/old_ids.hh"
#include "height_info.hh"
#include "loaders/load.hh"
#include "map_man.hh"
#include "cwin_man.hh"

#include "objs/map_piece.hh"
#include "lisp/li_load.hh"
#include "map_cell.hh"
#include "map_vert.hh"
#include "tick_count.hh"
#include "map_data.hh"

void g1_map_class::save_objects(g1_saver_class *out)
{
  g1_object_class *olist[G1_MAX_OBJECTS];
  sw32 t=make_object_list(olist, G1_MAX_OBJECTS), ttypes=0, i;
            
  out->set_helpers(olist, t);

  // save the names of all the object types
  out->mark_section(G1_SECTION_OBJECT_TYPES_V1);
  out->write_32(g1_last_object_type+1);

  for (i=0; i<=g1_last_object_type; i++)
    if (g1_object_type_array[i])
    {
      const char *name=g1_object_type_array[i]->name();
      int len=strlen(name)+1;
      out->write_16(len);
      out->write((void*)name,len);
    }
    else out->write_16(0);


  out->mark_section("golgotha object type info");
  for (i=0; i<=g1_last_object_type; i++)
  {
    int han=out->mark_size();
    if (g1_object_type_array[i])
      g1_object_type_array[i]->save(out);      
    out->end_mark_size(han);
  }


  out->mark_section(G1_SECTION_OBJECTS_V1);
  out->write_32(t);

  // if objects change at a base level (g1_object_class) then a new section
  // should be created here
  out->mark_section(G1_SECTION_OBJECT_BASE_V1);
  for (i=0; i<t; i++)
  {
    out->mark_section(G1_SECTION_OBJECT_LIST + i);
    out->write_16(olist[i]->id);
    olist[i]->save(out);
  }
}

void g1_map_class::save(g1_saver_class *out, w32 sections)
{
  w32 i,j,k, tobjs=0;

  out->mark_section("li_type_info");
  li_save_type_info(out,0);
  
  // need to save the width and height of the map if saves cells or verts
  if (sections & (G1_MAP_CELLS | G1_MAP_VERTS))
  {
    out->mark_section(G1_SECTION_MAP_DIMENSIONS_V1);
    out->write_16(width());
    out->write_16(height());
  }

  if (sections & G1_MAP_SKY)
    save_sky(out);


  if (sections & G1_MAP_TICK)
  {
    out->mark_section(G1_SECTION_TICK);
    out->write_32(get_tick());
  }


  if (sections & G1_MAP_CELLS)
  {
    // this so if the tiles get moved around after save, we can match them back up
    out->mark_section(G1_SECTION_TILE_MATCHUP_V1);
    out->write_32(g1_tile_man.total());

    for (i=0; i<g1_tile_man.total(); i++)
      out->write_32(g1_tile_man.get(i)->filename_checksum);

    g1_save_map_cells(cells, width() * height(), out);
  }

  if (sections & G1_MAP_VERTS)
    g1_save_map_verts(verts, (w+1)*(h+1), out, i4_T);

  if (sections & G1_MAP_SELECTED_VERTS)
  {
    out->mark_section("golgotha selected verts");
    
    int l=(w+1)*(h+1);
    g1_map_vertex_class *v=verts;

    for (i=0; i<l; i++, v++)
      if (v->need_undo())
      {
        out->write_16(i);
        g1_save_map_verts(v, 1, out, i4_F);
      }

    out->write_16(0xffff);
  }
        
  if (sections & G1_MAP_PLAYERS)
  {
    out->mark_section(G1_SECTION_PLAYER_INFO);
    g1_player_man.save(out);
  }

  if (sections & G1_MAP_OBJECTS)
    save_objects(out);


  if (sections & G1_MAP_LIGHTS)
    g1_lights.save(out);


  if ((sections & G1_MAP_MOVIE) && current_movie)
  {
    out->mark_section(G1_SECTION_MOVIE);
    current_movie->save(out);    
  }

//   if ((sections & G1_MAP_CRITICAL_POINTS) && critical_graph)
//   {
//     out->mark_section(G1_SECTION_CRITICAL_POINTS_V1);
//     critical_graph->save_points(out);
//   }

//   if ((sections & G1_MAP_CRITICAL_DATA) && critical_graph)
//   {
//     out->mark_section(G1_SECTION_CRITICAL_GRAPH_V1);
//     critical_graph->save_graph(out);

//     out->mark_section(G1_SECTION_CRITICAL_MAP_V1);
//     save_critical_map(out);
//   }

  if (sections & G1_MAP_VIEW_POSITIONS)
    g1_cwin_man->save_views(out);

  for (g1_map_data_class *md=g1_map_data_class::first; md; md=md->next)
    md->save(out, sections);

}


struct tile_matchup
{
  w16 old_tile_number;
  w32 old_checksum;
};

int tile_matchup_compare(const void *a, const void *b)
{
  if ( ((tile_matchup *)a)->old_checksum<((tile_matchup *)b)->old_checksum)
    return -1;
  else if ( ((tile_matchup *)a)->old_checksum>((tile_matchup *)b)->old_checksum)
    return 1;
  else
    return 0;
}

// this will create a remap array which maps the tiles saved previously into the
// currently tile, so we can added and remove tiles from our list without screwing
// up previously saved maps
static w16 *g1_map_get_tile_remap(g1_loader_class *fp)   // returns 0 on failure
{
  if (!fp->goto_section(G1_SECTION_TILE_MATCHUP_V1))
    return 0;
  w32 i,t=fp->read_32();    // how many tile types there were when sved

  tile_matchup *tl=(tile_matchup *)i4_malloc(t*sizeof(tile_matchup), "tile matchup");
  for (i=0; i<t; i++)
  {
    tl[i].old_tile_number=i;
    tl[i].old_checksum=fp->read_32();
  }

  // sort so we can do a binary search for faster matchup
  qsort(tl,  t, sizeof(tile_matchup), tile_matchup_compare);

  w16 *remap=(w16 *)i4_malloc(t*sizeof(w16), "tile remap");
  memset(remap, 0, t*sizeof(w16));

  for (i=0; i< g1_tile_man.total(); i++)
  {
    w32 find_id=g1_tile_man.get(i)->filename_checksum,cur;
    w32 lo=0,hi=t,mid;
    i4_bool done=i4_F;
    mid=(hi+lo+1)/2;

    do
    {
      cur=tl[mid].old_checksum;
      if (cur==find_id)
      {
        done=i4_T;
        remap[tl[mid].old_tile_number]=i;
      }
      else
      {
        if (cur<find_id)
          lo=mid;
        else hi=mid;

        w32 last_mid=mid;
        mid=(hi+lo)/2;
        if (last_mid==mid)
          done=i4_T;
      }
        
    } while (!done);
  }
  i4_free(tl);
  return remap;
}

i4_bool g1_map_class::load_sky(g1_loader_class *fp)
{  
  if (sky_name)
    delete sky_name;

  if (fp->goto_section(G1_SECTION_SKY_V1))
    sky_name=fp->read_counted_str();    

  return i4_T;
}

void g1_map_class::save_sky(g1_saver_class *fp)
{
  fp->mark_section(G1_SECTION_SKY_V1); 

  if (sky_name)    
    fp->write_counted_str(*sky_name);
}


struct g1_sorted_obj_struct
{
  const char *name;
  w16 type;
};

int g1_sorted_obj_struct_compare(const g1_sorted_obj_struct *a, const g1_sorted_obj_struct *b)
{
  return strcmp(a->name, b->name);
}


extern w32 g1_num_objs_in_view; // from map_fast

//returns a list of objects
g1_object_class **g1_map_class::load_objects(g1_loader_class *fp, w32 &tobjs)
{
  int i;
  tobjs=0;
  g1_object_class **obj_list=0;

  g1_object_type *o_remap=0;

  for (i=0; i<G1_MAX_PLAYERS; i++)
      g1_player_man.get(i)->owned_objects.clear();

  if (fp->goto_section(G1_SECTION_OBJECT_TYPES_V1))
  {
    i4_array<g1_sorted_obj_struct> sorted_objs(g1_last_object_type+1, 0);
    for (i=0; i<=g1_last_object_type; i++)
      if (g1_object_type_array[i])
      {
        g1_sorted_obj_struct o;
        o.name=g1_object_type_array[i]->name();
        o.type=i;
        sorted_objs.add(o);
      }
      
    sorted_objs.sort(g1_sorted_obj_struct_compare);

    int old_total=fp->read_32();
    o_remap=(g1_object_type *)i4_malloc(sizeof(g1_object_type) * old_total, "obj remap");

    char name[512];
    for (i=0; i<old_total; i++) 
    {
      int nlen=fp->read_16();

      if (nlen)
      {
        fp->read(name, nlen);

        g1_sorted_obj_struct o;
        o.name=name;
        
        int find=sorted_objs.binary_search(&o, g1_sorted_obj_struct_compare);
        if (find==-1)
        {
          i4_warning("No match for old object type %s", name);
          o_remap[i]=-1;
        }
        else
          o_remap[i]=sorted_objs[find].type;
      } 
      else 
        o_remap[i]=-1;
    }

    if (fp->goto_section("golgotha object type info"))
    {
      for (i=0; i<old_total; i++)
      {
        w32 size=fp->read_32();
        if (o_remap[i]==-1)             // skip over types we don't know about
          fp->seek(fp->tell()+size);
        else
          g1_object_type_array[o_remap[i]]->load(fp);
      }
    }
  }
  

  if (fp->goto_section(G1_SECTION_OBJECTS_V1))
  {
    think_head=think_tail=0;

    // reset the global id list
    g1_global_id.init();
    g1_global_id.claim_freespace();

  

    if (!o_remap)
      o_remap=g1_get_old_object_type_remap();

    tobjs=fp->read_32();
    //tobjs=0;
    if (tobjs)
    {
      w16 bomber_type = g1_get_object_type("bomber"); //(OLI) bomber hack for demo

      obj_list=(g1_object_class **)i4_malloc(sizeof(g1_object_class *) * tobjs, "tmp object list");
    
      fp->set_remap(tobjs);
      if (fp->goto_section(G1_SECTION_OBJECT_BASE_V1))
      {
        for (i=0; i<tobjs; i++)
        {        
          fp->goto_section(G1_SECTION_OBJECT_LIST + i);
          sw16 obj_type=o_remap[fp->read_16()];
      
          if (obj_type==bomber_type)    //(OLI) bomber hack for demo
            obj_type=-1;

          if (obj_type>=0 && obj_type<=g1_last_object_type && g1_object_type_array[obj_type])
          {
            g1_object_class *o=g1_object_type_array[obj_type]->create_object(obj_type,fp);

            g1_map_piece_class *mp = g1_map_piece_class::cast(o);            

            if (o->global_id==g1_global_id.invalid_id())
            {
              i4_warning("object has invalid id for itself");
              delete o;
              o=0;
            }
//             else if (mp)
//             {
//               delete o;
//               o=0;
//             }
            else
            if (o->player_num>=G1_MAX_PLAYERS)
            {
              i4_warning("map load : object '%s' deleted, bad player number",
                         g1_object_type_array[obj_type]->name());
              delete o;
              o=0;
            } else if (o->x<0 || o->y<0 || o->x>=width() || o->y>=height() || o->id!=obj_type)
            {
              i4_warning("map load : object '%s' deleted, off map or loaded wrong",
                         g1_object_type_array[obj_type]->name());
              delete o;
              o=0;
            }
            
            obj_list[i]=o;
          }
          else 
          {
            obj_list[i]=0;
          }
        }
      }
      fp->end_remap();
      // now the object will convert all of there refernces to other objects from w16 to
      // pointers
      
      fp->set_helpers(obj_list, tobjs);
  
      fp->convert_references();
    }
 
  }
    
  if (o_remap) 
    i4_free(o_remap);
  
  return obj_list;

}

// returns the sections that were actually read
w32 g1_map_class::load(g1_loader_class *fp, w32 sections)
{
  sw32 i, ret=0;
  g1_object_class **obj_list=0;

  g1_map_class *old_current=g1_current_map_PRIVATE;
  g1_set_map(this);

  w32 tobjs=0;
  sw32 lw,lh;
  i4_bool load_cells=(sections & G1_MAP_CELLS)!=0;
  i4_bool load_verts=(sections & G1_MAP_VERTS)!=0;

  if (sections & G1_MAP_RES_FILENAME && fp->goto_section("golgotha map res filename"))
  {
    i4_str *res = fp->read_counted_str();
    //(OLI) what do we do with this string?
    delete res;
  }

  if (fp->goto_section("li_type_info"))
    fp->li_remap=li_load_type_info(fp,0);

  if (sections & (G1_MAP_CELLS | G1_MAP_VERTS))
  {
    if (!fp->goto_section(G1_SECTION_MAP_DIMENSIONS_V1))
      return 0;
 
    lw=fp->read_16();
    lh=fp->read_16();  

    // must load cells and verts together if new load    
    if (load_cells != load_verts &&
        (cells!=0 || verts!=0) &&
        (lw!=w  || lh!=h))
    {
      i4_warning("trying to merge in cells or verts of differnt map size");
      sections &= ~(G1_MAP_CELLS | G1_MAP_VERTS);   // don't try to load these
    }
  }


  if (sections & G1_MAP_SKY)
  {
    if (load_sky(fp))
      ret|=G1_MAP_SKY; 
  }

  if ((sections & G1_MAP_TICK) && (fp->goto_section(G1_SECTION_TICK)))
  {
    g1_tick_counter=fp->read_32();
    ret|=G1_MAP_TICK;
  }
 

  g1_object_class *olist[G1_MAX_OBJECTS];
  sw32 t_old_objects=0;

  if (cells)  // if old cells in map get all the objects off the map before proceeding
  {
    t_old_objects=make_object_list(olist, G1_MAX_OBJECTS);
    for (i=0; i<t_old_objects; i++)
      olist[i]->unoccupy_location();
  }
  
  if (sections & G1_MAP_CELLS)    
  {
    if (cells)
    {
      i4_free(cells);
      cells=0;
    }
    
    if (load_cells == load_verts)
    {
      w=lw;
      h=lh;
      g1_map_width=w;
      g1_map_height=h;
    }

    int a_size=w * h *sizeof(g1_map_cell_class);
    cells=(g1_map_cell_class *)i4_malloc(a_size,"map");
    g1_cells=cells;
    memset(cells, 0, a_size);

    w16 *tile_remap=g1_map_get_tile_remap(fp);
    if (!tile_remap)
      return i4_F;

    if (g1_load_map_cells(cells, w*h, tile_remap, fp))
      ret|=G1_MAP_CELLS;


    i4_free(tile_remap);
    
    recalc|=G1_RECALC_PAD_LIST;
  }



  if (sections & G1_MAP_VERTS)
  {
    if (verts)
      i4_free(verts);

    int size=(lw+1)*(lh+1)*sizeof(g1_map_vertex_class);
    verts=(g1_map_vertex_class *)i4_malloc(size,"");
    g1_verts=verts;

    g1_load_map_verts(verts, (lw+1)*(lh+1), fp, 1);
    ret|=G1_MAP_VERTS;
  }
  else if (sections & G1_MAP_SELECTED_VERTS)
  {  
    if (fp->goto_section("golgotha selected verts"))
    {
      i=fp->read_16();
      while (i!=0xffff)
      {
        g1_load_map_verts(verts+i, 1, fp, 0);
        i=fp->read_16();
      }

      ret|=G1_MAP_SELECTED_VERTS;
    }
  }
   
  if ((sections & G1_MAP_PLAYERS) && fp->goto_section(G1_SECTION_PLAYER_INFO))
  {
    if (g1_player_man.load(fp)) 
      ret|=G1_MAP_PLAYERS;
  }

  if (sections & G1_MAP_OBJECTS)  
  {
    for (i=0; i<t_old_objects; i++)
    {
      request_remove(olist[i]);
      g1_remove_man.process_requests();        
    }
    t_old_objects=0;


    obj_list=load_objects(fp, tobjs);
    ret|=G1_MAP_OBJECTS;    
  }

  if (sections & G1_MAP_LIGHTS)
  {
    if (g1_lights.load(fp))
      ret|=G1_MAP_LIGHTS;
  }

  if ((sections & G1_MAP_MOVIE) && (fp->goto_section(G1_SECTION_MOVIE)))
  {
    if (current_movie)
      delete current_movie;

    current_movie=g1_load_movie_flow(fp);


    ret|=G1_MAP_MOVIE;
  }

  if (sections & G1_MAP_VIEW_POSITIONS)
    g1_cwin_man->load_views(fp);
  else if (sections & G1_MAP_OBJECTS)
    g1_cwin_man->load_views(0);

  if (obj_list)
  {
    for (i=0; i<tobjs; i++)
      if (obj_list[i])
      {
        g1_object_class *o=obj_list[i];

        o->occupy_location();
        o->grab_old();          // is this ok?  currently, this is needed to get all ground
                                //   rolls and pitches correct in the level
        
        if (o->get_flag(g1_object_class::THINKING))
          request_think(o);

        g1_player_man.get(o->player_num)->add_object(o->global_id);
      }
 
    fp->convert_references();    
    fp->set_helpers(0,0);   // make sure no one tries to use this later  

    // validate everyone's data, especially if other objects have been deleted
    for (i=0; i<tobjs; i++)
      if (obj_list[i])
        obj_list[i]->validate();

    i4_free(obj_list);
  }
  else if (fp->references_were_loaded())
    i4_error("could not convert references because objects not loaded");

  // add previously removed object into the map if we aren't loading them later
  if (t_old_objects)
  {
    if (sections & G1_MAP_OBJECTS)  // delete these object, they were supposedly loaded
    {
      for (i=0; i<t_old_objects; i++)
      {
        request_remove(olist[i]);
        g1_remove_man.process_requests();        
      }
    }
    else  // otherwise add them back into the map
    {

      for (i=0; i<t_old_objects; i++)
        olist[i]->occupy_location();
    }
  }

  for (g1_map_data_class *md=g1_map_data_class::first; md; md=md->next)
    md->load(fp, sections);


  g1_map_vertex_class *v[4];
  i4_3d_vector v0,v1,v2,v3,vec1,vec2,vec3;
  sw32 cx,cy;

  for (cy=0; cy<height(); cy++)
  for (cx=0; cx<width();  cx++)
  {    
    g1_map_cell_class *cell = cells+cx+cy*width();

    v[0] = verts + cx + cy * (width()+1);       //v[0]   v[1]
    v[1] = v[0]+1;                             //v[3]   v[2] 
    v[2] = v[1]+width()+1;
    v[3] = v[2]-1;

    v0.x = cx;
    v0.y = cy;
    v0.z = v[0]->get_height();

    v1.x = cx+1;
    v1.y = cx;
    v1.z = v[1]->get_height();

    v2.x = cx+1;
    v2.y = cy+1;
    v2.z = v[2]->get_height();

    v3.x = cx;
    v3.y = cy+1;
    v3.z = v[3]->get_height();

    vec1.x = v1.x - v0.x;
    vec1.y = v1.y - v0.y;
    vec1.z = v1.z - v0.z;

    vec2.x = v2.x - v0.x;
    vec2.y = v2.y - v0.y;
    vec2.z = v2.z - v0.z;

    vec3.cross(vec1,vec2);

    i4_float dist = vec3.dot(v0);

    if (fabs(vec3.dot(v3) - dist) < 0.00000001) cell->flags |= g1_map_cell_class::PLANAR;
  }

  recalc_static_stuff();
  tick_time.get();

  g1_set_map(old_current);

  if (fp->li_remap)
  {
    li_free_type_info(fp->li_remap);
    fp->li_remap=0;
  }


  return i4_T;
}

void g1_map_class::reload()
{
  i4_file_class *in=i4_open(*filename);
  if (in)
  {
    g1_loader_class *l=g1_open_save_file(in);
    if (l)
    {
      load(l, G1_MAP_ALL);
      delete l;
    }
  }
}

// void g1_map_class::save_critical_map(g1_saver_class *f)
// {
//   int i,j,k;
//   g1_map_cell_class *c=cell(0,0);

//   for (j=0; j<height(); j++)
//     for (i=0; i<width(); i++, c++)
//       f->write(c->nearest_critical, sizeof(c->nearest_critical));
// }

// void g1_map_class::load_critical_map(g1_loader_class *f)
// {
//   int i,j,k;
//   g1_map_cell_class *c=&cell(0,0);

//   for (j=0; j<height(); j++)
//     for (i=0; i<width(); i++, c++)
//       f->read(c->nearest_critical, sizeof(c->nearest_critical));
// }

