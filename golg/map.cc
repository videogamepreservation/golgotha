/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "player.hh"
#include "error/error.hh"
#include "g1_limits.hh"
#include "image/color.hh"
#include "map.hh"
#include "window/win_evt.hh"
#include "device/kernel.hh"
#include "objs/model_id.hh"
#include "image/image.hh"
#include "loaders/load.hh"
#include "init/init.hh"
#include "resources.hh"
#include "g1_object.hh"
#include "objs/map_piece.hh"
#include "sound_man.hh"
#include "tile.hh"
#include "resources.hh"
#include "remove_man.hh"
#include "math/pi.hh"
#include "g1_speed.hh"
#include "light.hh"
#include "m_flow.hh"
#include "statistics.hh"
#include "input.hh"
#include "time/profile.hh"
#include "g1_render.hh"
#include "height_info.hh"
#include "checksum/checksum.hh"
#include "solvemap_astar.hh"
#include "cwin_man.hh"
#include "map_light.hh"
#include "map_man.hh"
#include "lisp/li_class.hh"
#include "objs/path_object.hh"
#include "objs/vehic_sounds.hh"
#include "map_cell.hh"
#include "map_vert.hh"
#include "tick_count.hh"
#include "map_view.hh"
#include "map_data.hh"

g1_height_info::g1_height_info()
{ 
  floor_height   = 0.f;
  ceiling_height = 1.f;
  flags = BELOW_GROUND;  
}


i4_profile_class pf_map_think("map.cc think objects");
i4_profile_class pf_map_post_think("map.cc post_think objects");
i4_profile_class pf_notify_target_position_change("notify_target_pos_change");

g1_statistics_counter_class g1_stat_counter;

w32 g1_map_class::get_tick()
{
  return g1_tick_counter;
}
    
g1_map_cell_class *g1_map_class::cell(w16 x, w16 y) const { return cells + y*w + x; }
g1_map_cell_class *g1_map_class::cell(w32 offset) const  { return cells + offset; }
g1_map_vertex_class *g1_map_class::vertex(w16 x, w16 y) const { return verts + y*(w+1) + x; }


void g1_map_class::add_object(g1_object_chain_class &c, w32 x, w32 y)
{
  cells[c.offset = y*w+x].add_object(c);
}

void g1_map_class::remove_object(g1_object_chain_class &c)
{ 
  cells[c.offset].remove_object(c);
}

i4_float g1_map_class::min_terrain_height(w16 x, w16 y)
{
  g1_map_vertex_class *v1,*v2,*v3,*v4;
    
  v1 = verts+ x + y * (w+1);
  v2 = v1+1;
  v3 = v1+w+1;
  v4 = v3+1;
    
  return g1_vertex_min(v1, g1_vertex_min(v2, g1_vertex_min(v3, v4)))->get_height();
}

// only use this if you know what you are doing
void g1_map_class::change_map(int _w, int _h,
                              g1_map_cell_class *_cells, g1_map_vertex_class *_vertex)
{
  if (cells)
    i4_free(cells);
  
  if (verts)
    i4_free(verts);

  cells=_cells;
  verts=_vertex;
  w=_w;
  h=_h;
}


void g1_map_class::remove_object_type(g1_object_type type)
{
  int i,j,k;
  g1_map_cell_class *c=cell(0,0);
  g1_object_chain_class *cell, *l;
  g1_object_class *obj;

  for (j=0; j<height(); j++)
    for (i=0; i<width(); i++, c++)
    {
      for (cell=c->get_obj_list(); cell;)
      {
        l=cell;
        cell=cell->next;
        obj=l->object;
        if (obj->id==type)
        {          
          obj->unoccupy_location();
          obj->request_remove();
        }
      }
    }
}


g1_map_class::g1_map_class(const i4_const_str &fname)
{
  recalc=0xffffffff;     // recalculate everything until further notice

  sky_name=0;

//   lod=0;
  //  lod=(g1_map_lod_cell *) i4_malloc(sizeof(g1_map_lod_cell) * MAX_MAP_LOD, "lod");

  filename=0;
  set_filename(fname);

  post_cell_draw=0;

  w=0;
  h=0;


  think_head=think_tail=0;

  current_movie=0;
  
  cells=0;
  verts=0;

  //  map=(g1_map_cell_class *)i4_malloc(w*h*sizeof(g1_map_cell_class),"map");
  //  vert=(g1_map_vertex_class *)i4_malloc((w+1)*(h+1)*sizeof(g1_map_vertex_class),"map vert");

  w32 x,count=w*h,y;
  
  solver = new g1_astar_map_solver_class;
//   critical_graph=0;
  movie_in_progress=i4_F;
}

