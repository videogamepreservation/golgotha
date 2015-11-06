/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "r1_api.hh"
#include "tmanage.hh"
#include "r1_clip.hh"
#include "image/context.hh"
#include <string.h>

r1_render_api_class     *r1_render_api_class_instance=0;

r1_shading_type         r1_render_api_class::shade_mode=R1_SHADE_DISABLED;
r1_alpha_type           r1_render_api_class::alpha_mode=R1_ALPHA_DISABLED;
r1_write_mask_type      r1_render_api_class::write_mask=0;
w32                     r1_render_api_class::const_color=0;
r1_miplevel_t          *r1_render_api_class::last_node=0;
r1_render_api_class    *r1_render_api_class::first;
r1_filter_type          r1_render_api_class::filter_mode=R1_NO_FILTERING;
i4_draw_context_class  *r1_render_api_class::context=0;
i4_float                r1_render_api_class::r_tint_mul=0;
i4_float                r1_render_api_class::g_tint_mul=0;
i4_float                r1_render_api_class::b_tint_mul=0;
i4_bool                 r1_render_api_class::color_tint_on=i4_F;
r1_color_tint           r1_render_api_class::color_tint_list[r1_render_api_class::MAX_COLOR_TINTS];

//start with 1, since 0 disables tinting
sw8                     r1_render_api_class::num_color_tints = 1;                                                                    

r1_color_tint_handle r1_render_api_class::register_color_tint(i4_float r, i4_float g, i4_float b)
{
  if (num_color_tints==MAX_COLOR_TINTS) return 0;

  color_tint_list[num_color_tints].r = r;
  color_tint_list[num_color_tints].g = g;
  color_tint_list[num_color_tints].b = b;

  num_color_tints++;

  return num_color_tints-1;
}


void r1_render_api_class::put_image(i4_image_class *im,                                          
                                    int _x, int _y,             // position on screen
                                    int x1, int y1,           // area of image to copy 
                                    int x2, int y2)
{
  i4_rect_list_class *clip=&context->clip;  
  i4_rect_list_class::area_iter cl;

  for (cl = clip->list.begin(); cl != clip->list.end(); ++cl)
  {
    int lx1,ly1,lx2,ly2,x=_x,y=_y;

    if (x1<0) { lx1=0; _x+=-x1; } else lx1=x1;
    if (y1<0) { ly1=0; _y+=-y1; } else ly1=y1;
    if (x2>=im->width())  lx2=im->width()-1;   else lx2=x2;
    if (y2>=im->height()) ly2=im->height()-1;  else ly2=y2;
  
    if (!(lx1>lx2 || ly1>ly2))
    {
      if (x<cl->x1)
      { lx1+=(cl->x1-x); x=cl->x1; }

      if (y<cl->y1)
      { ly1+=(cl->y1-y); y=cl->y1; }


      if (x+lx2-lx1+1>cl->x2)
        lx2=cl->x2-x+lx1; 

      if (y+ly2-ly1+1>cl->y2)
        ly2=cl->y2-y+ly1; 


      if (!(lx1>lx2 || ly1>ly2))
        copy_part(im, x,y, lx1,ly1,lx2,ly2);
    }
  }
}

void r1_render_api_class::set_color_tint(r1_color_tint_handle c)
{
  if (c==0)
  {
    r_tint_mul = 1.f;
    g_tint_mul = 1.f;
    b_tint_mul = 1.f;

    color_tint_on = i4_F;
	return;
  }

  color_tint_on = i4_T;

  r_tint_mul = color_tint_list[c].r;
  g_tint_mul = color_tint_list[c].g;
  b_tint_mul = color_tint_list[c].b;    
}


r1_render_api_class *r1_create_api(i4_display_class *for_display, char *name)
{
  for (r1_render_api_class *r=r1_render_api_class::first; r; r=r->next)
  {
   
    if ((!name || strcmp(r->name(), name)==0) && r->init(for_display))
    {
      r1_render_api_class_instance = r;
      return r;
    }
  }

  return 0;
}

void r1_destroy_api(r1_render_api_class *r)
{
  r->uninit();
  if (r1_render_api_class_instance==r) r1_render_api_class_instance = 0;
}

