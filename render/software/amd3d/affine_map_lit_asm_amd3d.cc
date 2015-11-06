/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "software/r1_software_globals.hh"
#include "software/inline_fpu.hh"

w32 *texture_affine_lit_starter_amd3d()
{
  w32 returnval;
  _asm
  {
    mov eax,OFFSET dumb_addr
dumb_addr:
    mov returnval,eax
  }
  return (w32 *)returnval;
}

extern w8 last_bh;

void texture_scanline_affine_lit_amd3d(w16 *start_pixel,
                                       sw32 start_x,
                                       void *left,//perspective_span *left,
                                       sw32 width)
{
  start_pixel = (w16 *)((w8 *)start_pixel + start_x);
  last_bh = 0;

  _asm
  {
    //num_subdivisions = width >> 4;
    //num_leftover     = width & 15;
        
    mov edi,dword ptr [left]

    mov eax,dword ptr [width]
    mov ebx,dword ptr [width]
    
    shr ebx,4
    and eax,15

    mov dword ptr [num_leftover],eax
    mov dword ptr [num_subdivisions],ebx
    
    //esi = starting_s_coordinate >> 16 + starting_t_coordinate >> 16 << r1_software_twidth_log2
    //ecx = starting_s_coordinate << 16
    //edx = starting_t_coordinate << 16
    //dx  = starting_light_value
    
    mov eax,dword ptr [edi]affine_span.s
    mov ebx,dword ptr [edi]affine_span.t

    sar eax,16
    mov esi,dword ptr [r1_software_texture_ptr]

    sar ebx,16
    mov ecx,dword ptr [edi]affine_span.s
    
    shr esi,1
    mov cl,byte ptr [r1_software_twidth_log2]

    shl ebx,cl

    sal ecx,16
    mov edx,dword ptr [edi]affine_span.t

    sal edx,16
    add eax,ebx

    mov dx,word ptr [edi+AFFINE_SPAN_L]
    mov edi,dword ptr [start_pixel]
    
    add esi,eax
    push ebp

    mov ch,dl //copy the initial error from the 1st lighting value

    mov ebx,0
    mov eax,0 //must make sure the high bits of these are zeroed out
  }  
  
  while (num_subdivisions)
  {
    _asm
    {
      mov cl,4
      mov bh,byte ptr [last_bh] //necessary? bh should be preserved from the bottom of the loop..

      add ch,0 //this is used to clear the carry flag (the actual clc instruction takes 2 cycles. stupid.)

      ALIGN 16

      //high 16 bits of ecx is the fractional s component
      //high 16 bits of edx is the fractional t component

      //eax is used to lookup the texel as well as the low 8-bits of the lit texel
      //ebx is used to lookup the high 8-bits of the lit texel
      //ebp is used to detect a t-carry as well as lookup the lit texel
      //cl  is the loop count variable
      //dx  is the lighting value (8 bits integer, 8 bits fraction)
      //ch  is the lighting error
      //bh  is used to dither the lighting (mov bh,0 then add ch,dl then adc bh,dh)

    looper1:
      adc bh,dh
      add edi,8 //the only convenient place for the stepping of edi was way up here

      movzx eax,word ptr [esi*2]
      add edx,dword ptr [dtdx_frac]
 
      sbb ebp,ebp
      mov bl,ah

      add ecx,dword ptr [dsdx_frac]
      mov ah,bh
      
      adc esi,dword ptr [4+s_t_carry+ebp*4]
      mov ebp,dword ptr [0xDEADBEEF+ctable_size_bytes+ebx*4]
        
      add ebp,dword ptr [0xDEADBEEF+eax*4]
      add edx,dword ptr [dldx_fixed]

      mov bh,0
      add ch,dl

      mov word ptr [edi-8],bp
      adc bh,dh

      movzx eax,word ptr [esi*2]  //first pixel is finished aroundhere
      add edx,dword ptr [dtdx_frac]

      sbb ebp,ebp
      mov bl,ah
                
      add ecx,dword ptr [dsdx_frac]
      mov ah,bh

      adc esi,dword ptr [4+s_t_carry+ebp*4]
      mov ebp,dword ptr [0xDEADBEEF+ctable_size_bytes+ebx*4]
        
      add ebp,dword ptr [0xDEADBEEF+eax*4]
      add edx,dword ptr [dldx_fixed]
              
      mov bh,0
      add ch,dl

      mov word ptr [edi-6],bp
      adc bh,dh

      movzx eax,word ptr [esi*2]
      add edx,dword ptr [dtdx_frac]

      sbb ebp,ebp
      mov bl,ah

      add ecx,dword ptr [dsdx_frac]
      mov ah,bh

      adc esi,dword ptr [4+s_t_carry+ebp*4]
      mov ebp,dword ptr [0xDEADBEEF+ctable_size_bytes+ebx*4]
       
      add ebp,dword ptr [0xDEADBEEF+eax*4]
      add edx,dword ptr [dldx_fixed]

      mov bh,0
      add ch,dl
      
      mov word ptr [edi-4],bp
      adc bh,dh

      movzx eax,word ptr [esi*2]  //first pixel is finished aroundhere
      add edx,dword ptr [dtdx_frac]

      sbb ebp,ebp
      mov bl,ah

      add ecx,dword ptr [dsdx_frac]
      mov ah,bh

      adc esi,dword ptr [4+s_t_carry+ebp*4]
      mov ebp,dword ptr [0xDEADBEEF+ctable_size_bytes+ebx*4]
        
      add ebp,dword ptr [0xDEADBEEF+eax*4]
      add edx,dword ptr [dldx_fixed]
        
      mov bh,0
      add ch,dl

      mov word ptr [edi-2],bp
      dec cl //thank god this doesnt modify the carry flag (the above add ch,dl needs to effect the adc bh,dh at the top of the loop)
      
      jnz looper1
      
      adc bh,0 //if we're done looping, save the last carry information here
      mov byte ptr [last_bh],bh
    }

    _asm dec dword ptr [num_subdivisions]
  }

  if (num_leftover)
  {
    _asm
    {
      mov cl, byte ptr [num_leftover]
      mov bh, byte ptr [last_bh] //necessary?

      add ch,0 //this is used to clear the carry flag (the actual clc instruction takes 2 cycles. stupid.)

      ALIGN 16

    looper3:
      adc bh,dh
      add edi,2

      movzx eax,word ptr [esi*2]
      add edx,dword ptr [dtdx_frac]

      sbb ebp,ebp
      mov bl,ah

      add ecx,dword ptr [dsdx_frac]
      mov ah,bh

      adc esi,dword ptr [4+s_t_carry+ebp*4]
      mov ebp,dword ptr [0xDEADBEEF+ctable_size_bytes+ebx*4]

      add ebp,dword ptr [0xDEADBEEF+eax*4]
      add edx,dword ptr [dldx_fixed]
      
      mov bh,0
      add ch,dl
      
      mov word ptr [edi-2],bp
      dec cl //thank god this doesnt modify the carry flag (the above add ch,dl needs to effect the adc bh,dh at the top of the loop)
      
      jnz looper3
    }
  }  
  _asm pop ebp
}

w32 *texture_affine_lit_sentinel_amd3d()
{
  w32 returnval;
  _asm
  {
    mov eax,OFFSET dumb_addr
dumb_addr:
    mov returnval,eax
  }
  return (w32 *)returnval;
}

void insert_color_modify_address_low(w32 *address);
void insert_color_modify_address_high(w32 *address);
extern w32 color_modify_list[];
extern sw32 num_color_modifies;

void setup_color_modify_affine_lit_amd3d()
{
  w32 *stop = texture_affine_lit_sentinel_amd3d();

  w32 *search = texture_affine_lit_starter_amd3d();
  //start searching for 0xDEADBEEF
  while (search < stop)
  {
    //casting craziness
    search = (w32 *)((w8 *)search + 1);
    if (*search==0xDEADBEEF)
    {
      insert_color_modify_address_low(search);
    }
    else
    if (*search==(0xDEADBEEF + ctable_size_bytes))
    {
      insert_color_modify_address_high(search);
    }
  }
}