g1_map_class::~g1_map_class()
{
  g1_stop_sound_averages();

  g1_map_class *old_current=g1_current_map_PRIVATE;
  g1_set_map(this);


  
  for (g1_map_data_class *md=g1_map_data_class::first; md; md=md->next)
    md->free();


  if (filename)
    delete filename;

  if (sky_name)
    delete sky_name;

  if (cells)
  {
    g1_object_class *olist[G1_MAX_OBJECTS];
    w32 ids[G1_MAX_OBJECTS];

    sw32 t=make_object_list(olist, G1_MAX_OBJECTS), i;

    // can't use object pointers directly because one
    // object might delete another
    for (i=0; i<t; i++)
      ids[i]=olist[i]->global_id;
      

    for (i=0; i<t; i++)
    {
      if (g1_global_id.check_id(ids[i]))
      {
        g1_object_class *o=g1_global_id.get(ids[i]);        
        o->unoccupy_location();
        o->request_remove();
        g1_remove_man.process_requests();        
      }
    }
    i4_free(cells);
  }
    

  if (verts)
    i4_free(verts);

  if (current_movie)
    delete current_movie;
  
  if (solver)
    delete solver;
  


  if (old_current==this)
    g1_set_map(0);
  else
    g1_set_map(old_current);

}

void check_olist()
{
  g1_object_class *olist[G1_MAX_OBJECTS];
  sw32 t=g1_get_map()->make_object_list(olist, G1_MAX_OBJECTS);
}

// returns total added
sw32 g1_map_class::make_object_list(g1_object_class **buffer, sw32 buf_size)  
{
  int x=width()*height(), i;
  sw32 t=0;
  g1_map_cell_class *c=cells;
  if (buf_size==0) return 0;


  for (i=0; i<x; i++, c++)
    if (c->object_list)
    {
      for (g1_object_chain_class *obj=c->get_obj_list(); obj; obj=obj->next)
      {
        g1_object_class *o=obj->object;
        if (!o->get_flag(g1_object_class::SCRATCH_BIT))   /// make sure object only added once
        {
          o->set_flag(g1_object_class::SCRATCH_BIT, 1);
          buffer[t++]=o;
          if (t==buf_size) 
          {
            i=x;
            obj=0;
          }
        }
      }
    }

  for (i=0; i<t; i++)
    buffer[i]->set_flag(g1_object_class::SCRATCH_BIT, 0);

  return t;
}

sw32 g1_map_class::make_selected_objects_list(w32 *buffer, sw32 buf_size)
{
  int x=width()*height(), i;
  sw32 t=0;
  g1_map_cell_class *c=cells;
  if (buf_size==0) return 0;


  for (i=0; i<x; i++, c++)
    if (c->object_list)
    {
      for (g1_object_chain_class *obj=c->get_obj_list(); obj; obj=obj->next)
      {
        g1_object_class *o=obj->object;

        // make sure object only added once
        if (o->selected() && !o->get_flag(g1_object_class::SCRATCH_BIT))   
        {
          o->set_flag(g1_object_class::SCRATCH_BIT, 1);
          buffer[t++]=o->global_id;
          if (t==buf_size) 
          {
            i=x;
            obj=0;
          }
        }
      }
    }

  for (i=0; i<t; i++)
    g1_global_id.get(buffer[i])->set_flag(g1_object_class::SCRATCH_BIT, 0);

  return t;
}

void g1_map_class::think_objects()
{
  li_class *old_this=li_this;

  recalc_static_stuff();


  pf_map_think.start();
  g1_input.que_keys(tick_time);


  w32 i,h;
  
  w32 start_tail = think_tail;
  w32 start_head = think_head;
  
  //do the think()'s  
  i = start_tail;
  h = start_head;

  g1_reset_sound_averages();

  pf_map_think.start();

  for (; i!=h; )
  {
    g1_object_class *o=think_que[i];

    //always check to make sure the pointer
    //is good. objects might have removed themselves
    //from the map while still in the que, and left
    //a null pointer in their place
    if (o)
    {
      li_this=o->vars;
      o->grab_old();
    }

    i++;
    if (i>=THINK_QUE_SIZE)
      i=0;
  }

  i = start_tail;
  h = start_head;
  for (; i!=h; )
  {
    g1_object_class *o=think_que[i];

    //always check to make sure the pointer
    //is good. objects might have removed themselves
    //from the map while still in the que, and left
    //a null pointer in their place
    if (o)
    {
      li_this=o->vars;
      o->set_flag(g1_object_class::THINKING, 0);
      o->think();
    }

    i++;
    if (i>=THINK_QUE_SIZE)
      i=0;
  }

  pf_map_think.stop();


  pf_map_post_think.start();
  
  //do the post_think()'s
  i = start_tail;
  h = start_head;

  for (; i!=h; )
  {
    g1_object_class *o=think_que[i];    

    think_tail++;
    if (think_tail>=THINK_QUE_SIZE)
      think_tail=0;

    //always check to make sure the pointer
    //is good. objects might have removed themselves
    //from the map while still in the que, and left
    //a null pointer in their place
    if (o)
    {
      li_this=o->vars;
      o->post_think();
    }

    i++;
    if (i>=THINK_QUE_SIZE)
      i=0;
  }
  pf_map_post_think.stop();

  g1_recalc_sound_averages();


  g1_tick_counter++;
  tick_time.add_milli((1000/G1_HZ));


  li_this=old_this;
  pf_map_think.stop();
  
}


