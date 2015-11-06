/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "objs/model_draw.hh"
#include "math/transform.hh"
#include "g1_object.hh"
#include "draw_context.hh"
#include "objs/model_id.hh"
#include "math/pi.hh"
#include "math/angle.hh"
#include "g1_render.hh"
#include "g1_texture_id.hh"
#include "r1_api.hh"
#include "r1_clip.hh"
#include "objs/map_piece.hh"
#include "resources.hh"
#include "lisp/lisp.hh"
#include "map_man.hh"
#include "map.hh"
#include "tick_count.hh"
#include "controller.hh"

void g1_model_draw_parameters::setup(g1_quad_object_class *_model, 
                                     g1_quad_object_class *_shadow_model,
                                     g1_quad_object_class *_lod_model)
{
  model=_model;
  shadow_model=_shadow_model;
  lod_model=_lod_model;
}


float g1_model_draw_parameters::extent() const    // gets extents of model_id from model_draw
{
  if (model)
    return model->extent;
  else
    return 0;
}

void g1_model_draw_parameters::setup(w16 _model_id, w16 _shadow_model, w16 _lod_model)
{
  model=g1_model_list_man.get_model(_model_id);
  
  if (shadow_model)
    shadow_model=g1_model_list_man.get_model(_model_id);
  else
    shadow_model=0;

  if (_lod_model)
    lod_model=g1_model_list_man.get_model(_lod_model);
  else
    lod_model=0;
}

void g1_model_draw_parameters::setup(const char *model_name, char *shadow_model_name,
                                     char *lod_model_name)
{
  int id=g1_model_list_man.find_handle(model_name);
  model = g1_model_list_man.get_model(id);
    
  if (shadow_model_name)
  {
    id=g1_model_list_man.find_handle(shadow_model_name);
    if (id)
      shadow_model = g1_model_list_man.get_model(id);
    else
      shadow_model = 0;
  }
  else
    shadow_model = 0;

  if (lod_model_name)
  {
    id=g1_model_list_man.find_handle(lod_model_name);
    if (id)
      lod_model = g1_model_list_man.get_model(id);
    else
      lod_model = 0;
  }
  else lod_model=0;

}

static li_symbol_ref team_icons("team_icons");

void g1_model_draw(g1_object_class *_this,
                   g1_model_draw_parameters &params,
                   g1_draw_context_class *context)
{  
  i4_3d_vector cpos;

  if (!g1_current_controller.get())
    return;

  g1_current_controller->get_pos(cpos);

  float dist_sqrd=(cpos.x-_this->x)*(cpos.x-_this->x)+
    (cpos.y-_this->y)*(cpos.y-_this->y)+
    (cpos.z-_this->h)*(cpos.z-_this->h);


  if (dist_sqrd>g1_resources.lod_disappear_dist)
    return ;

  if (dist_sqrd>g1_resources.lod_switch_dist && !params.lod_model)
    return ;

  g1_screen_box *bbox=0;
  
  i4_transform_class view_transform;

  i4_transform_class *old = context->transform;
  context->transform = &view_transform;
  
  view_transform.multiply(*old,*(_this->world_transform));
    
  if (_this->get_flag(g1_object_class::SELECTABLE | g1_object_class::TARGETABLE))
  {
    if (g1_render.current_selectable_list)
      bbox=g1_render.current_selectable_list->add();
    if (bbox)
    {
      bbox->x1 = 2048;
      bbox->y1 = 2048;
      bbox->x2 = -1;
      bbox->y2 = -1;
      bbox->z1 = 999999;
      bbox->z2 = -999999;
      bbox->w  = 1.0/999999;
      bbox->object_id = _this->global_id;
    }
  }

  int max_health=_this->get_type()->defaults->health;
  int damage_level;
  
  if (_this->health >= max_health)
    damage_level = 7;
  else
    if (_this->health < 0)
      damage_level = 0;
    else
      damage_level = i4_f_to_i(7.f * _this->health / (float)max_health);
    
  g1_render.set_render_damage_level(damage_level);


  if (dist_sqrd<g1_resources.lod_switch_dist)
  {
    if (params.shadow_model)
    {
      g1_quad_object_class *model = params.shadow_model;
    
      g1_render.r_api->disable_texture();
      g1_render.r_api->set_shading_mode(R1_CONSTANT_SHADING);
      g1_render.r_api->set_alpha_mode(R1_ALPHA_CONSTANT);
      g1_render.r_api->set_constant_color(0x7F000000);
      g1_render.r_api->set_write_mode(R1_COMPARE_W | R1_WRITE_COLOR);

      i4_transform_class t=*(_this->world_transform), shadow_view_transform;
      // drop shadow to ground    
      t.t.z=g1_get_map()->map_height(_this->x, _this->y, _this->h);

      shadow_view_transform.multiply(*old,t);
      
      
      g1_render.render_object_polys(model,
                                    &shadow_view_transform,
                                    params.frame);

      g1_render.r_api->set_shading_mode(R1_COLORED_SHADING);
      g1_render.r_api->set_alpha_mode(R1_ALPHA_DISABLED);
      g1_render.r_api->set_write_mode(R1_WRITE_W | R1_COMPARE_W | R1_WRITE_COLOR);
    }

    g1_quad_object_class *model = params.model;
    if (model)
    {
      if (!(params.flags & g1_model_draw_parameters::SUPPRESS_SPECIALS) && model->num_special>0)
        model->update(i4_float(g1_tick_counter + _this->global_id) + g1_render.frame_ratio);

      g1_render.render_object(model,
                              &view_transform,                          
                              params.flags & g1_model_draw_parameters::NO_LIGHTING ? 
                              0 : _this->world_transform,
                              1,                   
                              _this->player_num,
                              params.frame,
                              bbox,
                              0);

      for (int i=0;i<_this->num_mini_objects;i++) 
      {
        _this->mini_objects[i].draw(context,
                                    _this->world_transform,
                                    bbox,
                                    _this->player_num);      
      }
    }
  }
  else
  {
    g1_render.r_api->set_filter_mode(R1_NO_FILTERING);

    g1_render.render_object(params.lod_model,
                            &view_transform,                          
                            params.flags & g1_model_draw_parameters::NO_LIGHTING ? 
                            0 : _this->world_transform,
                            1,                   
                            _this->player_num,
                            params.frame,
                            bbox,
                            0);

    for (int i=0;i<_this->num_mini_objects;i++) 
    {
      _this->mini_objects[i].draw(context,
                                  _this->world_transform,
                                  bbox,
                                  _this->player_num,
                                  0, i4_T, i4_T);
    }

    g1_render.r_api->set_filter_mode(R1_BILINEAR_FILTERING);

  }

  g1_render.set_render_damage_level(-1);

  context->transform = old;
}

void g1_editor_model_draw(g1_object_class *_this,
                          g1_model_draw_parameters &params,
                          g1_draw_context_class *context)
{
  if (context->draw_editor_stuff)
    g1_model_draw(_this, params, context);
}




