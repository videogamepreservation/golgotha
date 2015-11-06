/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "draw_context.hh"
#include "r1_clip.hh"
#include "math/transform.hh"
#include "objs/sprite.hh"
#include "g1_render.hh"
#include "r1_api.hh"


void g1_draw_flare(i4_3d_point_class world_pos,
                   g1_draw_context_class *context,
                   float flare_scale,
                   int type)
{
  i4_3d_vector screen_pos;
  context->transform->transform(world_pos, screen_pos);

 
  if (screen_pos.z > r1_near_clip_z)         
  {
    float center_x=g1_render.center_x, center_y=g1_render.center_y;
    i4_float ooz = r1_ooz(screen_pos.z);                   
    i4_float xs = center_x * ooz * g1_render.scale_x;
    i4_float ys = center_y * ooz * g1_render.scale_y;        


    float theta=atan2(screen_pos.y, screen_pos.x);    
    float dist = sqrt(screen_pos.y*screen_pos.y + screen_pos.x*screen_pos.x);


    g1_render.r_api->set_filter_mode(R1_BILINEAR_FILTERING);
    //g1_render.r_api->set_write_mode(R1_COMPARE_W | R1_WRITE_COLOR);


    float dists[5]={dist, 0.0, -0.3*dist, -0.7*dist, -1*dist };
    int sprite_types[5]={0, 1,1,1,1};
    float sprite_scales[5]={ 1.0, 0.3, 0.8, 0.5, 1.0};

    float cos_theta=cos(theta);
    float sin_theta=sin(theta);

    for (int i=0; i<5; i++)
    {
      float cx=center_x + cos_theta * dists[i] * xs;
      float cy=center_y + sin_theta * dists[i] * ys;

      g1_sprite_class *sprite=g1_sprite_list_man.get_sprite(sprite_types[i]);
      float w=sprite->texture_scale * sprite_scales[i] * center_x * 2 * flare_scale;

      r1_clip_render_textured_rect(i4_f_to_i(cx-w/2), i4_f_to_i(cy-w/2), 
                                   i4_f_to_i(cx+w/2), i4_f_to_i(cy+w/2), r1_near_clip_z, 1,
                                   i4_f_to_i(center_x*2), i4_f_to_i(center_y*2),
                                   sprite->texture,
                                   0,
                                   g1_render.r_api);
    }

    //g1_render.r_api->set_write_mode(R1_COMPARE_W | R1_WRITE_COLOR | R1_WRITE_W);
    g1_render.r_api->set_filter_mode(R1_NO_FILTERING);

  }
}



void g1_draw_exhaust(i4_3d_point_class world_pos,
                     g1_draw_context_class *context,
                     float flare_scale,
                     int type, float alpha)
{
  i4_3d_vector screen_pos;
  context->transform->transform(world_pos, screen_pos);

 
  if (screen_pos.z > r1_near_clip_z)         
  {
    float center_x=g1_render.center_x, center_y=g1_render.center_y;
    i4_float ooz = r1_ooz(screen_pos.z);                   

    g1_render.r_api->set_filter_mode(R1_BILINEAR_FILTERING);
    //g1_render.r_api->set_write_mode(R1_COMPARE_W | R1_WRITE_COLOR);


    float cx=center_x + screen_pos.x * center_x * ooz * g1_render.scale_x;
    float cy=center_y + screen_pos.y * center_x * ooz * g1_render.scale_x;

    g1_sprite_class *sprite=g1_sprite_list_man.get_sprite(0);
    float w=sprite->texture_scale * center_x * 2 * flare_scale;

    r1_clip_render_textured_rect(i4_f_to_i(cx-w/2), i4_f_to_i(cy-w/2), 
                                 i4_f_to_i(cx+w/2), i4_f_to_i(cy+w/2), screen_pos.z, alpha,
                                 i4_f_to_i(center_x*2), i4_f_to_i(center_y*2),
                                 sprite->texture,
                                 0,
                                 g1_render.r_api);

    //g1_render.r_api->set_write_mode(R1_COMPARE_W | R1_WRITE_COLOR | R1_WRITE_W);
    g1_render.r_api->set_filter_mode(R1_NO_FILTERING);
  }
}
