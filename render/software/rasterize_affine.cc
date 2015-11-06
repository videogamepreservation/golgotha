/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "software/r1_software.hh"
#include "software/r1_software_globals.hh"
#include "software/span_buffer.hh"
#include "software/inline_fpu.hh"
#include "software/mappers.hh"

//all calls to qftoi() here assume it will truncate, so be sure that start_trunc() gets
//called before these functions execute

void tri_draw_affine_lit(tri_edge &top_to_middle,tri_edge &top_to_bottom, tri_edge &mid_to_bottom, sw32 start_y, i4_bool edge_comp)
{
  sw32 cur_y = start_y;  
  
  w16 *screen_line;
  
  screen_line = r1_software_render_buffer_ptr + cur_y*r1_software_render_buffer_wpl;
  
  tri_edge *left;
  tri_edge *right;
  tri_edge *last_left = 0;
  
  affine_span cas; //cur_affine_span
  
  float cur_s,cur_t,cur_l;
  float systep,tystep,lystep;

  //rasterize top to middle / top to bottom
  if (top_to_middle.dy)
  {
    if (edge_comp)
    {      
      left  = &top_to_bottom;
      right = &top_to_middle;
    }
    else
    {      
      left  = &top_to_middle;
      right = &top_to_bottom;
    }
    
    last_left = left;

    sw32 lx,rx;

    lx = (left->px  + 0xFFFF) >> 16;
    rx = (right->px + 0xFFFF) >> 16;
    
    cur_s = cur_grads.sat00 + ((float)cur_y*cur_grads.dsdy) + ((float)lx * cur_grads.dsdx);
    cur_t = cur_grads.tat00 + ((float)cur_y*cur_grads.dtdy) + ((float)lx * cur_grads.dtdx);
    cur_l = cur_grads.lat00 + ((float)cur_y*cur_grads.dldy) + ((float)lx * cur_grads.dldx);
    
    systep = cur_grads.dsdy + cur_grads.dsdx * left->dxdy;
    tystep = cur_grads.dtdy + cur_grads.dtdx * left->dxdy;
    lystep = cur_grads.dldy + cur_grads.dldx * left->dxdy;

    while (top_to_middle.dy)
    {      
      lx = (left->px  + 0xFFFF) >> 16;
      rx = (right->px + 0xFFFF) >> 16;

      sw32 width = rx - lx;
      if (width>0)
      {                
        cas.s = qftoi(cur_s);
        cas.t = qftoi(cur_t);
        cas.l = qftoi(cur_l);

        cur_scanline_texture_func(screen_line,(lx<<1),&cas,width);        
      }
     
      //advance y down
      cur_y++;

      cur_s += systep;
      cur_t += tystep;
      cur_l += lystep;

      top_to_middle.px += top_to_middle.dxdy_fixed;
      top_to_bottom.px += top_to_bottom.dxdy_fixed;
      
      top_to_middle.dy--;
      top_to_bottom.dy--;
      
      screen_line += r1_software_render_buffer_wpl;
    }    
  }
  
  if (mid_to_bottom.dy)
  {
    if (edge_comp)
    {
      left  = &top_to_bottom;
      right = &mid_to_bottom;
    }
    else
    {
      left  = &mid_to_bottom;
      right = &top_to_bottom;
    }

    sw32 lx,rx;

    lx = (left->px  + 0xFFFF) >> 16;
    rx = (right->px + 0xFFFF) >> 16;

    if (left != last_left)
    {
      cur_s = cur_grads.sat00 + ((float)cur_y*cur_grads.dsdy) + ((float)lx * cur_grads.dsdx);
      cur_t = cur_grads.tat00 + ((float)cur_y*cur_grads.dtdy) + ((float)lx * cur_grads.dtdx);
      cur_l = cur_grads.lat00 + ((float)cur_y*cur_grads.dldy) + ((float)lx * cur_grads.dldx);
    
      systep = cur_grads.dsdy + cur_grads.dsdx * left->dxdy;
      tystep = cur_grads.dtdy + cur_grads.dtdx * left->dxdy;
      lystep = cur_grads.dldy + cur_grads.dldx * left->dxdy;
    }    

    while (mid_to_bottom.dy)
    {      
      lx = (left->px  + 0xFFFF) >> 16;
      rx = (right->px + 0xFFFF) >> 16;

      sw32 width = rx - lx;
      if (width>0)
      {                
        cas.s = qftoi(cur_s);
        cas.t = qftoi(cur_t);
        cas.l = qftoi(cur_l);        

        cur_scanline_texture_func(screen_line,(lx<<1),&cas,width);        
      }
     
      //advance y down
      cur_y++;

      cur_s += systep;
      cur_t += tystep;
      cur_l += lystep;

      top_to_bottom.px += top_to_bottom.dxdy_fixed;
      mid_to_bottom.px += mid_to_bottom.dxdy_fixed;
      
      top_to_bottom.dy--;
      mid_to_bottom.dy--;
            
      screen_line += r1_software_render_buffer_wpl;
    }
  }
}

