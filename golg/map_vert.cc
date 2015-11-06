/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "map.hh"
#include "map_vert.hh"
#include "math/trig.hh"
#include "light.hh"
#include "tick_count.hh"
#include "g1_render.hh"
#include "map_man.hh"
#include "compress/rle.hh"
#include "loaders/dir_save.hh"
#include "saver_id.hh"
#include "saver.hh"
#include "light.hh"
#include "objs/model_collide.hh"
#include "time/profile.hh"
#include "tile.hh"
#include "map_cell.hh"

i4_profile_class pf_get_map_height("get_map_height");
i4_profile_class pf_calc_height_pitch_roll("calc_height_pitch_roll");
i4_profile_class pf_calc_pitch_roll("calc_pitch_roll");
i4_profile_class pf_recalc_light_sum("recalc_light_sum");
i4_profile_class pf_terrain_height("terrain_height");


void g1_map_vertex_class::init()
{
  shadow_subtract=0;
  light_sum=0x80000000;
  normal=0x8000;
  dynamic_light=0;

  height=5;

  flags=0;
  clip_code=0;
}

void g1_map_vertex_class::wave_transform(i4_transform_class &t, float map_x, float map_y)
{
#if 0
  int tick=g1_tick_counter;

  float lx = map_x + cos((tick+map_x) * 0.1)*0.1;
  float ly = map_y + cos((tick+map_y) * 0.01)*0.1;
  float lh = get_height() + cos((tick+(map_y+map_x)*10.0) * 0.2)*0.1;

  tick++;
  float nx = map_x + cos((tick+map_x) * 0.1)*0.1;
  float ny = map_y + cos((tick+map_y) * 0.01)*0.1;
  float nh = get_height() + cos((tick+(map_y+map_x)*10.0) * 0.2)*0.1;


  t.transform(i4_3d_point_class(lx+(nx-lx)*g1_render.frame_ratio,
                                ly+(ny-ly)*g1_render.frame_ratio,
                                lh+(nh-lh)*g1_render.frame_ratio),
              v);
#else
  t.transform(i4_3d_point_class(map_x, map_y, t_height), v);
#endif
}

void g1_map_class::calc_height_pitch_roll(i4_float x, i4_float y, i4_float z,
                                          i4_float &height, i4_float &pitch, i4_float &roll)
{
  pf_calc_height_pitch_roll.start();
  
  i4_3d_vector normal;
  
  sw32 ix = i4_f_to_i(x);
  sw32 iy = i4_f_to_i(y);

  if (!(ix>=0 && iy>=0 && ix<w && iy<h)) 
    i4_error("off map");

    
  g1_map_cell_class *c = cell(ix,iy);  

  i4_3d_vector ray(0,0,-5);
  float h;
  i4_bool check_with_object=i4_F;
        
  g1_object_chain_class *chain=c->get_solid_list();
  for (;chain; chain=chain->next_solid())
    if (chain->object->flags & g1_object_class::CAN_DRIVE_ON)
    {
      g1_object_class *o=chain->object;        

      if (g1_model_collide_polygonal(o, o->draw_params, i4_3d_vector(x,y,z+0.2), ray, normal))
      {
        height=z+0.2+ray.z;
        pitch = i4_atan2(normal.x,sqrt(normal.z*normal.z  + normal.y*normal.y));
        roll  = i4_atan2(-normal.y,sqrt(normal.z*normal.z + normal.x*normal.x));

        check_with_object=i4_T;
        
        if (ray.z>-0.4)    // if we are near the object don't check terrain
        {
          pf_calc_height_pitch_roll.stop();
          return;
        }

        r1_texture_handle han = g1_tile_man.get_texture(c->type);   
        if (han==g1_tile_man.get_pink())    // don't check pink surfaces
        {
          pf_calc_height_pitch_roll.stop();
          return;
        }

      }
    }


  x-=ix;
  y-=iy;



  i4_3d_vector u,v;
  if (x>y)
  {
    g1_map_vertex_class *v1=verts+ ix + iy * (w+1), *v2,*v3;
    v2=v1+1;
    v3=v2+w+1;    

    i4_float z1=v1->get_height(), z2=v2->get_height(), z3=v3->get_height();    

    u=i4_3d_vector(-1,0,z1-z2);
    v=i4_3d_vector(0,1,z3-z2);
    h=z2+(z1-z2)*(1-x) + (z3-z2)*y;
  }
  else
  {
    g1_map_vertex_class *v1=verts+ ix + iy * (w+1), *v2,*v3;
    v2=v1+w+1;
    v3=v2+1;    

    i4_float z1=v1->get_height(), z2=v2->get_height(), z3=v3->get_height();    

    u=i4_3d_vector(1,0,z3-z2);
    v=i4_3d_vector(0,-1,z1-z2);
    h=z2+(z1-z2)*(1-y) + (z3-z2)*x;
  }


  if ((check_with_object && h>height && h-0.2<z) || !check_with_object)
  {    
    normal.cross(v,u);
    pitch = i4_atan2(normal.x,sqrt(normal.z*normal.z  + normal.y*normal.y));
    roll  = i4_atan2(-normal.y,sqrt(normal.z*normal.z + normal.x*normal.x));
    height=h;
  }

  pf_calc_height_pitch_roll.stop();
}



