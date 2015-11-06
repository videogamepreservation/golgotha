/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "image/image.hh"
#include "palette/pal.hh"
#include "map.hh"
#include "math/transform.hh"
#include "map_cell.hh"
#include "poly/polydraw.hh"
#include "resources.hh"
#include "map_cell.hh"
#include "poly/poly.hh"
#include "tile.hh"
#include "g1_object.hh"
#include "statistics.hh"
#include "time/profile.hh"
#include "r1_api.hh"
#include "g1_render.hh"
#include "r1_clip.hh"
#include "height_info.hh"
#include "lisp/li_class.hh"
#include "tmanage.hh"
#include "map_cell.hh"
#include "map_vert.hh"
#include "draw_context.hh"
#include "camera.hh"

#if 0
static i4_profile_class pf_map_fast_draw_tri("map_fast::draw tri"),
  pf_map_fast_draw_terrain("map_fast::draw terrain"),
  pf_map_fast_draw_objects("map_fast::dra objects"),
  pf_map_fast_obj_list("map_fast::gen obj list"),
  pf_map_fast_clear("map_fast::clear structures"),
  pf_map_fast_clip_code("map_fast::clip codes"),
  pf_map_fast_pack_uv("map_fast::pack uv's"),
  pf_map_fast_transform("map_fast::transform"),
  pf_project("map_fast::project"),
  pf_pack_light("map_fast::pack light"),
  pf_backface_test("map_fast::backface test"),
  pf_map_fast_clip_tri("map_fast::clip tri"),
  pf_map_fast_use_texture("map_fast::use texture"),
  pf_map_fast_render("map_fast::render poly");


static i4_bool do_fog;
static r1_texture_ref g1_default_texture("tron_grid");

i4_profile_class pf_draw_objects("draw objects");

const w32          g1_max_objs_in_view = 256;
w32                g1_num_objs_in_view = 0;
w32 g1_objs_in_view[g1_max_objs_in_view];

i4_array<i4_transform_class> g1_obj_transforms_in_view(0, g1_max_objs_in_view);
class transform_killer_class : public i4_init_class
{
  //fix this trey!  can't have global i4_array's without something to clean it up.
  void uninit() { g1_obj_transforms_in_view.uninit(); }
} transform_killer;

struct g1_draw_tri_struct
{
  g1_map_vertex_class *v1;
  int vx1, vy1;

  g1_map_vertex_class *v2;
  int vx2, vy2;

  g1_map_vertex_class *v3;
  int vx3, vy3;

  r1_texture_handle texture;
  i4_float texture_scale;
};


r1_vert terrain_verts[4];
          
static r1_vert temp_buf_1[8],temp_buf_2[8];
static w16 indices[4] = {0,1,2,3};

