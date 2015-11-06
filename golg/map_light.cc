/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "map_man.hh"
#include "map.hh"
#include "cwin_man.hh"
#include "light.hh"
#include "map_vert.hh"
#include "map_cell.hh"

void g1_calc_static_lighting()
{
  if (!g1_map_is_loaded())
    return ;

  g1_object_class *olist[G1_MAX_OBJECTS];
  sw32 t=g1_get_map()->make_object_list(olist, G1_MAX_OBJECTS);
  int i;
  for (i=0; i<t; i++)
    olist[i]->unoccupy_location();

  g1_map_vertex_class *v=g1_get_map()->vertex(0,0);
  int mw=g1_get_map()->width(), mh=g1_get_map()->height();

  int x,y;
  for (y=0; y<=mh; y++)
    for (x=0; x<=mw; x++, v++)
    {
      v->light_sum=0x80000000;  // mark for recalc
      v->normal=0x8000;    // mark for recalc
    }

  g1_map_cell_class *c=g1_get_map()->cell(0,0);
  for (y=0; y<mh; y++)
    for (x=0; x<mw; x++, c++)
    {
      c->bottom_right_normal=0x8000;
      c->top_left_normal=0x8000;
    }


  v=g1_get_map()->vertex(0,0);
  for (y=0; y<=mh; y++)
    for (x=0; x<=mw; x++, v++)
    {
      float r,g,b;
      v->static_intensity=i4_f_to_i(g1_lights.directional_intensity * 255.0);
      v->get_rgb(r,g,b, x,y);  // this will force it to be recalculated
    }

  for (i=0; i<t; i++)
    olist[i]->occupy_location();
}
