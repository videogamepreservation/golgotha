/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "software/r1_software_globals.hh"

void texture_scanline_solid_blend_half(w16 *start_pixel,
                                       sw32 start_x,
                                       void *left,//solid_blend_span *left,
                                       sw32 width)
{
  __asm
  {        
    mov edi,left

    mov ecx,pre_blend_and
    mov eax,[edi]solid_blend_span.color
    
    mov dword ptr and_patch+2,ecx
    mov edx,width

    and eax,ecx
    mov edi,start_pixel

    shr eax,1    
    add edi,start_x    
    
    mov esi,edi    
    mov  ebx,0

    test edx,1       //even # of pixels?
    jz   start_loop  //yes, begin looped drawing

    //no, draw single pixel 1st
    mov bx,[esi]
    add edi,2
    
    and ebx,pre_blend_and    
    add esi,2

    shr ebx,1    
    add ebx,eax
    
    mov [edi-2],bx
    dec edx

//enter main loop
start_loop:
    shr edx,1
    jz  end_sbh

bogus_label:    
    mov ebx,[esi]
    add edi,4
    
and_patch:
    and ebx,0xDDDDDDDD
    add esi,4      

    shr ebx,1
    //nop

    add ebx,eax
    //nop

    mov [edi-4],ebx
    dec edx
    
    jnz bogus_label

end_sbh:
  }
}