i4_bool r1_render_api_class::init(i4_display_class *display)
{
  // this should clear out the states and set to a default
  set_constant_color(0xffffffff);
  set_constant_color(0);

  set_shading_mode(R1_COLORED_SHADING);
  set_shading_mode(R1_SHADE_DISABLED);

  set_alpha_mode(R1_ALPHA_LINEAR);
  set_alpha_mode(R1_ALPHA_DISABLED);

  set_write_mode(0xffff);
  set_write_mode(0);

  set_filter_mode(R1_BILINEAR_FILTERING);
  set_filter_mode(R1_NO_FILTERING);

  set_z_range(0.001, 1.0);
  return i4_T;
}


void r1_render_api_class::uninit()
{
  if (tmanager)
  {
    delete tmanager;
    tmanager=0;
  }
}


void r1_render_api_class::render_sprite(r1_vert *verts)
{
  render_poly(4,verts);
}

void r1_render_api_class::clear_area(int x1, int y1, int x2, int y2, w32 color, float z)
{
  w32 old_const_color=get_constant_color();
  set_constant_color(color);

  r1_shading_type old_shade_mode=get_shade_mode();
  set_shading_mode(R1_CONSTANT_SHADING);

  disable_texture();

  r1_vert v[4];

  float w = r1_ooz(z);

  v[0].px=x1;     v[0].py=y1;      v[0].w=w;   v[0].v.z=z;
  v[1].px=x1;     v[1].py=y2+1;    v[1].w=w;   v[1].v.z=z;
  v[2].px=x2+1;   v[2].py=y2+1;    v[2].w=w;   v[2].v.z=z;
  v[3].px=x2+1;   v[3].py=y1;      v[3].w=w;   v[3].v.z=z;
  
  render_poly(4, v);

  set_shading_mode(old_shade_mode);
  set_constant_color(old_const_color);
}


r1_vert tmp_render_poly_verts[128];

// default method
void r1_render_api_class::render_poly(int t_verts, r1_vert *verts, int *vertex_index)
{
  if (t_verts>128) return;

  for (int i=0; i<t_verts; i++)
    tmp_render_poly_verts[i] = verts[vertex_index[i]];
  
  render_poly(t_verts, tmp_render_poly_verts);  
}

void r1_render_api_class::render_poly(int t_verts, r1_vert *verts, w16 *vertex_index)
{  
  if (t_verts>128) return;

  for (int i=0; i<t_verts; i++)
    tmp_render_poly_verts[i] = verts[vertex_index[i]];

  render_poly(t_verts, tmp_render_poly_verts);
}

i4_float r1_near_clip_z = 0.001;
i4_float r1_far_clip_z  = 100;

#define R1_NEXT(i,j) ( (((i)+1)==(j))?(0):((i)+1) )
#define R1_PREV(i,j) ( ((i)==(0))?((j)-1):((i)-1) )

