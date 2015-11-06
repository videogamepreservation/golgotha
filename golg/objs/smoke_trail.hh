/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef SMOKE_TRAIL_HH
#define SMOKE_TRAIL_HH

#include "g1_object.hh"
#include "player_type.hh"
#include "path.hh"

class g1_solid_class;
class g1_map_piece_class;
class g1_map_class;

/*
  To create a smoke trail call :

    s=(g1_smoke_trail_class *)g1_create_object(G1_SMOKE_TRAIL);
    if (s)
    {
      s->setup(x, y, h, 0.01, 0.2, 0xffd000, 0xffffff);   // orange to white
      s->occupy_location(map);
      smoke_trail.reference_object(0,s);
    }


  when you (the head of the smoke trail, move, call :
    if (s)
      s->update_head(map, x,y,h);

  when you die call :
    if (s)    // delete the smoke trail
    {
      s->unoccupy_location(map);
      map->request_remove(s);
    }

 */

class g1_smoke_trail_class : public g1_object_class
{
public:
  enum { DATA_VERSION=1 };
  enum { TAIL_LENGTH=4 };
  i4_3d_point_class tspots[TAIL_LENGTH];

  w32 last_draw_frame;
  w32 sc, ec;
  i4_float sw,ew;
  w16 ticks_advanced;

  virtual i4_float occupancy_radius() const 
  {
    float w=(tspots[0].x-tspots[TAIL_LENGTH-1].x);
    float h=(tspots[0].y-tspots[TAIL_LENGTH-1].y);

    w = (w<0)? w=-w : w;
    h = (h<0)? h=-h : h;

    return w>h ? w : h;
  }


  virtual void setup(i4_float start_x, i4_float start_y, i4_float start_h,
             i4_float start_width, i4_float end_width,
             i4_color start_color, i4_color end_color);

  virtual void update_head(i4_float nx, i4_float ny, i4_float nh);

  g1_smoke_trail_class(g1_object_type id, g1_loader_class *fp);
  virtual void save(g1_saver_class *fp);
  virtual void draw(g1_draw_context_class *context);
  virtual void think();
};

#endif