i4_float g1_map_class::map_height(i4_float x, i4_float y, i4_float z) const
{
  
  sw32 ix = i4_f_to_i(x);
  sw32 iy = i4_f_to_i(y);

  if (!(ix>=0 && iy>=0 && ix<w && iy<h))
  {
    return 1000;
  }

  g1_map_cell_class *c = cell(ix,iy);  

  i4_3d_vector normal;
  i4_bool check_with_object=i4_F;
  float height, h;
  
  g1_object_chain_class *chain=c->get_solid_list();
  for (;chain; chain=chain->next_solid())
    if (chain->object->flags & g1_object_class::CAN_DRIVE_ON)
    {
      g1_object_class *o=chain->object;        
      i4_3d_vector ray(0,0,-5); 
      if (g1_model_collide_polygonal(o, o->draw_params, i4_3d_vector(x,y,z+0.2), ray, normal))
      {
        height=z+0.2+ray.z;
        if (ray.z>-0.4)    // if we are near the object don't check terrain
        {
          return height;
        }
        
        r1_texture_handle han = g1_tile_man.get_texture(c->type);   
        if (han==g1_tile_man.get_pink())    // don't check pink surfaces
        {
          return height;
        }

        check_with_object=i4_T;
      }

    }
  
  x-=ix;
  y-=iy;

  if (x>y)
  {
    g1_map_vertex_class *v1=verts+ ix + iy * (w+1), *v2,*v3;
    v2=v1+1;
    v3=v2+w+1;
    
    i4_float z1=v1->get_height(), z2=v2->get_height(), z3=v3->get_height();    
    h=z2+(z1-z2)*(1-x) + (z3-z2)*y;    
  }
  else
  {
    g1_map_vertex_class *v1=verts+ ix + iy * (w+1), *v2,*v3;
    v2=v1+w+1;
    v3=v2+1;    
    
    i4_float z1=v1->get_height(), z2=v2->get_height(), z3=v3->get_height();    
    h=z2+(z1-z2)*(1-y) + (z3-z2)*x;    
  }
  
  if ((check_with_object && h>height && h-0.2<z) || !check_with_object)
  {
    return h;
  }
  else
  {
    return height;
  }
}

i4_float g1_map_class::terrain_height(i4_float x, i4_float y) const
{
  pf_terrain_height.start();
  
  sw32 ix=i4_f_to_i(x), iy=i4_f_to_i(y);

  x-=ix;
  y-=iy;

  float ret=0;
  if (x>y)
  {
    g1_map_vertex_class *v1=verts+ ix + iy * (w+1), *v2,*v3;
    v2=v1+1;
    v3=v2+w+1;
  
    i4_float z1=v1->get_height(), z2=v2->get_height(), z3=v3->get_height();
    ret=z2+(z1-z2)*(1-x) + (z3-z2)*y;
  }
  else
  {
    g1_map_vertex_class *v1=verts+ ix + iy * (w+1), *v2,*v3;
    v2=v1+w+1;
    v3=v2+1;    
  
    i4_float z1=v1->get_height(), z2=v2->get_height(), z3=v3->get_height();
    ret=z2+(z1-z2)*(1-y) + (z3-z2)*x;
  }  


  
  pf_terrain_height.stop();
  return ret;
}