inline void g1_draw_tri(g1_draw_tri_struct &ts, i4_transform_class &t, w16 cell_flags)
{
  g1_stat_counter.increment(g1_statistics_counter_class::TOTAL_POLYS);
  g1_stat_counter.increment(g1_statistics_counter_class::TERRAIN);

  
  int and_code=ts.v1->clip_code & ts.v2->clip_code & ts.v3->clip_code;

  if (and_code==0)     // at least one of the verts isn't clipped out
  {
    i4_3d_vector dot1,dot2, norm;
    dot1 = i4_3d_vector(ts.v2->v.x - ts.v1->v.x, ts.v2->v.y - ts.v1->v.y, ts.v2->v.z - ts.v1->v.z);
    dot2 = i4_3d_vector(ts.v3->v.x - ts.v1->v.x, ts.v3->v.y - ts.v1->v.y, ts.v3->v.z - ts.v1->v.z);
    norm.cross(dot1,dot2);

    i4_float facing=ts.v1->v.dot(norm);


    if (facing < 0.0001) 
    {
      int ins=0;

      pf_pack_light.start();

      if (do_fog)
      {
        if (!ts.v1->get_flag(g1_map_vertex_class::FOGGED))
          ts.v1->get_rgb(terrain_verts[0].r, terrain_verts[0].g, terrain_verts[0].b, ts.vx1, ts.vy1);
        else
          terrain_verts[0].r=terrain_verts[0].g=terrain_verts[0].b=0;
          
        if (!ts.v2->get_flag(g1_map_vertex_class::FOGGED))
          ts.v2->get_rgb(terrain_verts[1].r, terrain_verts[1].g, terrain_verts[1].b, ts.vx1, ts.vy1);
        else
          terrain_verts[1].r=terrain_verts[1].g=terrain_verts[1].b=0;

        if (!ts.v3->get_flag(g1_map_vertex_class::FOGGED))
          ts.v3->get_rgb(terrain_verts[2].r, terrain_verts[2].g, terrain_verts[2].b, ts.vx1, ts.vy1);
        else
          terrain_verts[2].r=terrain_verts[2].g=terrain_verts[2].b=0;
      }
      else
      {
        ts.v1->get_rgb(terrain_verts[0].r, terrain_verts[0].g, terrain_verts[0].b, ts.vx1, ts.vy1);
        ts.v2->get_rgb(terrain_verts[1].r, terrain_verts[1].g, terrain_verts[1].b, ts.vx1, ts.vy1);
        ts.v3->get_rgb(terrain_verts[2].r, terrain_verts[2].g, terrain_verts[2].b, ts.vx1, ts.vy1);
      }
      
      pf_pack_light.stop();

      pf_project.start();

      float closest_ooz=0.0001;      
      
      if (!ts.v1->clip_code)
      {
        ts.v1->project(g1_render.center_x, g1_render.center_y);
        terrain_verts[0].px  = ts.v1->px;
        terrain_verts[0].py  = ts.v1->py;
        terrain_verts[0].v.z = ts.v1->v.z;
        terrain_verts[0].w   = ts.v1->w;
        ins++;
      }
      if (ts.v1->w> closest_ooz) closest_ooz = ts.v1->w;
      
      if (!ts.v2->clip_code)
      {
        ts.v2->project(g1_render.center_x, g1_render.center_y);
        terrain_verts[1].px  = ts.v2->px;
        terrain_verts[1].py  = ts.v2->py;
        terrain_verts[1].v.z = ts.v2->v.z;
        terrain_verts[1].w   = ts.v2->w;        
        
        ins++;
      }
      if (ts.v2->w > closest_ooz) closest_ooz = ts.v2->w;
      
      if (!ts.v3->clip_code)
      {
        ts.v3->project(g1_render.center_x, g1_render.center_y);
        terrain_verts[2].px  = ts.v3->px;
        terrain_verts[2].py  = ts.v3->py;
        terrain_verts[2].v.z = ts.v3->v.z;
        terrain_verts[2].w   = ts.v3->w;        

        ins++;
      }
      if (ts.v3->w > closest_ooz) closest_ooz = ts.v3->w;
      if (closest_ooz>100) closest_ooz = 100;
      pf_project.stop();
      


      sw32 num_poly_verts = 3;
      r1_render_api_class *tmap=g1_render.r_api;

      if (g1_render.draw_mode==g1_render_class::WIREFRAME)
      {        
        tmap->set_constant_color(0x7f7f7f);
        tmap->disable_texture();
        tmap->set_shading_mode(R1_SHADE_DISABLED);

        terrain_verts[0].v.x = ts.v1->v.x;
        terrain_verts[0].v.y = ts.v1->v.y;
        terrain_verts[0].v.z = ts.v1->v.z;

        terrain_verts[1].v.x = ts.v2->v.x;
        terrain_verts[1].v.y = ts.v2->v.y;
        terrain_verts[1].v.z = ts.v2->v.z;

        terrain_verts[2].v.x = ts.v3->v.x;
        terrain_verts[2].v.y = ts.v3->v.y;
        terrain_verts[2].v.z = ts.v3->v.z;

        r1_vert v[4];
        v[0]=terrain_verts[0];
        v[1]=terrain_verts[1];
        v[2]=terrain_verts[2];
        v[3]=terrain_verts[0];

        r1_clip_render_lines(3, v, g1_render.center_x, g1_render.center_y, tmap);
        tmap->set_shading_mode(R1_WHITE_SHADING);

      }
      else
      {
        if (ins == 3)
        {
          sw32 texture_size=i4_f_to_i(ts.texture_scale * g1_render.center_x * closest_ooz * 0.5);

          if (g1_render.draw_mode!=g1_render_class::SOLID)
            // completely in view        
            tmap->use_texture(ts.texture, texture_size, 0);


          pf_map_fast_render.start();
          tmap->render_poly(num_poly_verts,terrain_verts);
          pf_map_fast_render.stop();
        }
        else
        {
          pf_map_fast_clip_tri.start();

          terrain_verts[0].v.x = ts.v1->v.x;
          terrain_verts[0].v.y = ts.v1->v.y;
          terrain_verts[0].v.z = ts.v1->v.z;
          
          terrain_verts[1].v.x = ts.v2->v.x;
          terrain_verts[1].v.y = ts.v2->v.y;
          terrain_verts[1].v.z = ts.v2->v.z;

          terrain_verts[2].v.x = ts.v3->v.x;
          terrain_verts[2].v.y = ts.v3->v.y;
          terrain_verts[2].v.z = ts.v3->v.z;

          r1_vert *clipped_poly = (tmap)->clip_poly(&num_poly_verts,
                                                    terrain_verts,
                                                    indices,
                                                    temp_buf_1,
                                                    temp_buf_2,
                                                    0);
          pf_map_fast_clip_tri.stop();

          if (clipped_poly && num_poly_verts>=3)
          {
            pf_project.start();
            for (sw32 j=0;j<num_poly_verts;j++)
            {        
              float ooz = r1_ooz(clipped_poly[j].v.z);
            
              clipped_poly[j].px = clipped_poly[j].v.x * 
                ooz * g1_render.center_x + g1_render.center_x;

              clipped_poly[j].py = clipped_poly[j].v.y * 
                ooz * g1_render.center_y + g1_render.center_y;

              clipped_poly[j].w  = ooz;

            }
            pf_project.stop();
          
            sw32 texture_size=i4_f_to_i(ts.texture_scale * g1_render.center_x * closest_ooz * 0.5);

            if (g1_render.draw_mode!=g1_render_class::SOLID)
              tmap->use_texture(ts.texture,  texture_size, 0);


            pf_map_fast_render.start();
            tmap->render_poly(num_poly_verts,clipped_poly);
            pf_map_fast_render.stop();
          }
        } 
      }
    }
  }
}


