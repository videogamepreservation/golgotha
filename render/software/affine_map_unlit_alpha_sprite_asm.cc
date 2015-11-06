/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "software/r1_software_globals.hh"

void texture_scanline_affine_unlit_alpha_sprite(w16 *start_pixel,
                                                sw32 start_x,
                                                void *left, //actually affine_span *left
                                                sw32 width)
{
  _asm
  {  
    mov ebx,dword ptr [width]
    mov edi,dword ptr [left]
    
    mov eax,dword ptr [dsdx_frac]
    mov esi,dword ptr [r1_software_texture_ptr] //esi has texture base ptr

    mov dword ptr [s_frac_add_patch+2],eax
    mov ecx,dword ptr [edi]affine_span.s //get s in ecx
    
    shr esi,1 //get half the texture pointer
    mov eax,dword ptr [edi]affine_span.t

    sar eax,16 //get integral start t
    mov edx,dword ptr [edi]affine_span.s //get s in edx
    
    sar ecx,16 //get integral part of s
    mov edi,dword ptr [start_pixel]
    
    sal edx,16 //shift left to get fractional part of s
    add esi,ecx //setup starting texture ptr (add integral start s to esi)

    add edi,dword ptr [start_x]
    mov cl,byte ptr [r1_software_twidth_log2]

    shl eax,cl
    
    push ebp        
    add esi,eax

    mov ch,16 //initial alpha accumulator value is 16 (since max alpha 15 should draw the pixel)
    mov ebp,dword ptr [4+s_t_carry]
    
    ALIGN 16        
loopcount:
    mov ax,word ptr [esi*2]
s_frac_add_patch:
    add edx,0xDDDDDDDD
    
    adc esi,ebp
    and ch,240

    mov cl,ah    
    and eax,4095    
    
    add edi,2    
    add ch,cl
                   
    jnc skip_pixel
    mov ax,word ptr [alpha_table+eax*2]
    
    mov word ptr [edi-2],ax
    add ch,16

skip_pixel:
            
    dec ebx
    jnz loopcount
    
    pop ebp
  }  
}
