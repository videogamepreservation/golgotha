/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "software/r1_software_globals.hh"

void texture_scanline_affine_unlit_holy_blend(w16 *start_pixel,
                                              sw32 start_x,
                                              void *left, //affine_span *left
                                              sw32 width)
{
  _asm
  {  
    mov eax,dword ptr [width]
    
    mov edi,dword ptr [left]
    mov dword ptr [width_global],eax
    
    mov eax,dword ptr [dsdx_frac]
    mov ebx,dword ptr [dtdx_frac]

    mov dword ptr [s_frac_add_patch_2+2],eax
    mov dword ptr [t_frac_add_patch_2+2],ebx

    mov esi,dword ptr [r1_software_texture_ptr] //esi has texture base ptr
    mov ebx,dword ptr [edi]affine_span.s //get s in ecx
    
    shr esi,1 //get half the texture pointer   
    mov eax,dword ptr [edi]affine_span.t //get t in edx

    sar ebx,16 //get integral part of s
    mov cl,byte ptr [r1_software_twidth_log2]
    
    sar eax,16 //get integral part of t
    add esi,ebx //setup starting texture ptr

    shl eax,cl //shift left to multiply t by texture width    
    
    mov edx,dword ptr [edi]affine_span.s //get s in edx
    add esi,eax //finish setup of starting texture ptr

    sal edx,16 //shift left to get fractional part of s
    mov ecx,dword ptr [edi]affine_span.t //get t again in ecx
            
    sal ecx,16 //shift left to get fractional part of t                
    mov edi,dword ptr [start_pixel]

    add edi,dword ptr [start_x]
    push ebp    

    mov ebp,dword ptr [width_global] //setup loop counter    
    mov eax,0

    mov ebx,0
    
    ALIGN 16
loopcount:
    mov ax,[esi*2]
    and eax,eax

    jz  skip_a_pixel
    mov bx,[edi]

    shr eax,1
    and ebx,dword ptr [pre_blend_and]
        
    shr ebx,1
    and eax,dword ptr [post_blend_and]

    add eax,ebx
    mov [edi],ax

skip_a_pixel:  
t_frac_add_patch_2:
    add ecx,0xDDDDDDDD     
    sbb ebx,ebx    
    
    add edi,2
s_frac_add_patch_2:
    add edx,0xDDDDDDDD        
        
    adc esi,dword ptr [4+s_t_carry+ebx*4]
    mov ebx,0 //gotta clear this guy

    dec ebp    
    jnz loopcount
    
end_affine_lit:
    pop ebp
  }  
}