void g1_map_class::fast_draw_cells(g1_draw_context_class  *context)
{
  i4_transform_class t=*context->transform;
  
  g1_map_vertex_class *v=verts;

  sw32 i,cnt,cx,cy,mw_p1, w_xoff, w_yoff;
  mw_p1=width()+1;
  
  g1_map_vertex_class *vt[4];


  if (!g1_current_view_state())
    do_fog=i4_F;
  else
    do_fog=g1_current_view_state()->get_view_mode()==G1_STRATEGY_MODE ? i4_T : i4_F;

  r1_render_api_class *r_api = g1_render.r_api;
  
  
  //runs through the cells, determines which objects will be drawn, generates their
  //shadows if necessary
  g1_num_objs_in_view = 0;
  g1_obj_transforms_in_view.clear();

  
  g1_visible_cell *cell;

  //step through the cells, front to back  
  cell=cell_list+t_visible_cells-1;

  g1_map_cell_class *m=cells;



  pf_map_fast_obj_list.start();
  for (i=t_visible_cells-1; i>=0; i--, cell--)
  {
    cx=cell->x;
    cy=cell->y;
    sw16 cell_number=cy*width()+cx;
    g1_map_cell_class *map_cell=m+cell_number;

    for (g1_object_chain_class *o=map_cell->get_obj_list(); o; o=o->next)
    {
      g1_object_class *obj=o->object;

      if ((!do_fog ||
          ((map_cell->flags & g1_map_cell_class::FOGGED)==0)) &&
           !obj->get_flag(g1_object_class::SCRATCH_BIT))
      {
        g1_object_class *obj=o->object;

        g1_stat_counter.increment(g1_statistics_counter_class::OBJECTS);            
        if (g1_num_objs_in_view<g1_max_objs_in_view) 
        {              
          
          g1_objs_in_view[g1_num_objs_in_view] = o->object->global_id;
          o->object->world_transform = g1_obj_transforms_in_view.add();
          
          g1_num_objs_in_view++;
          
          //have the object update his transform
          o->object->calc_world_transform(g1_render.frame_ratio);
          
          o->object->set_flag(g1_object_class::SCRATCH_BIT, 1);
        }   
      }
    }          
  }
  pf_map_fast_obj_list.stop();


  if (g1_render.draw_mode==g1_render_class::SOLID)
    g1_render.r_api->use_texture(g1_default_texture.get(), 1, 0);
  
  pf_map_fast_draw_terrain.start();
  cell=cell_list;
  for (i=0; i<t_visible_cells; i++, cell++)
  {

    cx=cell->x;
    cy=cell->y;

    if (cx<0 || cy<0 || cx>=width() || cy>=height())
      i4_error("cell out of bounds");
    else
    {
      g1_map_cell_class *map_cell=m+cx+cy*width();


      r1_texture_handle han = g1_tile_man.get_texture(map_cell->type);
   
      if (han && han!=g1_tile_man.get_pink())
      {

        pf_map_fast_transform.start();

        g1_draw_tri_struct ts;
        vt[0]=v + cx + cy * mw_p1;          
        vt[1]=vt[0]+1;
        vt[2]=vt[1]+mw_p1;
        vt[3]=vt[2]-1;

        vt[0]->transform(t, cx, cy,     g1_render.scale_x,g1_render.scale_y);
        vt[1]->transform(t, cx+1, cy,   g1_render.scale_x,g1_render.scale_y);
        vt[2]->transform(t, cx+1, cy+1, g1_render.scale_x,g1_render.scale_y);
        vt[3]->transform(t, cx, cy+1,   g1_render.scale_x,g1_render.scale_y);
        pf_map_fast_transform.stop();



        pf_map_fast_clip_code.start();
        vt[0]->calc_clip_code();
        vt[1]->calc_clip_code();
        vt[2]->calc_clip_code();
        vt[3]->calc_clip_code();      
        pf_map_fast_clip_code.stop();


        g1_stat_counter.set_current_counter(g1_statistics_counter_class::TERRAIN_POLYS);

        pf_map_fast_pack_uv.start();
        int uv_on=map_cell->get_rotation();
        int uv_dir=map_cell->mirrored() ? 3 : 1;

        float u[4]={0,1,1,0},v[4]={1,1,0,0};

        terrain_verts[0].s = u[uv_on];
        terrain_verts[0].t = v[uv_on];
        uv_on=(uv_on+uv_dir)&3;

        terrain_verts[1].s = u[uv_on];
        terrain_verts[1].t = v[uv_on];
        uv_on=(uv_on+uv_dir)&3;

        terrain_verts[2].s = u[uv_on];
        terrain_verts[2].t = v[uv_on];
        pf_map_fast_pack_uv.stop();



        pf_map_fast_draw_tri.start();


        ts.v1 = vt[0];
        ts.v2 = vt[1];
        ts.v3 = vt[2];
        ts.texture = han;
        ts.texture_scale = 1;
        ts.vx1 = cx;
        ts.vy1 = cy;
        ts.vx2 = cx+1;
        ts.vy2 = cy;
        ts.vx3 = cx+1;
        ts.vy3 = cy+1;


        g1_draw_tri(ts, t, map_cell->flags);
        pf_map_fast_draw_tri.stop();



        pf_map_fast_pack_uv.start();
        uv_on=map_cell->get_rotation();
        terrain_verts[0].s = u[uv_on];
        terrain_verts[0].t = v[uv_on];

        uv_on=(uv_on+uv_dir*2)&3;
        terrain_verts[1].s = u[uv_on];
        terrain_verts[1].t = v[uv_on];

        uv_on=(uv_on+uv_dir)&3;
        terrain_verts[2].s = u[uv_on];
        terrain_verts[2].t = v[uv_on];
        pf_map_fast_pack_uv.stop();


        ts.v1 = vt[0];
        ts.v2 = vt[2];
        ts.v3 = vt[3];
        ts.vx1 = cx;
        ts.vy1 = cy;
        ts.vx2 = cx+1;
        ts.vy2 = cy+1;
        ts.vx3 = cx;
        ts.vy3 = cy+1;


        pf_map_fast_draw_tri.start();



        g1_draw_tri(ts, t, map_cell->flags);
        pf_map_fast_draw_tri.stop();


        if (post_cell_draw)
          post_cell_draw(cx,cy, post_cell_draw_context); 
      }       


    }
  }
  pf_map_fast_draw_terrain.stop();



  cell=cell_list;

  li_class *old_this=li_this;
  pf_map_fast_draw_objects.start();
  //draw the objects BACK TO FRONT
  for (i=g1_num_objs_in_view-1;i>=0;i--)
  {
    g1_object_class *o=g1_global_id.checked_get(g1_objs_in_view[i]);
    if (o)
    {
      li_this=o->vars;

      if (o->world_transform!=0)
      {
        o->draw(context);
        o->set_flag(g1_object_class::SCRATCH_BIT, 0);
      }
      else i4_warning("null transform");
    }
  }

  if (context->draw_editor_stuff)
  {
    for (i=g1_num_objs_in_view-1;i>=0;i--)
    {
      g1_object_class *o=g1_global_id.checked_get(g1_objs_in_view[i]);
      if (o)
      {
        li_this=o->vars;
        o->editor_draw(context);
      }
    }
  }  
  
  li_this=old_this;
    
  pf_map_fast_draw_objects.stop();

  pf_map_fast_clear.start();
  cell=cell_list;
  for (i=0; i<t_visible_cells; i++, cell++)
  {    
    cx=cell->x;
    cy=cell->y;

    g1_map_cell_class *map_cell=m+cx+cy*width();

    vt[0]=v + cx + cy * mw_p1;
    vt[1]=vt[0]+1;
    vt[2]=vt[1]+mw_p1;
    vt[3]=vt[2]-1;


    vt[0]->clear_calculations();
    vt[1]->clear_calculations();
    vt[2]->clear_calculations();
    vt[3]->clear_calculations();        

  }

  pf_map_fast_clear.stop();
}



#endif