void tri_draw_affine_unlit(tri_edge &top_to_middle,tri_edge &top_to_bottom, tri_edge &mid_to_bottom, sw32 start_y, i4_bool edge_comp)
{
  sw32 cur_y = start_y;
  
  w16 *screen_line;
  
  screen_line = r1_software_render_buffer_ptr + cur_y*r1_software_render_buffer_wpl;

  tri_edge *left;
  tri_edge *right;
  tri_edge *last_left = 0;
  
  affine_span cas; //cur_affine_span
  
  float cur_s,cur_t;
  float systep,tystep;

  //rasterize top to middle / top to bottom
  if (top_to_middle.dy)
  {
    if (edge_comp)
    {      
      left  = &top_to_bottom;
      right = &top_to_middle;
    }
    else
    {      
      left  = &top_to_middle;
      right = &top_to_bottom;
    }
    
    last_left = left;

    sw32 lx,rx;

    lx = (left->px  + 0xFFFF) >> 16;
    rx = (right->px + 0xFFFF) >> 16;
    
    cur_s = cur_grads.sat00 + ((float)cur_y*cur_grads.dsdy) + ((float)lx * cur_grads.dsdx);
    cur_t = cur_grads.tat00 + ((float)cur_y*cur_grads.dtdy) + ((float)lx * cur_grads.dtdx);    
    
    systep = cur_grads.dsdy + cur_grads.dsdx * left->dxdy;
    tystep = cur_grads.dtdy + cur_grads.dtdx * left->dxdy;

    while (top_to_middle.dy)
    {      
      lx = (left->px  + 0xFFFF) >> 16;
      rx = (right->px + 0xFFFF) >> 16;

      sw32 width = rx - lx;
      if (width>0)
      {                
        cas.s = qftoi(cur_s);
        cas.t = qftoi(cur_t);        

        cur_scanline_texture_func(screen_line,(lx<<1),&cas,width);        
      }
     
      //advance y down
      cur_y++;

      cur_s += systep;
      cur_t += tystep;      

      top_to_middle.px += top_to_middle.dxdy_fixed;
      top_to_bottom.px += top_to_bottom.dxdy_fixed;
      
      top_to_middle.dy--;
      top_to_bottom.dy--;
                  
      screen_line += r1_software_render_buffer_wpl;
    }    
  }
  
  if (mid_to_bottom.dy)
  {
    if (edge_comp)
    {
      left  = &top_to_bottom;
      right = &mid_to_bottom;
    }
    else
    {
      left  = &mid_to_bottom;
      right = &top_to_bottom;
    }

    sw32 lx,rx;

    lx = (left->px  + 0xFFFF) >> 16;
    rx = (right->px + 0xFFFF) >> 16;

    if (left != last_left)
    {
      cur_s = cur_grads.sat00 + ((float)cur_y*cur_grads.dsdy) + ((float)lx * cur_grads.dsdx);
      cur_t = cur_grads.tat00 + ((float)cur_y*cur_grads.dtdy) + ((float)lx * cur_grads.dtdx);      
    
      systep = cur_grads.dsdy + cur_grads.dsdx * left->dxdy;
      tystep = cur_grads.dtdy + cur_grads.dtdx * left->dxdy;
    }    

    while (mid_to_bottom.dy)
    {      
      lx = (left->px  + 0xFFFF) >> 16;
      rx = (right->px + 0xFFFF) >> 16;
   
      sw32 width = rx - lx;
      if (width>0)
      {                
        cas.s = qftoi(cur_s);
        cas.t = qftoi(cur_t);        

        cur_scanline_texture_func(screen_line,(lx<<1),&cas,width);        
      }
     
      //advance y down
      cur_y++;

      cur_s += systep;
      cur_t += tystep;

      top_to_bottom.px += top_to_bottom.dxdy_fixed;
      mid_to_bottom.px += mid_to_bottom.dxdy_fixed;
      
      top_to_bottom.dy--;
      mid_to_bottom.dy--;

      screen_line += r1_software_render_buffer_wpl;
    }
  }
}

