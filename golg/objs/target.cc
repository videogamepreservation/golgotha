/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "objs/target.hh"
#include "object_definer.hh"
#include "map.hh"
#include "map_man.hh"

g1_object_type g1_target_type;

static void g1_target_init();

g1_object_definer<g1_target_class>
g1_target_def("target", 0, g1_target_init);

static void g1_target_init()
{
  g1_target_type = g1_target_def.type;
}

g1_target_class::g1_target_class(g1_object_type id, g1_loader_class *fp)
  : g1_object_class(id, fp)
{
  draw_params.setup("target");
  
  player_num=0;
  set_flag(TARGETABLE    |
           GROUND,
           1);
  
}

i4_bool g1_target_class::occupy_location()
{
  h=g1_get_map()->terrain_height(x,y);
  lh=h;
  return g1_object_class::occupy_location();
}

void g1_target_class::draw(g1_draw_context_class *context)
{
  g1_model_draw(this, draw_params, context);
}

void g1_target_class::setup(i4_float _x, i4_float _y)
{
  lx = x = _x;
  ly = y = _y;

  occupy_location();
}