void g1_map_class::damage_range(g1_object_class *obj,
                                i4_float x, i4_float y, i4_float z, 
                                i4_float range, w16 damage, i4_float falloff)
// damage to vehicles centers at x,y,z and falls off by falloff*damage at range distance
//   from the center of the damage range
{
  sw32 ix,iy, 
    sx = sw32(x-range),
    ex = sw32(x+range)+1,
    sy = sw32(y-range),
    ey = sw32(y+range)+1;

  // clip region
  if (sx<0) sx=0;
  if (ex>width()) ex=width();
  if (sy<0) sy=0;
  if (ey>height()) ey=height();

  // get first one
  g1_map_cell_class *c;
  g1_object_chain_class *objlist;
  i4_float dist,dx,dy,dz;

  range *= range;
  falloff *= i4_float(damage)/range;

  for (iy=sy; iy<ey; iy++)
  {
    c = cell(sx,iy);
    for (ix=sx; ix<ex; ix++, c++)
    {
      for (objlist=c->get_obj_list(); objlist; objlist=objlist->next)
      {
        g1_object_class *hurt_obj=objlist->object;

        if (objlist==&hurt_obj->occupied_squares[0]) // make sure object is only hurt once
        {
          if (hurt_obj->get_flag(g1_object_class::TARGETABLE))
// && hurt_obj->player_num!=obj->player_num)
          {
            dx=x-hurt_obj->x;
            dy=y-hurt_obj->y;
            dz=z-hurt_obj->h;
            dist = dx*dx+dy*dy+dz*dz;
            if (dist<range)
              hurt_obj->damage(obj, damage - (int)(falloff*range), i4_3d_vector (0,0,g1_resources.gravity));
          }
        }
      }
    }
  }
}



g1_object_class *g1_map_class::find_object_by_id(w32 object_id,
                                                 g1_player_type prefered_team)
{
  sw32 i,j=width()*height();
  g1_map_cell_class *c=cells;
  g1_object_chain_class *o;
  g1_object_class *best=0;

  for (i=0; i<j; i++, c++)
  {
    for (o=c->get_obj_list(); o; o=o->next)
    {
      if (o->object->id==object_id)
      {
        if (o->object->player_num==prefered_team)              
          return o->object;
        else
          best=o->object;
      }
    }
  }
  return best;
}


void g1_map_class::remove_from_think_list(g1_object_class *obj)
{
  w32 i = think_tail,
      h = think_head;
  
  for (; i!=h;)
  {
    if (think_que[i]==obj) think_que[i]=0;
      
    i++;
    if (i>=THINK_QUE_SIZE) 
      i=0;
  }
}

void g1_map_class::request_remove(g1_object_class *obj)
{
  obj->flags |= g1_object_class::DELETED;

  if (obj->flags & g1_object_class::THINKING)
  {
    obj->stop_thinking();
  }
  
  g1_remove_man.request_remove(obj);
}


i4_const_str g1_map_class::get_filename()
{
  return *filename;
}

void g1_map_class::set_filename(const i4_const_str &fname)
{
  if (filename)
    delete filename;
  filename=new i4_str(fname);
}




