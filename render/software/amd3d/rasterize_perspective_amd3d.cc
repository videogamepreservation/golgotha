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
#include "software/amd3d/amd3d.h"

#ifndef USE_AMD3D

void span_draw_perspective_lit_amd3d(span_tri_info *tri) {}
void span_draw_perspective_unlit_amd3d(span_tri_info *tri) {}

#else

void span_draw_perspective_lit_amd3d(span_tri_info *tri)
{
  //setup some of the global variables
  r1_software_class_instance.set_color_tint(tri->color_tint);
  
  r1_software_texture_ptr    = tri->texture;
  r1_software_twidth_log2    = tri->twidth_log2;
  r1_software_texture_width  = tri->texture_width;
  r1_software_texture_height = tri->texture_height;
  cur_grads                  = tri->grads;
    
  s_mask = ((r1_software_texture_width -1)<<16) | 0xFFFF;
  t_mask = ((r1_software_texture_height-1)<<16) | 0xFFFF;
  
  //dont forget the light
  //dldx_fixed = qftoi(tri->grads.dldx);
  _asm
  {
    movd mm0,dword ptr [cur_grads]tri_gradients.dldx
        
    pf2id (m0, m0)
    
    movd dword ptr [dldx_fixed],mm0
  }

  span_entry *s = &global_span_list[tri->span_list_head];

  perspective_span left;  
  while (s!=global_span_list)
  {
    left.ooz = s->s.ooz;
    
    //float left.toz = cur_grads.tozat00 + (s->x * cur_grads.dtozdx) + (s->y * cur_grads.dtozdy);
    //float left.soz = cur_grads.sozat00 + (s->x * cur_grads.dsozdx) + (s->y * cur_grads.dsozdy);
    //float left.l   = qftoi(cur_grads.lat00 + (s->x * cur_grads.dldx) + (s->y * cur_grads.dldy));
    _asm
    {
      mov edi, dword ptr [s]
      lea eax, dword ptr [cur_grads]

      pi2fd (m0, _edi)
      movq  mm1,qword ptr [eax]tri_gradients.dsozdx
      
      pfmul (m1, m0)
      movq mm2,qword ptr [eax]tri_gradients.dtozdx
      
      pfmul (m2, m0)
      movq mm4,qword ptr [eax]tri_gradients.sozat00
      
      pfacc (m1, m2)
      movq mm3,qword ptr [eax]tri_gradients.dldx

      //low  32bits of m1 - fx*dsdx + fy*dsdy
      //high 32bits of m1 - fx*dtdx + fy*dtdy
      
      pfadd (m1, m4)
      pfmul (m3, m0)

      //low  32bits of m1 - sat00 + fx*dsdx + fy*dsdy
      //high 32bits of m1 - tat00 + fx*dtdx + fy*dtdy

      pfacc (m3, m3) //2nd arg can be whatever
      movq mm4,dword ptr [eax]tri_gradients.lat00
      
      //low 32 bits of m3 - fx*dldx + fy*dldy
      
      pfadd (m3, m4)
      movq qword ptr [left]perspective_span.soz,mm1

      //low 32 bits of m3 - lat00 + fx*dldx + fy*dldy

      pf2id (m5,m3)
      
      //low 32 bits of m5 - (int) (lat00 + fx*dldx + fy*dldy)

      movd dword ptr [left]perspective_span.l,mm5
    }

    cur_scanline_texture_func(s->s.scanline_ptr,(s->s.x<<1),&left,s->s.width);    
    s = &global_span_list[s->s.next_tri_span];
  }
}

void span_draw_perspective_unlit_amd3d(span_tri_info *tri)
{
  //setup some of the global variables
  r1_software_class_instance.set_color_tint(tri->color_tint);
  
  r1_software_texture_ptr    = tri->texture;
  r1_software_twidth_log2    = tri->twidth_log2;
  r1_software_texture_width  = tri->texture_width;
  r1_software_texture_height = tri->texture_height;
  cur_grads                  = tri->grads;
    
  s_mask = ((r1_software_texture_width -1)<<16) | 0xFFFF;
  t_mask = ((r1_software_texture_height-1)<<16) | 0xFFFF;
  
  span_entry *s = &global_span_list[tri->span_list_head];

  perspective_span left;  
  while (s!=global_span_list)
  {
    left.ooz = s->s.ooz;
    
    //left.toz = cur_grads.tozat00 + (fx * cur_grads.dtozdx) + (fy * cur_grads.dtozdy);
    //left.soz = cur_grads.sozat00 + (fx * cur_grads.dsozdx) + (fy * cur_grads.dsozdy);
    //left.l   = qftoi(cur_grads.lat00 + (fx * cur_grads.dldx) + (fy * cur_grads.dldy));
    _asm
    {
      mov edi, dword ptr [s]
      lea eax, dword ptr [cur_grads]

      pi2fd (m0, _edi)
      movq  mm1,qword ptr [eax]tri_gradients.dsozdx
      
      pfmul (m1, m0)
      movq mm2,qword ptr [eax]tri_gradients.dtozdx
      
      pfmul (m2, m0)
      movq mm4,qword ptr [eax]tri_gradients.sozat00
      
      pfacc (m1, m2)

      //low  32bits of m1 - fx*dsdx + fy*dsdy
      //high 32bits of m1 - fx*dtdx + fy*dtdy
      
      pfadd (m1, m4)

      //low  32bits of m1 - sat00 + fx*dsdx + fy*dsdy
      //high 32bits of m1 - tat00 + fx*dtdx + fy*dtdy

      movq qword ptr [left]perspective_span.soz,mm1
    }

    cur_scanline_texture_func(s->s.scanline_ptr,(s->s.x<<1),&left,s->s.width);    
    s = &global_span_list[s->s.next_tri_span];
  }
}

#endif