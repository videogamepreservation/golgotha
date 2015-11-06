/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "g1_object.hh"
#include "math/num_type.hh"
#include "math/random.hh"
#include "object_definer.hh"
#include "map.hh"
#include "map_man.hh"
#include "g1_render.hh"
#include "r1_api.hh"
#include "g1_texture_id.hh"
#include "r1_clip.hh"
#include "objs/miracle.hh"
#include "sound_man.hh"
#include "sfx_id.hh"

g1_object_definer<g1_miracle_class>
g1_miracle_def("miracle", g1_object_definition_class::EDITOR_SELECTABLE);

g1_miracle_class::g1_miracle_class(g1_object_type id, 
                                   g1_loader_class *fp)
  : g1_object_class(id,fp)
{    
  
  miracle_sound = g1_sound_man.alloc_dynamic_3d_sound(g1_sfx_misc_electric_car_charged);
  electric_arcs = new i4_array<arc_point_list_struct>(4,4);
}

void g1_miracle_class::save(g1_saver_class *fp)
{
  g1_object_class::save(fp);

  fp->start_version(DATA_VERSION);

  fp->end_version();
}

g1_miracle_class::~g1_miracle_class()
{
  //make sure the sound gets deallocated
  if (miracle_sound)
  {
    g1_sound_man.free_dynamic_3d_sound(miracle_sound);
    miracle_sound = 0;
  }    
}

void g1_miracle_class::draw(g1_draw_context_class *context)
{  
  sw32 i,j;

  if (!electric_arcs)
    return;

  for (i=0; i<electric_arcs->size(); i++)
  {
    arc_point_struct *points = (*electric_arcs)[i].arc_points;
    
    i4_3d_point_class pts[NUM_ARC_POINTS];

    for (j=0; j<NUM_ARC_POINTS; j++)
    {
      pts[j].interpolate(points[j].lposition, points[j].position, g1_render.frame_ratio);
    }
  
    g1_render.add_translucent_trail(context->transform,
                                    pts,
                                    NUM_ARC_POINTS,
                                    0.02f   , 0.02f,
                                    1.f     , 1.f, 
                                    0xFFFFFF, 0xFFFFFF);

  }
}

void g1_miracle_class::think()
{
}

void g1_miracle_class::setup(g1_convoy_class *convoy_to_attack, w32 _miracle_type, i4_3d_vector pos)
{
  x = pos.x;
  y = pos.y;
  h = pos.z;
  
  grab_old();

  occupy_location();

  convoy       = convoy_to_attack;
  miracle_type = _miracle_type;
  
  health       = 10;
  
  request_think();
}

void g1_miracle_class::post_think()
{
  if (health < 0 || !convoy.valid() || !electric_arcs)
  {
    if (electric_arcs)
    {
      delete electric_arcs;
      electric_arcs = 0;
    }

    unoccupy_location();
    request_remove();
    return;
  }
  
  request_think();

  health--;

  sw32 i;

  sw32 num_needed = 0;
  for (i=0; i<g1_convoy_class::CONVOY_SIZE; i++)
  {
    g1_map_piece_class *mp = convoy->get_member(i);
    if (mp)
    {
      num_needed++;
            
      arc_point_list_struct *points;
      
      if (electric_arcs->size() < num_needed)
      {
        points = electric_arcs->add();
        new_arc_points(i4_3d_vector(x,y,h+3),
                       i4_3d_vector(mp->x,mp->y,mp->h),
                       points->arc_points);
        copy_old_points(points->arc_points);
      }
      else
      {
        points = &(*electric_arcs)[num_needed-1];
        
        copy_old_points(points->arc_points);
        new_arc_points(i4_3d_vector(x,y,h+3),
                       i4_3d_vector(mp->x,mp->y,mp->h),
                       points->arc_points);
      }
    }
  }
}

void g1_miracle_class::copy_old_points(arc_point_struct *points)
{
  sw32 i,j;
  
  for (j=0; j<NUM_ARC_POINTS; j++)
  {
    points[j].lposition = points[j].position;
  }
}

void g1_miracle_class::new_arc_points(i4_3d_vector src, i4_3d_vector dest, arc_point_struct *points) 
{
  i4_float rx,ry,rz,px,py,pz;
  i4_float map_point_height;
  
  i4_3d_vector vec1;

  vec1 = dest;
  vec1 -= src;

  sw32 i;

  for (i=0; i<NUM_ARC_POINTS; i++)
  {
    //ARC #1
    i4_float random_scale = (1 - (i/(NUM_ARC_POINTS-1)))*3;
    
    rx = (g1_float_rand(6) * 0.4 - 0.2) * random_scale;
    ry = (g1_float_rand(4) * 0.4 - 0.2) * random_scale;
    rz = (g1_float_rand(2) * 0.4 - 0.2) * random_scale;
    
    px = src.x + vec1.x*i/(NUM_ARC_POINTS-1) + rx;
    py = src.y + vec1.y*i/(NUM_ARC_POINTS-1) + ry;
    pz = src.z + vec1.z*i/(NUM_ARC_POINTS-1) + rz;
    
    points[i].position = i4_3d_vector(px,py,pz);
  }
}