r1_vert *r1_render_api_class::clip_poly(sw32 *initial_num_vertices,r1_vert *t_vertices,
                                        w16 *indices,r1_vert *clip_buf_1,
                                        r1_vert *clip_buf_2,
                                        w8 flags)
{

  w32 ORCODE  = 0;
  w32 ANDCODE = 0xffffffff;
  w32 i,j,c0,c1;
  w32 bitmask;
  i4_float ooz,dx,dy,dz,ds,dt,dr,dg,db,da,t;  

  r1_vert *vertices          = clip_buf_1;
  sw32     num_vertices      = *initial_num_vertices;
                             
  r1_vert *clip_vertices     = clip_buf_2;
  sw32     num_clip_vertices = 0;

  r1_vert clip_point;        

  if (flags & R1_CLIP_NO_CALC_OUTCODE)
  {
    for (i=0; i<num_vertices; i++)
    {    
      vertices[i] = t_vertices[indices[i]];      
      ORCODE  |= vertices[i].outcode;
      ANDCODE &= vertices[i].outcode;
    }
  }
  else
  {
    for (i=0; i<num_vertices; i++)
    {    
      w8 outcode;
      vertices[i] = t_vertices[indices[i]];
      if (flags & R1_CLIP_ORTHO)
        outcode = r1_calc_ortho_outcode(&vertices[i]);
      else
        outcode = r1_calc_outcode(&vertices[i]);
      ORCODE  |= outcode;
      ANDCODE &= outcode;    
    }
  }
  
  if (ANDCODE)
  {
    *initial_num_vertices = 0;
    return 0;
  }
  
  if (!ORCODE)
  {
    return vertices;
  }
  
  
  for (bitmask=32;bitmask;bitmask=bitmask>>1)
  {
    if ((bitmask&ORCODE)==0) continue;
    
    for (i=0;i<num_vertices;i++)
    {			
      
      j  = R1_NEXT(i,num_vertices);
      
      c0 = bitmask & vertices[i].outcode;
      c1 = bitmask & vertices[j].outcode;
      
      //if c0 is not outside of this plane,
      //add it
      if (c0==0) {
        clip_vertices[num_clip_vertices] = vertices[i];
        num_clip_vertices++;
      }
      
      //if they are on the same
      //side, move to the next vert
      if (c0==c1) continue;
      
      //otherwise, generate a clipped
      //point
      
      dx = vertices[j].v.x - vertices[i].v.x;
      dy = vertices[j].v.y - vertices[i].v.y;
      dz = vertices[j].v.z - vertices[i].v.z;
      ds = vertices[j].s - vertices[i].s;
      dt = vertices[j].t - vertices[i].t;
      dr = vertices[j].r - vertices[i].r;
      dg = vertices[j].g - vertices[i].g;
      db = vertices[j].b - vertices[i].b;
      da = vertices[j].a - vertices[i].a;
            
      switch (bitmask | (flags & R1_CLIP_ORTHO)) {
        case 1: 
          t = (-vertices[i].v.x + vertices[i].v.z) / ( dx - dz);                  
          clip_point.v.y = vertices[i].v.y + (t * dy);
          clip_point.v.z = vertices[i].v.z + (t * dz);
          
          clip_point.v.x = clip_point.v.z;
          break;
          
        case 2: 
          t = ( vertices[i].v.x + vertices[i].v.z) / (-dx - dz);                  
          clip_point.v.y = vertices[i].v.y + (t * dy);
          clip_point.v.z = vertices[i].v.z + (t * dz);
          
          clip_point.v.x = -clip_point.v.z;
          break;
        
        case 4: 
          t = (-vertices[i].v.y + vertices[i].v.z) / ( dy - dz);
          clip_point.v.x = vertices[i].v.x + (t * dx);                 
          clip_point.v.z = vertices[i].v.z + (t * dz);
          
          clip_point.v.y = clip_point.v.z;
          break;
        
        case 8:
          t = ( vertices[i].v.y + vertices[i].v.z) / (-dy - dz);
          clip_point.v.x = vertices[i].v.x + (t * dx);                 
          clip_point.v.z = vertices[i].v.z + (t * dz);
          
          clip_point.v.y = -clip_point.v.z; 
          break;
          
        case 16: 
          t = (r1_near_clip_z - vertices[i].v.z) / (dz);		                
          clip_point.v.x = vertices[i].v.x + (t * dx);
          clip_point.v.y = vertices[i].v.y + (t * dy);
          
          clip_point.v.z = r1_near_clip_z;
          break;

        case 32: 
          t = -(vertices[i].v.z - r1_far_clip_z) / (dz);		                
          clip_point.v.x = vertices[i].v.x + (t * dx);
          clip_point.v.y = vertices[i].v.y + (t * dy);
          
          clip_point.v.z = r1_far_clip_z;
          break;
          
        case 1|128:
          t = (1.0 - vertices[i].v.x) / dx;
          clip_point.v.y = vertices[i].v.y + (t * dy);
          clip_point.v.z = vertices[i].v.z + (t * dz);
          clip_point.v.x = 1.0;
          break;
          
        case 2|128:
          t = (vertices[i].v.x + 1.0) / -dx;
          clip_point.v.y = vertices[i].v.y + (t * dy);
          clip_point.v.z = vertices[i].v.z + (t * dz);
          clip_point.v.x = -1.0;
          break;
          
        case 4|128:
          t = (1.0 - vertices[i].v.y) / dy;
          clip_point.v.x = vertices[i].v.x + (t * dx);                 
          clip_point.v.z = vertices[i].v.z + (t * dz);
          clip_point.v.y = 1.0;
          break;
          
        case 8|128:
          t = (vertices[i].v.y + 1.0) / -dy;
          clip_point.v.x = vertices[i].v.x + (t * dx);                 
          clip_point.v.z = vertices[i].v.z + (t * dz);
          clip_point.v.y = -1.0; 
          break;
      }
      
      clip_point.s = vertices[i].s + (t * ds);
      clip_point.t = vertices[i].t + (t * dt);
      clip_point.r = vertices[i].r + (t * dr);
      clip_point.g = vertices[i].g + (t * dg);
      clip_point.b = vertices[i].b + (t * db);
      clip_point.a = vertices[i].a + (t * da);
            
      if (flags & R1_CLIP_ORTHO)
        ORCODE |= r1_calc_ortho_outcode(&clip_point);
      else      
        ORCODE |= r1_calc_outcode(&clip_point);
      
      clip_vertices[num_clip_vertices] = clip_point;
      num_clip_vertices++;            
    }
    
    num_vertices      = num_clip_vertices;
    num_clip_vertices = 0;

    if (vertices==clip_buf_1)
    {
      vertices      = clip_buf_2;
      clip_vertices = clip_buf_1;
    }
    else
    {
      vertices      = clip_buf_1;
      clip_vertices = clip_buf_2;
    }
  }
  
  *initial_num_vertices = num_vertices;
  return vertices;
}

