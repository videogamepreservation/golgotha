/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "objs/goal.hh"
#include "object_definer.hh"
#include "lisp/li_init.hh"
#include "lisp/li_class.hh"
#include "li_objref.hh"
#include "map_man.hh"
#include "map.hh"
#include "saver.hh"

#include <stdio.h>

static g1_object_type stank;

static void g1_goal_init()
{
  stank = g1_get_object_type("stank");
}

g1_object_definer<g1_goal_class> 
g1_goal_def("goal", g1_object_definition_class::EDITOR_SELECTABLE, g1_goal_init);

static li_int_class_member type("type");
static li_int_class_member ticks_to_think("ticks_to_think");
static li_int_class_member think_delay("think_delay");

static li_float_class_member range("range");

g1_goal_class::g1_goal_class(g1_object_type id, g1_loader_class *fp)
  : g1_object_class(id, fp)
{
  draw_params.setup("lightbulb");
}

void g1_goal_class::save(g1_saver_class *fp)
{
  g1_object_class::save(fp);
}

void g1_goal_class::draw(g1_draw_context_class *context)
{  
  g1_editor_model_draw(this, draw_params, context);
}

void g1_goal_class::think()
{
  if (ticks_to_think())
    ticks_to_think()--;
  else
  {
#if 0
    for (int i=0; i<G1_MAX_PLAYERS; i++)
      power[i] = 0;

    ticks_to_think() = think_delay();

    g1_map_class::range_iterator p;
    
    p.begin(x,y,range());
    p.mask(g1_object_class::DANGEROUS);
    
    while (!p.end())
#else
    g1_object_class *objs[1024], **p;

    int num = g1_get_map()->get_objects_in_range(x, y, range(), 
                                                 objs, 1024, g1_object_class::DANGEROUS);
    p = objs;
    for (int i=0; i<num; i++)
#endif
    {
      g1_object_class *o = *p;
      int n= o->player_num;

      if (o->id == stank)
        power[n] += 10;
      else
        power[n]++;

      ++p;
    }
  }
  request_think();
}