void g1_map_class::calc_pitch_and_roll(i4_float x, i4_float y, i4_float z, 
                                       i4_float &pitch, i4_float &roll) 
{
  pf_calc_pitch_roll.start();
  
  i4_3d_vector normal;
  sw32 ix=i4_f_to_i(x), iy=i4_f_to_i(y);

  x-=(i4_float)ix;
  y-=(i4_float)iy;

  g1_map_cell_class *c = cell(ix,iy);  

  if (!(ix>=0 && iy>=0 && ix<w && iy<h))
    i4_error("off map");

  g1_object_chain_class *chain=c->get_solid_list();
  for (;chain; chain=chain->next_solid())
    if (chain->object->flags & g1_object_class::CAN_DRIVE_ON)
    {
      g1_object_class *o=chain->object;        
      i4_3d_vector ray(0,0,-5); 
      if (g1_model_collide_polygonal(o, o->draw_params, i4_3d_vector(x,y,z+0.2), ray, normal))
      {
        pitch = i4_atan2(normal.x,sqrt(normal.z*normal.z  + normal.y*normal.y));
        roll  = i4_atan2(-normal.y,sqrt(normal.z*normal.z + normal.x*normal.x));
        pf_calc_pitch_roll.stop();
        return ;
      }
    }


 
  if (x>y)
  {
    g1_map_vertex_class *v1=verts+ ix + iy * (w+1), *v2,*v3;
    v2=v1+1;
    v3=v2+w+1;    

    i4_float z1=v1->get_height(), z2=v2->get_height(), z3=v3->get_height();    

    i4_3d_vector u=i4_3d_vector(-1,0,z1-z2), v=i4_3d_vector(0,1,z3-z2);
    normal.cross(v,u);
  }
  else
  {
    g1_map_vertex_class *v1=verts+ ix + iy * (w+1), *v2,*v3;
    v2=v1+w+1;
    v3=v2+1;    

    i4_float z1=v1->get_height(), z2=v2->get_height(), z3=v3->get_height();    

    i4_3d_vector u=i4_3d_vector(1,0,z3-z2), v=i4_3d_vector(0,-1,z1-z2);
    normal.cross(v,u);
  }

  pitch = i4_atan2(normal.x,sqrt(normal.z*normal.z  + normal.y*normal.y));
  roll  = i4_atan2(-normal.y,sqrt(normal.z*normal.z + normal.x*normal.x));
  pf_calc_pitch_roll.stop();
}

void g1_map_class::calc_terrain_normal(i4_float x, i4_float y, i4_3d_vector &normal)
{
  sw32 ix=i4_f_to_i(x), iy=i4_f_to_i(y);
  g1_map_cell_class *c=cells + ix + iy * width();

  if (x>y)
    c->get_bottom_right_normal(normal,ix,iy);
  else
    c->get_top_left_normal(normal,ix,iy);
}


void g1_map_class::change_vert_height(sw32 x, sw32 y, w8 new_height)
{ 
  g1_map_vertex_class *v=verts + x + y*(w+1);
  v->height=new_height;
  v->normal=0x8000;
  v->light_sum=0x80000000; 
}


void g1_map_vertex_class::recalc_normal(int x, int y)
{
  i4_3d_vector sum(0,0,0), v,v2;
  g1_map_cell_class *c=g1_get_map()->cell(x,y);
  int mw=g1_get_map()->width(), mh=g1_get_map()->height();

  I4_ASSERT(x<=mw && y<=mh, "recalc normal : vert out of range");

  if (y<mh)
  {
    if (x>0)
    {
      c[-1].get_bottom_right_normal(v, x-1, y);
      sum+=v;
    }

    if (x<mw)
    {
      c[0].get_top_left_normal(v, x,y);
      c[0].get_bottom_right_normal(v2, x,y);
      
      v+=v2;
      v.normalize();
      sum+=v;
    }
  }

  if (y>0)
  {
    if (x<mw)
    {
      c[-mw].get_top_left_normal(v, x,y-1);
      sum+=v;
    }

    if (x>0)
    {
      c[-mw-1].get_top_left_normal(v, x-1,y-1);
      c[-mw-1].get_bottom_right_normal(v2, x-1,y-1);
      v+=v2;
      v.normalize();

      sum+=v;
    }
  }

  sum.normalize();
  normal = g1_normal_to_16(sum);
}

