/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "software/r1_software_globals.hh"

void texture_scanline_affine_unlit_alpha(w16 *start_pixel,
                                         sw32 start_x,
                                         void *left, //actually affine_span *left
                                         sw32 width)
{
  _asm
  {  
    mov edi,dword ptr [left]
    
    mov esi,dword ptr [r1_software_texture_ptr] //esi has texture base ptr
    mov ebx,dword ptr [edi]affine_span.s //get s in ecx
    
    shr esi,1 //get half the texture pointer   
    mov eax,dword ptr [edi]affine_span.t //get t in edx

    sar ebx,16 //get integral part of s
    mov cl,byte ptr [r1_software_twidth_log2]
    
    sar eax,16 //get integral part of t
    add esi,ebx //setup starting texture ptr

    shl eax,cl //shift left to multiply t by texture width    
    
    mov ecx,dword ptr [edi]affine_span.s //get s in ecx
    add esi,eax //finish setup of starting texture ptr

    sal ecx,16 //shift left to get fractional part of s
    mov edx,[edi]affine_span.t //get t again in edx
            
    sal edx,16 //shift left to get fractional part of t                
    mov edi,dword ptr [start_pixel]

    mov ebx,dword ptr [width] //setup loop counter
    add edi,dword ptr [start_x]

    mov ch,16 //initial alpha accumulator value is 16 (since max alpha 15 should draw the pixel)
    push ebp

    ALIGN 16        
loopcount:
    mov ax,word ptr [esi*2]    
    add edx,dword ptr [dtdx_frac]

    sbb ebp,ebp
    and ch,240

    mov cl,ah
    add edi,2

    and eax,4095
    add ecx,dword ptr [dsdx_frac]

    adc esi,dword ptr [4+s_t_carry+ebp*4]            
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