void g1_map_class::recalc_static_stuff()
{
  i4_bool reset_time=i4_F;

//   if (recalc & (G1_RECALC_BLOCK_MAPS | G1_RECALC_CRITICAL_DATA))
//     li_call("add_undo", li_make_list(new li_int(G1_MAP_CRITICAL_DATA), 0));
    
  if (recalc & G1_RECALC_STATIC_LIGHT)
    g1_calc_static_lighting();               // defined in light.cc


//   if (recalc & G1_RECALC_BLOCK_MAPS)    
//   {
//     make_block_maps();
//     reset_time=i4_T;
//   }

  if (recalc & G1_RECALC_RADAR_VIEW)
  {
//     init_lod();   //(OLI) need to put this in the proper place

    g1_radar_recalculate_backgrounds();
    reset_time=i4_T;
  }

  if (recalc & (G1_RECALC_WATER_VERTS))
  {
    g1_map_vertex_class *v=verts+(w+1)+1;
    g1_map_cell_class *c=cells+(w)+1;

    for (int y=1; y<h-1; y++)
    {
      for (int x=1; x<w-1; x++, c++, v++)
      {
        if ((g1_tile_man.get(c[0].type)->flags & g1_tile_class::WAVE) &&
            (g1_tile_man.get(c[-1].type)->flags & g1_tile_class::WAVE) &&
            (g1_tile_man.get(c[-w].type)->flags & g1_tile_class::WAVE) &&
            (g1_tile_man.get(c[-w-1].type)->flags & g1_tile_class::WAVE))
          v->flags |= g1_map_vertex_class::APPLY_WAVE_FUNCTION;
      }
      v+=2;
      c+=2;
      v++;      
    }

    reset_time=i4_T;
  }
  
  recalc=0;

  if (reset_time)
    tick_time.get();     // don't simulate ticks for the calculation stuff
}


void g1_map_class::range_iterator::begin(float x, float y, float range)
//{{{
{
  g1_map_class *map = g1_get_map();
  int wx = map->width(), wy = map->height();

  left   = i4_f_to_i(x - range); if (left<0)      left=0;
  right  = i4_f_to_i(x + range); if (right>wx-1)  right=wx-1;
  top    = i4_f_to_i(y - range); if (top<0)       top=0;
  bottom = i4_f_to_i(y + range); if (bottom>wy-1) bottom=wy-1;

  ix = right;
  iy = top-1;
  cell = 0;
  chain = 0;

  object_mask_flags=0xffffffff;
  type_mask_flags=0xffffffff;
}
//}}}

void g1_map_class::range_iterator::safe_restart()
//{{{
{
  chain=0;
}
//}}}

i4_bool g1_map_class::range_iterator::end()
//{{{
{
  return (iy>=bottom);
}
//}}}

void g1_map_class::range_iterator::next()
//{{{
{
  return;
  /*
  g1_object_class *o;

  if (chain) chain = chain->next;

  while (iy<bottom)
  {
    if (!chain)
    {
      ++ix;
      ++cell;
      if (ix<=right)
        chain = cell->get_obj_list();
      else
      {
        ix = left;
        iy++;
        cell = &g1_get_map()->cell(left, iy);
      }
    }
    else
    {
      o = chain->object;
      if (&o->occupied_squares[0]!=chain ||
          (o->flags & object_mask_flags)==0 ||
          (g1_object_type_array[o->id]->flags & type_mask_flags)==0)
        chain = chain->next;
      else
        return;
    }
  }
  */
}
//}}}

sw32 g1_map_class::get_objects_in_range(float x, float y, float range,
                                        g1_object_class *dest_array[], w32 array_size,
                                        w32 object_mask_flags, w32 type_mask_flags)
{
  sw32 x_left,x_right,y_top,y_bottom;
  
  x_left   = i4_f_to_i(x - range); if (x_left<0)     x_left=0;
  x_right  = i4_f_to_i(x + range); if (x_right>w-1)  x_right=w-1;
  y_top    = i4_f_to_i(y - range); if (y_top<0)      y_top=0;
  y_bottom = i4_f_to_i(y + range); if (y_bottom>h-1) y_bottom=h-1;
  
  sw32 ix,iy;  
  sw32 num_found=0;
  
  for (iy=y_top;  iy<=y_bottom; iy++)
  {
    g1_map_cell_class *cell = g1_get_map()->cell(x_left, iy);

    for (ix=x_left; ix<=x_right;  ix++, cell++)
    {      
      g1_object_chain_class *p     = cell->get_solid_list();

      while (p && num_found<array_size)
      {    
        g1_object_class *o = p->object;

        if (o && (&o->occupied_squares[0]==p) && (o->flags & object_mask_flags))
        {
          if (g1_object_type_array[o->id]->flags & type_mask_flags)
          {
            dest_array[num_found] = p->object;
            num_found++;
          }
        }
      
        p = p->next_solid();
      }
    
      if (num_found >= array_size)
        break;
    }
    
    if (num_found >= array_size)
        break;
  }
  
  return num_found;
}
