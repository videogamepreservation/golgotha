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

void span_draw_affine_lit_amd3d(span_tri_info *tri) {}
void span_draw_affine_unlit_amd3d(span_tri_info *tri) {}

#else

void span_draw_affine_unlit_amd3d(span_tri_info *tri)
{
  //setup some of the global variables
  r1_software_class_instance.set_color_tint(tri->color_tint);

  r1_software_texture_ptr   = tri->texture;
  r1_software_twidth_log2   = tri->twidth_log2;
  r1_software_texture_width = tri->texture_width;
  cur_grads                 = tri->grads;

  //about to draw, setup pertinent global vars (s_t_frac_add, s_t_carry, dldx_fixed)
  _asm
  {
    movd mm0,dword ptr [cur_grads]tri_gradients.dsdx
    nop

    movd mm1,dword ptr [cur_grads]tri_gradients.dtdx    
    pf2id (m0, m0)
        
    movd dword ptr [temp_dsdx],mm0
    pf2id (m1, m1)

    movd dword ptr [temp_dtdx],mm1
  }

  dsdx_frac = (temp_dsdx<<16);
  dtdx_frac = (temp_dtdx<<16);

  s_t_carry[1] = (temp_dsdx>>16) + ((temp_dtdx>>16)<<r1_software_twidth_log2); //integral add when t doesnt carry
  s_t_carry[0] = s_t_carry[1] + r1_software_texture_width;           //integral add when t carrys

  span_entry *s = &global_span_list[tri->span_list_head];  

  affine_span left;  
  while (s!=global_span_list)
  { 
    left.ooz = s->s.ooz;
    
    _asm
    {
      //the ordering of variables in the structures used here is AMAZINGLY important
      
      mov edi, dword ptr [s]
      lea eax, dword ptr [cur_grads]

      pi2fd (m0, _edi)
      movq  mm1,qword ptr [eax]tri_gradients.dsdx
      
      pfmul (m1, m0)
      movq mm2,qword ptr [eax]tri_gradients.dtdx
      
      pfmul (m2, m0)
      movq mm4,qword ptr [eax]tri_gradients.sat00
      
      pfacc (m1, m2)
      
      //low  32bits of m1 - fx*dsdx + fy*dsdy
      //high 32bits of m1 - fx*dtdx + fy*dtdy
      
      pfadd (m1, m4)
      
      //low  32bits of m1 - sat00 + fx*dsdx + fy*dsdy
      //high 32bits of m1 - tat00 + fx*dtdx + fy*dtdy

      pf2id (m5,m1)

      paddd mm5, qword ptr [eax]tri_gradients.s_adjust
      
      movq qword ptr [left]affine_span.s,mm5
    }

    cur_scanline_texture_func(s->s.scanline_ptr,(s->s.x<<1),&left,s->s.width);    
    s = &global_span_list[s->s.next_tri_span];
  }
}

void span_draw_affine_lit_amd3d(span_tri_info *tri)
{
  //setup some of the global variables
  r1_software_class_instance.set_color_tint(tri->color_tint);

  r1_software_texture_ptr   = tri->texture;
  r1_software_twidth_log2   = tri->twidth_log2;
  r1_software_texture_width = tri->texture_width;
  cur_grads                 = tri->grads;

  //about to draw, setup pertinent global vars (s_t_frac_add, s_t_carry, dldx_fixed)
  _asm
  {
    movd mm0,dword ptr [cur_grads]tri_gradients.dsdx
    movd mm1,dword ptr [cur_grads]tri_gradients.dtdx
    
    pf2id (m0, m0)
    movd mm2,dword ptr [cur_grads]tri_gradients.dldx
        
    pf2id (m1, m1)
    movd dword ptr [temp_dsdx],mm0

    pf2id (m2, m2)
    movd dword ptr [temp_dtdx],mm1

    movd dword ptr [dldx_fixed],mm2
  }

  dsdx_frac = (temp_dsdx<<16);
  dtdx_frac = (temp_dtdx<<16);

  s_t_carry[1] = (temp_dsdx>>16) + ((temp_dtdx>>16)<<r1_software_twidth_log2); //integral add when t doesnt carry
  s_t_carry[0] = s_t_carry[1] + r1_software_texture_width;           //integral add when t carrys

  span_entry *s = &global_span_list[tri->span_list_head];  

  affine_span left;  
  while (s!=global_span_list)
  { 
    left.ooz = s->s.ooz;
    
    _asm
    {
      //the ordering of variables in the structures used here is AMAZINGLY important
      
      mov edi, dword ptr [s]
      lea eax, dword ptr [cur_grads]

      pi2fd (m0, _edi)
      movq  mm1,qword ptr [eax]tri_gradients.dsdx
      
      pfmul (m1, m0)
      movq mm2,qword ptr [eax]tri_gradients.dtdx
      
      pfmul (m2, m0)
      movq mm4,qword ptr [eax]tri_gradients.sat00
      
      pfacc (m1, m2)
      movq mm3,qword ptr [eax]tri_gradients.dldx

      //low  32bits of m1 - fx*dsdx + fy*dsdy
      //high 32bits of m1 - fx*dtdx + fy*dtdy
      
      pfadd (m1, m4)
      pfmul (m3, m0)

      //low  32bits of m1 - sat00 + fx*dsdx + fy*dsdy
      //high 32bits of m1 - tat00 + fx*dtdx + fy*dtdy

      pfacc (m3,m0) //2nd arg can be whatever
      movq mm4,dword ptr [eax]tri_gradients.lat00
      
      //low 32 bits of m3 - fx*dldx + fy*dldy
      
      pfadd (m3, m4)
      //low 32 bits of m3 - lat00 + fx*dldx + fy*dldy
      pf2id (m5,m1)

      pf2id (m6,m3)
      paddd mm5, qword ptr [eax]tri_gradients.s_adjust
      
      movq qword ptr [left]affine_span.s,mm5
      movd dword ptr [left]affine_span.l,mm6
    }
    
    cur_scanline_texture_func(s->s.scanline_ptr,(s->s.x<<1),&left,s->s.width);    
    s = &global_span_list[s->s.next_tri_span];
  }
}

#endif