void span_draw_affine_unlit(span_tri_info *tri)
{  
  //setup some of the global variables
  r1_software_texture_ptr   = tri->texture;
  r1_software_twidth_log2   = tri->twidth_log2;
  r1_software_texture_width = tri->texture_width;
  cur_grads                 = tri->grads;

  //about to draw, setup pertinent global vars (s_frac_add, t_frac_add, and s_t_carry)
  temp_dsdx = qftoi(tri->grads.dsdx);
  temp_dtdx = qftoi(tri->grads.dtdx);

  s_t_carry[1] = (temp_dsdx>>16) + ((temp_dtdx>>16)<<r1_software_twidth_log2); //integral add when t doesnt carry
  s_t_carry[0] = s_t_carry[1] + r1_software_texture_width;                     //integral add when t carrys

  dsdx_frac = (temp_dsdx<<16);
  dtdx_frac = (temp_dtdx<<16);

  span_entry *s = &global_span_list[tri->span_list_head];

  affine_span left;  
  while (s!=global_span_list)
  {    
    float fx = s->s.x;
    float fy = s->s.y;

    left.t   = qftoi(tri->grads.tat00 + (fx * tri->grads.dtdx) + (fy * tri->grads.dtdy)) + tri->grads.t_adjust;
    left.s   = qftoi(tri->grads.sat00 + (fx * tri->grads.dsdx) + (fy * tri->grads.dsdy)) + tri->grads.s_adjust;    

    cur_scanline_texture_func(s->s.scanline_ptr,(s->s.x<<1),&left,s->s.width);    
    s = &global_span_list[s->s.next_tri_span];
  }  
}