void g1_map_class::get_illumination_light(i4_float wx, i4_float wy, 
                                          i4_float &red, 
                                          i4_float &green, 
                                          i4_float &blue)
{
  sw32 ix=i4_f_to_i(wx), iy=i4_f_to_i(wy);

  i4_float x=wx-ix, y=wy-iy;

  g1_map_vertex_class *v1,*v2,*v3;
  i4_float r[3],g[3],b[3];

  if (x>y)
  {
    v1=verts+ix+iy*(w+1);
    v2=v1+1;
    v3=v2+w+1;
    
    v1->get_rgb(r[0],g[0],b[0], ix,   iy);
    v2->get_rgb(r[1],g[1],b[1], ix+1, iy);
    v3->get_rgb(r[2],g[2],b[2], ix+1, iy+1);    

    red=r[1]+(r[0]-r[1])*(1-x) + (r[2]-r[2])*y;
    green=g[1]+(g[0]-g[1])*(1-x) + (g[2]-g[2])*y;
    blue=b[1]+(b[0]-b[1])*(1-x) + (b[2]-b[2])*y;

  }
  else
  {
    v1=verts+ix+iy*(w+1);
    v2=v1+w+1;
    v3=v2+1;
    
    v1->get_rgb(r[0],g[0],b[0], ix,  iy);
    v2->get_rgb(r[1],g[1],b[1], ix,  iy+1);
    v3->get_rgb(r[2],g[2],b[2], ix+1,iy+1);    

    red=r[1]+(r[0]-r[1])*(1-y) + (r[2]-r[1])*x;
    green=g[1]+(g[0]-g[1])*(1-y) + (g[2]-g[1])*x;
    blue=b[1]+(b[0]-b[1])*(1-y) + (b[2]-b[1])*x;
  }

}


float g1_map_vertex_class::get_non_dynamic_ligth_intensity(int cvx, int cvy)
{
  i4_3d_vector n;
  get_normal(n, cvx, cvy);

  // directional contribution (white light)
  float i=-n.dot(g1_lights.direction) * static_intensity * (1.0/255.0);
  if (i<0) i=0;
  i+=g1_lights.ambient_intensity;
  if (i>1) i=1;

  return i;
}

void g1_map_vertex_class::recalc_light_sum(int cvx, int cvy)
{
  pf_recalc_light_sum.start();

  float r,g,b;
  i4_3d_vector n;

  get_normal(n, cvx, cvy);

  // directional contribution (white light)
  float i=-n.dot(g1_lights.direction) * static_intensity * (1.0/255.0);
  i = (i<0) ? 0 : i;

  if (i>1)
    i4_warning("intensity>1");

  // global contribution (white light)
  i+=g1_lights.ambient_intensity;
  i = (i>1)? 1 : i;
  i*=g1_lights.shadow_intensity[shadow_subtract];
      
  // dynamic light contribution
  r=g1_table_0_255_to_0_1[((dynamic_light>>16)&0xff)] + i;
  g=g1_table_0_255_to_0_1[((dynamic_light>>8)&0xff)] + i;
  b=g1_table_0_255_to_0_1[((dynamic_light)&0xff)] + i;

  if (r>1) r=1;
  if (g>1) g=1;
  if (b>1) b=1;

  light_sum=(i4_f_to_i(r*255.0) << 16) |
    (i4_f_to_i(g*255.0) << 8) |
    (i4_f_to_i(b*255.0));

  pf_recalc_light_sum.stop();
}


void g1_save_map_verts(g1_map_vertex_class *list, 
                       int lsize, 
                       i4_saver_class *fp,
                       int mark_sections)

{
  int i;
  i4_rle_class<w32> fp32(fp);
  i4_rle_class<w16> fp16(fp);
  i4_rle_class<w8> fp8(fp);


  if (mark_sections)
    fp->mark_section("golgotha vert height");
  for (i=0; i<lsize; i++)
    fp8.write(list[i].height);
  fp8.flush();

  if (mark_sections)
    fp->mark_section("golgotha vert light sum v2");
  for (i=0; i<lsize; i++)
    fp32.write(list[i].light_sum);
  fp32.flush();

  if (mark_sections)
    fp->mark_section("golgotha vert normal");
  for (i=0; i<lsize; i++)
    fp16.write(list[i].normal);
  fp16.flush();

  if (mark_sections)
    fp->mark_section("golgotha vert static_intensity");
  for (i=0; i<lsize; i++)
    fp8.write(list[i].static_intensity);
  fp8.flush();

  if (mark_sections)
    fp->mark_section("golgotha vert flags v2");
  for (i=0; i<lsize; i++)
    fp8.write(list[i].flags & g1_map_vertex_class::SAVED_FLAGS);
  fp8.flush();
  


}


