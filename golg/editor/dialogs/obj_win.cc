/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "editor/dialogs/obj_win.hh"
#include "obj3d.hh"
#include "math/pi.hh"
#include "image/image.hh"
#include "draw_context.hh"
#include "window/win_evt.hh"
#include "device/kernel.hh"
#include "image/color.hh"
#include "g1_object.hh"
#include "remove_man.hh"
#include "g1_render.hh"
#include "r1_api.hh"
#include "editor/e_state.hh"
#include "editor/mode/e_object.hh"
#include "resources.hh"
#include "window/style.hh"
#include "app/app.hh"
#include "lisp/li_class.hh"

void g1_3d_object_window::do_idle()
{
  if (object.valid())
  {    
    if (!context_help_window.get())
    {
      i4_graphical_style_class *style=i4_current_app->get_style();
      i4_const_str help_str=i4gets((char *)object->name(),i4_F);
      if (!help_str.null())
        context_help_window=style->create_quick_context_help(x(), y()+height()+5, help_str);
      else
        context_help_window=style->create_quick_context_help(x(), y()+height()+5,
                                                         i4_const_str((char *)object->name()));
    }
  }
    
}

void g1_3d_object_window::set_object_type(g1_object_type type, 
                                          w16 _array_index)
{
  if (object.valid())
  {
    object->request_remove();
    g1_remove_man.process_requests();
    object=0;
  }

  if (type>=0)
  {
    object=g1_create_object(type);
    if (object.valid())
    {
      object->x=object->lx=object->y=object->ly=object->h=object->lh=0;
      object->player_num=g1_edit_state.current_team;
    }
  }

  object_type=type;
  array_index=_array_index;
  request_redraw(i4_F);
}

g1_3d_object_window::g1_3d_object_window(w16 w, w16 h,
                                         g1_object_type obj_type,
                                         w16 _array_index,
                                         g1_3d_pick_window::camera_struct &camera,
                                         i4_image_class *active_back,
                                         i4_image_class *passive_back,
                                         i4_event_reaction_class *reaction)
  :  g1_3d_pick_window(w,h,
                       active_back, passive_back,
                       camera,
                       reaction)
{
  object=0;
  set_object_type(obj_type, _array_index);
}


void g1_3d_object_window_ambient(i4_transform_class *object_to_world,
                                 i4_float &ar, i4_float &ag, i4_float &ab)
{
  ar = 1.0;
  ag = 1.0;
  ab = 1.0;
}


void g1_3d_object_window::draw_object(g1_draw_context_class *context)
{
  if (object.valid())
  {
    r1_render_api_class *render_api=g1_render.r_api;

    i4_transform_class spare_transform;
    object->world_transform = &spare_transform;

    object->player_num=g1_edit_state.current_team;
    object->calc_world_transform(g1_render.frame_ratio);
    
    //setup an ambient function that returns 1.0
    g1_get_ambient_function_type last_ambient_func = g1_render.get_ambient;
    g1_render.get_ambient = g1_3d_object_window_ambient;

    g1_render.r_api->clear_area(0,0, width()-1, height()-1, 0, g1_far_z_range());

      
    li_class_context c(object->vars);
    object->draw(context);
    
    //put the old ambient function back
    g1_render.get_ambient = last_ambient_func;

    
  }
  else
    local_image->clear(0, *context->context);
}

i4_bool g1_3d_object_window::selected()
{
  return (i4_bool)(g1_e_object.get_object_type() == object_type);
}

void g1_3d_object_window::do_press()
{
  if (object_type>=0 && g1_object_type_array[object_type])
  {
    g1_edit_state.hide_focus();

    g1_e_object.set_object_type(object_type);

    if (strcmp(g1_edit_state.major_mode, "OBJECT"))
      g1_edit_state.set_major_mode("OBJECT");

    if (g1_e_object.get_minor_mode() != g1_object_params::ADD)
      g1_edit_state.set_minor_mode("OBJECT", g1_object_params::ADD);

    g1_edit_state.show_focus();
  }
}
