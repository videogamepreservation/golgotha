/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "software/r1_software_globals.hh"

void texture_scanline_affine_unlit(w16 *start_pixel,
                                   sw32 start_x,
                                   void *left, //actually affine_span *left
                                   sw32 width)
{
  _asm
  {  
    mov eax,dword ptr [width]
    
    mov edi,dword ptr [left]
    mov dword ptr [width_global],eax
    
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

    mov ebx,dword ptr [width_global] //setup loop counter
    add edi,dword ptr [start_x]

    push ebp            

    ALIGN 16        
loopcount:
    mov ax,word ptr [esi*2]    
    add edx,dword ptr [dtdx_frac]

    sbb ebp,ebp
    mov word ptr [edi],ax

    add edi,2
    add ecx,dword ptr [dsdx_frac]

    adc esi,dword ptr [4+s_t_carry+ebp*4]            
    dec ebx
    
    jnz loopcount
    pop ebp
  }  
}