r1_vert *r1_render_api_class::clip_poly(sw32 *initial_num_vertices,r1_vert *t_vertices,
                                        w32 *indices,r1_vert *clip_buf_1,
                                        r1_vert *clip_buf_2,
                                        w8 flags)
{

  w32 ORCODE  = 0;
  w32 ANDCODE = 0xffffffff;
  w32 i,j,c0,c1;
  w32 bitmask;
  i4_float ooz,dx,dy,dz,ds,dt,dr,dg,db,da,t;  

  r1_vert *vertices          = clip_buf_1;
  sw32     num_vertices      = *initial_num_vertices;
                             
  r1_vert *clip_vertices     = clip_buf_2;
  sw32     num_clip_vertices = 0;

  r1_vert clip_point;        

  if (flags & R1_CLIP_NO_CALC_OUTCODE)
  {
    for (i=0; i<num_vertices; i++)
    {    
      vertices[i] = t_vertices[indices[i]];      
      ORCODE  |= vertices[i].outcode;
      ANDCODE &= vertices[i].outcode;
    }
  }
  else
  {
    for (i=0; i<num_vertices; i++)
    {    
      vertices[i] = t_vertices[indices[i]];
      w8 outcode = r1_calc_outcode(&vertices[i]);    
      ORCODE  |= outcode;
      ANDCODE &= outcode;    
    }
  }
  
  if (ANDCODE)
  {
    *initial_num_vertices = 0;
    return 0;
  }
  
  if (!ORCODE)
  {
    return vertices;
  }
  
  for (bitmask=32;bitmask;bitmask=bitmask>>1)
  {
    if ((bitmask&ORCODE)==0) continue;
    
    for (i=0;i<num_vertices;i++)
    {			
      
      j  = R1_NEXT(i,num_vertices);
      
      c0 = bitmask & vertices[i].outcode;
      c1 = bitmask & vertices[j].outcode;
      
      //if c0 is not outside of this plane,
      //add it
      if (c0==0) {
        clip_vertices[num_clip_vertices] = vertices[i];
        num_clip_vertices++;
      }
      
      //if they are on the same
      //side, move to the next vert
      if (c0==c1) continue;
      
      //otherwise, generate a clipped
      //point
      
      dx = vertices[j].v.x - vertices[i].v.x;
      dy = vertices[j].v.y - vertices[i].v.y;
      dz = vertices[j].v.z - vertices[i].v.z;
      ds = vertices[j].s - vertices[i].s;
      dt = vertices[j].t - vertices[i].t;
      dr = vertices[j].r - vertices[i].r;
      dg = vertices[j].g - vertices[i].g;
      db = vertices[j].b - vertices[i].b;
      da = vertices[j].a - vertices[i].a;
            
      switch (bitmask | (flags&R1_CLIP_ORTHO)) {
        case 1:  
          t = (-vertices[i].v.x + vertices[i].v.z) / ( dx - dz);                  
          clip_point.v.y = vertices[i].v.y + (t * dy);
          clip_point.v.z = vertices[i].v.z + (t * dz);
          
          clip_point.v.x = clip_point.v.z;
          break;
          
        case 2:
          t = ( vertices[i].v.x + vertices[i].v.z) / (-dx - dz);                  
          clip_point.v.y = vertices[i].v.y + (t * dy);
          clip_point.v.z = vertices[i].v.z + (t * dz);
          
          clip_point.v.x = -clip_point.v.z;
          break;
        
        case 4:  
          t = (-vertices[i].v.y + vertices[i].v.z) / ( dy - dz);
          clip_point.v.x = vertices[i].v.x + (t * dx);                 
          clip_point.v.z = vertices[i].v.z + (t * dz);
          
          clip_point.v.y = clip_point.v.z;
          break;
        
        case 8: 
          t = ( vertices[i].v.y + vertices[i].v.z) / (-dy - dz);
          clip_point.v.x = vertices[i].v.x + (t * dx);                 
          clip_point.v.z = vertices[i].v.z + (t * dz);
          
          clip_point.v.y = -clip_point.v.z; 
          break;
          
        case 16: 
          t = (r1_near_clip_z - vertices[i].v.z) / (dz);		                
          clip_point.v.x = vertices[i].v.x + (t * dx);
          clip_point.v.y = vertices[i].v.y + (t * dy);
          
          clip_point.v.z = r1_near_clip_z;
          break;            
        
        case 32:
          t = -(vertices[i].v.z - r1_far_clip_z) / (dz);		                
          clip_point.v.x = vertices[i].v.x + (t * dx);
          clip_point.v.y = vertices[i].v.y + (t * dy);
                 
          clip_point.v.z = r1_far_clip_z;
          break;

        case 1|128:
          t = (1.0 - vertices[i].v.x) / dx;
          clip_point.v.y = vertices[i].v.y + (t * dy);
          clip_point.v.z = vertices[i].v.z + (t * dz);
          clip_point.v.x = 1.0;
          break;
          
        case 2|128:
          t = (vertices[i].v.x + 1.0) / -dx;
          clip_point.v.y = vertices[i].v.y + (t * dy);
          clip_point.v.z = vertices[i].v.z + (t * dz);
          clip_point.v.x = -1.0;
          break;
          
        case 4|128:
          t = (1.0 - vertices[i].v.y) / dy;
          clip_point.v.x = vertices[i].v.x + (t * dx);                 
          clip_point.v.z = vertices[i].v.z + (t * dz);
          clip_point.v.y = 1.0;
          break;
          
        case 8|128:
          t = (vertices[i].v.y + 1.0) / -dy;
          clip_point.v.x = vertices[i].v.x + (t * dx);                 
          clip_point.v.z = vertices[i].v.z + (t * dz);
          clip_point.v.y = -1.0; 
          break;
      }
      
      clip_point.s = vertices[i].s + (t * ds);
      clip_point.t = vertices[i].t + (t * dt);
      clip_point.r = vertices[i].r + (t * dr);
      clip_point.g = vertices[i].g + (t * dg);
      clip_point.b = vertices[i].b + (t * db);
      clip_point.a = vertices[i].a + (t * da);
            
      if (flags & R1_CLIP_ORTHO)
        ORCODE |= r1_calc_ortho_outcode(&clip_point);
      else
        ORCODE |= r1_calc_outcode(&clip_point);
      
      clip_vertices[num_clip_vertices] = clip_point;
      num_clip_vertices++;            
    }
    
    num_vertices      = num_clip_vertices;
    num_clip_vertices = 0;

    if (vertices==clip_buf_1)
    {
      vertices      = clip_buf_2;
      clip_vertices = clip_buf_1;
    }
    else
    {
      vertices      = clip_buf_1;
      clip_vertices = clip_buf_2;
    }
  }
  
  *initial_num_vertices = num_vertices;
  return vertices;
}


