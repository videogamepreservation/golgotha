/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "controller.hh"
#include "map_cell.hh"
#include "resources.hh"
#include "math/pi.hh"
#include "sound_man.hh"
#include "map.hh"
#include "poly/poly.hh"
#include "objs/stank.hh"
#include "g1_render.hh"
#include "time/profile.hh"
#include "sky.hh"
#include "r1_clip.hh"
#include "g1_tint.hh"

static i4_profile_class pf_draw_overhead("draw_overhead");

void g1_object_controller_class::draw_overhead(g1_draw_context_class *context)
{
  w32 i;
  pf_draw_overhead.start();

#if 0
  i4_polygon_class vp;

  g1_camera_info_struct *camera=view.get_camera();
  
  if (view.get_view_mode()==G1_ACTION_MODE)
  {  
    i4_float a=i4_pi()/3.9,vd=g1_resources.visual_radius(),vb=3;
  
    i4_float cos_gma=cos(camera->ground_rotate-a),
      sin_gma=sin(camera->ground_rotate-a),
      cos_gpa=cos(camera->ground_rotate+a),
      sin_gpa=sin(camera->ground_rotate+a);

    vp.vert[0].v = i4_3d_point_class(camera->gx - cos_gma*vb, camera->gy - sin_gma*vb, 0);
    vp.vert[1].v = i4_3d_point_class(camera->gx + cos_gpa*vd, camera->gy + sin_gpa*vd, 0);
    vp.vert[2].v = i4_3d_point_class(camera->gx + cos_gma*vd, camera->gy + sin_gma*vd, 0);
    vp.vert[3].v = i4_3d_point_class(camera->gx - cos_gpa*vb, camera->gy - sin_gpa*vb, 0);    

    i4_float c0x,c0y,c1x,c1y,c2x,c2y,c3x,c3y,c4x,c4y;

    c0x=camera->gx - cos_gma*vb;  
    c0y=camera->gy - sin_gma*vb;

    c1x=camera->gx + cos_gpa*vd;
    c1y=camera->gy + sin_gpa*vd;

    c2x=camera->gx + cos_gma*vd;
    c2y=camera->gy + sin_gma*vd;

    c3x=camera->gx - cos_gpa*vb;  
    c3y=camera->gy - sin_gpa*vb;

    vp.vert[0].v = i4_3d_point_class(c0x, c0y, 0);
    vp.vert[1].v = i4_3d_point_class(c1x, c1y, 0);
    vp.vert[2].v = i4_3d_point_class(c2x, c2y, 0);
    vp.vert[3].v = i4_3d_point_class(c3x, c3y, 0);
  }    
  else
  {        
    sw32 top=-40,left=-40,bot=height()+40,right=width()+40;
    i4_float dx1,dy1, dx2,dy2;
    i4_3d_vector v0,v1,v2,v3, v3_1, v2_0;

    if (!view_to_game(right, bot, v2.x, v2.y, dx1,dy1))
      v2.set(camera->gx, camera->gy, 0);
    v2.z=0; 

    if (!view_to_game(left,  bot, v3.x, v3.y, dx2,dy2))
      v2.set(camera->gx, camera->gy, 0);
    v3.z=0;

    if (!view_to_game(left,  top, v0.x, v0.y, dx1,dy1) ||
        view.get_view_mode()!=G1_STRATEGY_MODE)
      v0.set(v3.x+dx1*g1_resources.visual_radius(), v3.y+dy1*g1_resources.visual_radius(), 0);
    v0.z=0;

    if (!view_to_game(right, top, v1.x, v1.y, dx2,dy2) || 
        view.get_view_mode()!=G1_STRATEGY_MODE)
      v1.set(v2.x+dx2*g1_resources.visual_radius(), v2.y+dy2*g1_resources.visual_radius(), 0);
    else
    {
      
    }

    v1.z=0;

    v3_1=i4_3d_vector(v1.x - v3.x, v1.y - v3.y,  0);  v3_1.normalize();
    v2_0=i4_3d_vector(v0.x - v2.x, v0.y - v2.y, 0);   v2_0.normalize();

    if ((v3_1.x==0 &&  v3_1.y==0 && v3_1.z==0) ||
        (v2_0.x==0 &&  v2_0.y==0 && v2_0.z==0))
    {
      //i4_warning("uh oh....\n"); i hate this fucking warning -Trey
      return ;
    }
    
    // move these back
    v2.x -= v2_0.x *1.5;   v2.y -= v2_0.y * 2.0;
    v3.x -= v3_1.x *1.5;   v3.y -= v3_1.y * 2.0;
      

    // move these forward
    v0.x += v2_0.x * 1.5;   v0.y += v2_0.y * 2.0;
    v1.x += v3_1.x * 1.5;   v1.y += v3_1.y * 2.0;


    vp.vert[0].v = v0;
    vp.vert[1].v = v1;
    vp.vert[2].v = v2;
    vp.vert[3].v = v3;  

  } 

  vp.t_verts=4;  

  g1_visible_cell cell_list[3000];
 
  float xscale = g1_render.scale_x;
  float yscale = g1_render.scale_y;

  int t_vis=get_map()->calc_visible(*context->transform, &vp, cell_list, 3000, xscale, yscale);
  
//   if (get_track())
//   {
//     sw32 x,y,sx,ex,sy,ey;
//     sx = i4_f_to_i(get_track()->x)-5; ex = sx + 10;
//     sy = i4_f_to_i(get_track()->y)-5; ey = sy + 10;

//     for (x=sx; x<ex; x++)
//     for (y=sy; y<ey; y++)
//     {
//       if (x<0 || y<0 || x>=map->width() || y>=map->height()) continue;

//       map->cell(x,y)->flags |= g1_map_cell_class::VISIBLE;
//     }
//   }
 
  r1_far_clip_z=1000;
  if (view.get_view_mode()!=G1_STRATEGY_MODE)
    g1_draw_sky(this, *camera,  transform, context);

  if (view.get_view_mode()==G1_ACTION_MODE ||
      view.get_view_mode()==G1_FOLLOW_MODE ||
      view.get_view_mode()==G1_WATCH_MODE)
    r1_far_clip_z=(float)(g1_resources.visual_radius() * 0.707106 - 2.0);

                               
  get_map()->fast_draw_cells(context, cell_list, t_vis);
#endif
  r1_far_clip_z=250.0;
  g1_camera_info_struct *camera=view.get_camera();

  if (g1_hurt_tint>0)
    g1_render.r_api->set_color_tint(g1_hurt_tint_handles[g1_hurt_tint]);

  if (view.get_view_mode()!=G1_STRATEGY_MODE)
    g1_draw_sky(this, *camera,  transform, context);

  get_map()->calc_map_lod(this);
  get_map()->fast_draw_cells(context);

  if (view.get_view_mode()!=G1_STRATEGY_MODE)
    g1_render.r_api->set_color_tint(0);

  pf_draw_overhead.stop();
}

