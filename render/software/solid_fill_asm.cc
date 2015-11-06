/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "software/r1_software_globals.hh"

void texture_scanline_solid_fill(w16 *start_pixel,
                                 sw32 start_x,
                                 void *left,//solid_blend_span *left,
                                 sw32 width)
{  
  __asm
  {        
    mov edi,left
    mov ecx,width

    mov eax,[edi]solid_blend_span.color    
    mov edi,start_pixel

    add edi,start_x
    nop

    test ecx,1       //even # of pixels?
    jz   start_loop  //yes, begin looped drawing

    //no, draw single pixel 1st    
    mov word ptr [edi],ax
    add edi,2
    dec ecx
    jz  done

start_loop:
    shr ecx,1
    rep stosd    

done:

  }
}
