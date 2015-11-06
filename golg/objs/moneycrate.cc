/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "objs/model_id.hh"
#include "objs/model_draw.hh"
#include "resources.hh"
#include "saver.hh"
#include "map_cell.hh"
#include "map.hh"
#include "map_man.hh"
#include "objs/moneyplane.hh"
#include "object_definer.hh"
#include "objs/moneycrate.hh"
#include "lisp/li_class.hh"

static g1_model_ref model_ref("moneycrate");
static li_float_class_member li_vspeed("vspeed");
static li_int_class_member li_value("crate_value");

static void g1_moneycrate_init()
{
}

g1_object_definer<g1_moneycrate_class>
g1_moneycrate_def("moneycrate",
                  g1_object_definition_class::EDITOR_SELECTABLE |
                  g1_object_definition_class::MOVABLE);

g1_moneycrate_class::g1_moneycrate_class(g1_object_type id,
                                         g1_loader_class *fp)
  : g1_object_class(id,fp)
{
  draw_params.setup(model_ref.id());

  set_flag(BLOCKING, 1);
}

i4_3d_vector g1_moneycrate_class::crate_attach()
{
  i4_3d_vector crate_offset(0,0,0);
  model_ref()->get_mount_point("mount",crate_offset);
  return crate_offset;
}

void g1_moneycrate_class::setup(i4_3d_vector pos, w32 value)
{
  x = pos.x;
  y = pos.y;
  h = pos.z;
  grab_old();

  li_class_context c(vars);
  
  li_value() = value;
  li_vspeed() = 1.0;

  occupy_location();
  request_think();
}

void g1_moneycrate_class::follow(i4_3d_vector pos, i4_3d_vector rot)
{
  unoccupy_location();
  x = pos.x;
  y = pos.y;
  h = pos.z;
  theta = rot.z;
  pitch = rot.y;
  roll = rot.x;

  li_class_context c(vars);
  li_vspeed() = 1.0;
  occupy_location();
  request_think();
}

void g1_moneycrate_class::release()
{
  li_class_context c(vars);
  li_vspeed() = 0;
  request_think();
}

w32 g1_moneycrate_class::value() const
{
  li_class_context c(vars);
  return li_value();
}

void g1_moneycrate_class::think()
{
  i4_float vspeed = li_vspeed();
  if (vspeed<=0.0)
  {
    i4_float height = g1_get_map()->map_height(x,y,h);
    if (h+vspeed-0.001 > height)
    {
      h += vspeed;
      vspeed -= g1_resources.gravity;    
      request_think();
    }
    else
    {
      h = height;      
      vspeed = 0;

      unoccupy_location();
      request_remove();
    }
  }
  li_vspeed() = vspeed;
}