void span_draw_affine_lit(span_tri_info *tri)
{  
  //setup some of the global variables
  r1_software_class_instance.set_color_tint(tri->color_tint);

  r1_software_texture_ptr   = tri->texture;
  r1_software_twidth_log2   = tri->twidth_log2;
  r1_software_texture_width = tri->texture_width;
  cur_grads                 = tri->grads;

  //about to draw, setup pertinent global vars (s_t_frac_add, s_t_carry, dldx_fixed)
  temp_dsdx = qftoi(tri->grads.dsdx);
  temp_dtdx = qftoi(tri->grads.dtdx);

  dsdx_frac = (temp_dsdx<<16);
  dtdx_frac = (temp_dtdx<<16);
    
  s_t_carry[1] = (temp_dsdx>>16) + ((temp_dtdx>>16)<<r1_software_twidth_log2); //integral add when t doesnt carry
  s_t_carry[0] = s_t_carry[1] + r1_software_texture_width;           //integral add when t carrys

  //dont forget the light
  dldx_fixed = qftoi(tri->grads.dldx);

  span_entry *s = &global_span_list[tri->span_list_head];  

  affine_span left;  
  while (s!=global_span_list)
  {
#ifndef USE_ASM
    float fx = s->x;
    float fy = s->y;
    left.t   = qftoi(tri->grads.tat00 + (fx * tri->grads.dtdx) + (fy * tri->grads.dtdy)) + tri->grads.t_adjust;
    left.s   = qftoi(tri->grads.sat00 + (fx * tri->grads.dsdx) + (fy * tri->grads.dsdy)) + tri->grads.s_adjust;
    left.l   = qftoi(tri->grads.lat00 + (fx * tri->grads.dldx) + (fy * tri->grads.dldy));
#else

    _asm
    {
      mov edi,dword ptr [s]
      
      fild dword ptr [edi]span_entry.s.x
      fild dword ptr [edi]span_entry.s.y

      fld dword ptr [cur_grads]tri_gradients.dtdx
      fld dword ptr [cur_grads]tri_gradients.dtdy

      //dtdy  dtdx  y   x
      fmul st(0),st(2)
      fxch st(1)

      //dtdx  dtdy*y  y  x

      fmul st(0),st(3)
      fxch st(1)

      //dtdy*y  dtdx*x  y  x

      fld dword ptr [cur_grads]tri_gradients.dsdx
      fld dword ptr [cur_grads]tri_gradients.dsdy

      //dsdy  dsdx  dtdy*y  dtdx*x  y  x

      fmul st(0),st(4)
      fxch st(1)

      //dsdx  dsdy*y  dtdy*y  dtdx*x  y  x

      fmul st(0),st(5)
      fxch st(1)

      //dsdy*y  dsdx*x  dtdy*y  dtdx*x  y  x
      fld dword ptr [cur_grads]tri_gradients.dldx
      fld dword ptr [cur_grads]tri_gradients.dldy

      //dldy  dldx  dsdy*y  dsdx*x  dtdy*y  dtdx*x  y  x

      fmul st(0),st(6)
      fxch st(1)

      //dldx  dldy*y  dsdy*y  dsdx*x  dtdy*y  dtdx*x  y  x

      fmul st(0),st(7)
      fxch st(4)
      
      //dtdy*y  dldx*x  dsdy*y  dsdx*x  dldy*y  dtdx*x  y  x
      
      fadd dword ptr [cur_grads]tri_gradients.tat00
      fxch st(2)

      //dsdy*y  dldx*x  dtdy*y+tat00  dsdx*x  dldy*y  dtdx*x  y  x

      fadd dword ptr [cur_grads]tri_gradients.sat00
      fxch st(4)

      //dldy*y  dldx*x  dtdy*y+tat00  dsdx*x  dsdy*y+sat00  dtdx*x  y  x

      fadd dword ptr [cur_grads]tri_gradients.lat00
      fxch st(2)

      //dtdy*y+tat00  dldx*x  dldy*y+lat00  dsdx*x  dsdy*y+sat00  dtdx*x  y  x
      faddp st(5),st(0)

      //dldx*x  dldy*y+lat00  dsdx*x  dsdy*y+sat00  dtdx*x+dtdy*y+tat00  y  x
      fxch st(3)

      //dsdy*y+sat00  dldy*y+lat00  dsdx*x  dldx*x  dtdx*x+dtdy*y+tat00  y  x
      faddp st(2),st(0)

      //dldy*y+lat00  dsdx*x+dsdy*y+sat00  dldx*x  dtdx*x+dtdy*y+tat00  y  x
      faddp st(2),st(0)

      //dsdx*x+dsdy*y+sat00  dldx*x+dldy*y+lat00  dtdx*x+dtdy*y+tat00  y  x
      fxch st(3)
      
      //y  dldx*x+dldy*y+lat00  dtdx*x+dtdy*y+tat00  dsdx*x+dsdy*y+sat00  x
      fstp st(0)
      fxch st(3)
      
      //x  dtdx*x+dtdy*y+tat00  dsdx*x+dsdy*y+sat00  dldx*x+dldy*y+lat00
      fstp st(0)

      fistp dword ptr [left]affine_span.t
      fistp dword ptr [left]affine_span.s
      fistp dword ptr [left]affine_span.l

      mov eax,dword ptr [left]affine_span.s
      mov ebx,dword ptr [left]affine_span.t

      add eax,dword ptr [cur_grads]tri_gradients.s_adjust
      add ebx,dword ptr [cur_grads]tri_gradients.t_adjust

      mov dword ptr [left]affine_span.s,eax
      mov dword ptr [left]affine_span.t,ebx
    }
#endif
    
    cur_scanline_texture_func(s->s.scanline_ptr,(s->s.x<<1),&left,s->s.width);    
    s = &global_span_list[s->s.next_tri_span];
  }  
}