i4_bool g1_load_map_verts(g1_map_vertex_class *list, int lsize, 
                          i4_loader_class *fp,
                          int goto_sections)
{
  w16 *index=0;
  int i, load_old=0;

  i4_rle_class<w32> fp32(fp);
  i4_rle_class<w16> fp16(fp);
  i4_rle_class<w8> fp8(fp);

  // initialize stuff we aren't loading
  for (i=0; i<lsize; i++)
  {
    list[i].clip_code=0;
    list[i].dynamic_light=0;       // this should be added in by lights in the level
    list[i].shadow_subtract=0;
  }

  if (!goto_sections || fp->goto_section("golgotha vert height"))
  {
    for (i=0; i<lsize; i++)
      list[i].height=fp8.read();
  }
  else load_old=1;

  if (!goto_sections || fp->goto_section("golgotha vert light sum v2"))
  {
    for (i=0; i<lsize; i++)
      list[i].light_sum=fp32.read();
  }
  else if (fp->goto_section("golgotha vert light sum"))
  {
    for (i=0; i<lsize; i++)
      list[i].light_sum=0x80000000;
  }


  if (!goto_sections || fp->goto_section("golgotha vert normal"))
  {
    for (i=0; i<lsize; i++)
      list[i].normal=fp16.read();
  }


  if (!goto_sections || fp->goto_section("golgotha vert static_intensity"))
  {
    for (i=0; i<lsize; i++)
      list[i].static_intensity=fp8.read();
  }


  if (!goto_sections || fp->goto_section("golgotha vert flags v2"))
  {
    for (i=0; i<lsize; i++)
      list[i].flags=fp8.read() & g1_map_vertex_class::SAVED_FLAGS;
  }
  else
    for (i=0; i<lsize; i++)
      list[i].flags=g1_map_vertex_class::FOGGED;

    
  g1_get_map()->mark_for_recalc(G1_RECALC_WATER_VERTS);



  if (load_old)
  {
    int k=lsize;
    if (fp->goto_section(OLD_G1_SECTION_MAP_VERT_V4))
    {
      if (fp->read_8())
      {
        w16 index;
        do
        {
          index=fp->read_16();
          if (index!=0xffff)
            list[index].load_v4(fp);
        } while (index!=0xffff);
      }
      else for (i=0; i<k; i++)
        list[i].load_v4(fp);
    }    
    else if (fp->goto_section(OLD_G1_SECTION_MAP_VERT_V3))
    {
      for (i=0; i<k; i++)
        list[i].load_v4(fp);
    }
    else if (fp->goto_section(OLD_G1_SECTION_MAP_VERT_V2))
    {
      for (i=0; i<k; i++)
        list[i].load_v2(fp);
    }
    else if (fp->goto_section(OLD_G1_SECTION_MAP_VERT_V1))
    {
      for (i=0; i<k; i++)
        list[i].load_v1(fp);
    }
    else return i4_F;
  }
    

  return i4_T;

}





////////////////// OLD LOAD METHODS ////////////////////////////////////

void g1_map_vertex_class::load_v1(i4_file_class *fp)
{
  dynamic_light=0;
  light_sum=0x80000000;
  normal=0x8000;
  shadow_subtract=0;

  static_intensity=(w8)(g1_lights.directional_intensity * (1.0/255.0));

  fp->read_16();           // light value gone
  height=fp->read_8();
  set_is_selected(0);
  clip_code=0;
  flags=0;
}

void g1_map_vertex_class::load_v2(i4_file_class *fp)
{
  dynamic_light=0;
  light_sum=0x80000000;
  shadow_subtract=0;
  normal=0x8000;
  static_intensity=(w8)(g1_lights.directional_intensity * (1.0/255.0));

  fp->read_16();           // light value gone
  height=fp->read_8();

  fp->read_16();
  normal &= ~0x8000;
  flags = 0;
  clip_code=0;
}

void g1_map_vertex_class::load_v4(i4_file_class *fp)
{
  dynamic_light=0;
  light_sum=0x80000000;
  shadow_subtract=0;
  static_intensity=(w8)(g1_lights.directional_intensity * (1.0/255.0));


  fp->read_16();           // light value gone
  height=fp->read_8();
  fp->read_16();
  normal=0x8000;

  flags=0;
  set_is_selected(fp->read_8());  
  clip_code = 0;
}
