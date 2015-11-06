/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "software/r1_software_globals.hh"
#include "software/inline_fpu.hh"
#include "software/amd3d/amd3d.h"

w32 *texture_perspective_lit_starter_amd3d()
{
  bogus_label:

  w32 returnval;
  _asm
  {
    lea eax,bogus_label
    mov dword ptr [returnval],eax
  }
  return (w32 *)returnval;
}

extern sw32 had_subdivisions;
extern w8 last_bh2;

//instead of using left_s, left_t, right_s, and right_t,
//the divides and multiplies are nicely vectorized by the amd3d,
//and storing them is a single quad store to an array of 2 floats,
//rather than two dword stores to two seperate floats

sw32 left_s_t[2];
sw32 right_s_t[2];

float mmx0[2];
float mmx1[2];
float mmx2[2];
float mmx3[2];
float mmx4[2];
float mmx5[2];
float mmx6[2];
float mmx7[2];

void texture_scanline_perspective_lit_amd3d(w16 *start_pixel,
                                            sw32 start_x,
                                            void *_left,//perspective_span *left,
                                            sw32 width)
{
  start_pixel = (w16 *)((w8 *)start_pixel + start_x);

  perspective_span *left = (perspective_span *)_left;
  
  last_bh2 = 0;

  _asm
  {
    //left_z = 1.f / left->ooz;
    //left_s = qftoi(left->soz * left_z) + cur_grads.s_adjust;
    //left_t = qftoi(left->toz * left_z) + cur_grads.t_adjust;
    
    //sw32 had_subdivisions = width & (~15);
    //num_subdivisions = width >> 4;
    //num_leftover     = width & 15;
    
    mov edi,dword ptr [left]
    mov eax,dword ptr [width]

    movd mm0, dword ptr [edi]perspective_span.ooz
    mov ebx,eax
    
    pfrcp (m1, m0)
    and eax,15

    shr ebx,4
    punpckldq mm0, mm0 //duplicate low 32bits of m0 into high 32 bits of m0
        
    pfrcpit1 (m0, m1)
    mov ecx,dword ptr [width]
    
    movq mm2, qword ptr [edi]perspective_span.soz
    mov dword ptr [num_leftover],eax
    
    pfrcpit2 (m0, m1)
    and ecx,(~15)
    
    mov eax,dword ptr [edi]perspective_span.l
    mov dword ptr [num_subdivisions],ebx

    pfmul (m2, m0)
    mov dword ptr [had_subdivisions],ecx
    
    mov dword ptr [left_l],eax
    //clear these out
    mov dword ptr [dsdx_frac],0

    //high 32 bits of mm2 - toz / ooz (aka t)
    //low  32 bits of mm2 - soz / ooz (aka s)

    pf2id (m3, m2)
    mov dword ptr [dtdx_frac],0

    //high 32 bits of mm3 - toz / ooz (aka t) - truncated ints
    //low  32 bits of mm3 - soz / ooz (aka s) - truncated ints

    paddd mm3, qword ptr [cur_grads]tri_gradients.s_adjust

    //high 32 bits of mm3 - t + t_adjust
    //low  32 bits of mm3 - s + s_adjust

    movq qword ptr [left_s_t], mm3
  }

  if (num_subdivisions)
  {
    _asm
    {
      //ooz_right = left->ooz + (cur_grads.doozdxspan);
      //soz_right = left->soz + (cur_grads.dsozdxspan);
      //toz_right = left->toz + (cur_grads.dtozdxspan);

      //edi still has dword ptr [left]
      lea ebx,dword ptr [cur_grads]
      nop

      movd mm1, dword ptr [edi]perspective_span.ooz
      mov esi,dword ptr [r1_software_texture_ptr]
      
      movd mm3, dword ptr [ebx]tri_gradients.doozdxspan
      mov eax,dword ptr [left_s_t] //left_s
      
      shr esi,1
      movq mm0, qword ptr [edi]perspective_span.soz
      
      pfadd (m1, m3)
      movq mm2, qword ptr [ebx]tri_gradients.dsozdxspan
      
      sar eax,16   //get integral left_s into eax
      mov edi,dword ptr [start_pixel]
      
      pfrcp (m6, m1)
      movq mm7,mm1
      
      pfadd (m0, m2)
      mov ebx,dword ptr [left_s_t+4] //left_t      
      
      //calculate the 1st right_z in mm7
      sar ebx,16 //get integral left_t into ebx
      punpckldq mm7, mm7 //duplicate high 32bits of mm7 into low 32 bits of mm7
      
      pfrcpit1 (m7, m6)
      mov edx,dword ptr [left_s_t+4] //left_t
      
      mov cl,byte ptr [r1_software_twidth_log2]
      add esi,eax
      
      pfrcpit2 (m7, m6)
      
      

      //calculate starting fractional and integral values for s and t
      //esi = starting_s_coordinate >> 16 + starting_t_coordinate >> 16 << r1_software_twidth_log2
      //ecx = starting_s_coordinate << 16
      //edx = starting_t_coordinate << 16
      //dx  = starting_light_value

      //some stuff has been moved up, interleaved w/the mmx code above
      
      shl ebx,cl //multiply integral left_t by texture width
      
      sal edx,16 //get fractional left_t into edx
      mov ecx,dword ptr [left_s_t] //left_s
    
      sal ecx,16
      add esi,ebx

      mov dx,word ptr [left_l]
      mov ch,dl //store the initial lighting error from the 1st lighting value
                //CH MUST not be touched between here and the actual rasterization loop
    }

    while (num_subdivisions)
    {
      _asm
      {
        //right_s = qftoi(soz_right * right_z);
        //right_t = qftoi(toz_right * right_z);
        
        //soz_right and toz_right are in mm0
        //right_z is in mm7
        pfmul (m7, m0)
        
        pf2id (m7, m7)

        movq qword ptr [right_s_t],mm7

      //calculate ooz_right, soz_right, toz_right, and right_z for the end of the next span. if there are
      //more subdivisions, calculate the end of the next span. if there are no more and there is > 1 leftover
      //in the leftover span, calculate the end of that.

      //if (num_subdivisions!=1)
      //{
          cmp dword ptr [num_subdivisions],1
          je  last_subdivision
        
          //ooz_right += (cur_grads.doozdxspan);
          //soz_right += (cur_grads.dsozdxspan);
          //toz_right += (cur_grads.dtozdxspan);
          
          pfadd (m0, m2)
          pfadd (m1, m3)

          jmp proceed_with_mapping
      //}
      //else
      //if (num_leftover > 1)
      //{

      last_subdivision:
          cmp dword ptr [num_leftover],1
          jle proceed_with_mapping
        
          //calculate the right_z for the end of the leftover span
          //ooz_right += (cur_grads.doozdx * num_leftover);
          //soz_right += (cur_grads.dsozdx * num_leftover);
          //toz_right += (cur_grads.dtozdx * num_leftover);
          
          movd mm2,dword ptr [num_leftover]
          movd mm3, dword ptr [cur_grads]tri_gradients.dsozdx
          
          pi2fd (m2, m2)
          movd mm4, dword ptr [cur_grads]tri_gradients.dtozdx

          pfmul (m3, m2)
          movd mm5, dword ptr [cur_grads]tri_gradients.doozdx
          
          pfmul (m4, m2)
          pfmul (m5, m2)

          pfacc (m3, m4) //gets dtozdx*num_leftover into high 32 bits of m3

          pfadd (m0, m3)
          pfadd (m1, m5)
      //}
            
      proceed_with_mapping:
        //cap the right_s and right_t's so that they're valid

        mov eax,dword ptr [right_s_t] //right_s
        mov ebx,dword ptr [right_s_t+4] //right_t
        
        add eax,dword ptr [cur_grads]tri_gradients.s_adjust
        add ebx,dword ptr [cur_grads]tri_gradients.t_adjust
 
        //cap the right s and t
        cmp eax,0
        jge cmp_eax_high

        mov eax,0
        jmp cmp_ebx_low

      cmp_eax_high:
        cmp eax,dword ptr [s_mask]
        jle cmp_ebx_low

        mov eax,dword ptr [s_mask]

      cmp_ebx_low:
        cmp ebx,0
        jge cmp_ebx_high

        mov ebx,0
        jmp done_compare
      
      cmp_ebx_high:
        cmp ebx,dword ptr [t_mask]
        jle done_compare

        mov ebx,dword ptr [t_mask]

      done_compare:

        //store the right_s and right_t
        //so they can be copied into left_s and left_t at the end of the 16-pixel span
        //(the cant be copied now because we have to calculate (right_s-left_s)>>4 and (right_t-left_t)>>4
        
        //calculate the next right_z in mm7
        //unfortunately, if the span is a multiple of 16, and this is the last set of 16, it will
        //calculate an unnecessary z. but its best to have the code here mixed in w/integer ops so
        //that the amd3d code has something for its executation latencies to sit through
        movq mm7, mm1
        pfrcp (m6, m1)

        mov dword ptr [right_s_t],eax //right_s
        mov dword ptr [right_s_t+4],ebx //right_t

        punpckldq mm7, mm7 //duplicate low 32bits of mm7 into high 32 bits of mm7
        sub eax,dword ptr [left_s_t] //left_s

        sar eax,4
        push ebp

        pfrcpit1 (m7, m6)
        sub ebx,dword ptr [left_s_t+4] //left_t

        sar ebx,4
        mov word ptr [dsdx_frac+2],ax //this sets the upper 16 bits of dword ptr [dsdx_frac] to ((right_s-left_s)>>4)<<16
        
        pfrcpit2 (m7, m6)
        nop
        
        sar eax,16
        mov word ptr [dtdx_frac+2],bx //this sets the upper 16 bits of dword ptr [dtdx_frac] to ((right_t-left_t)>>4)<<16
        
        sar ebx,16
        mov cl,byte ptr [r1_software_twidth_log2]
        
        shl ebx,cl

        add eax,ebx
        mov ebx,0 //clear high bits of ebx

        //s_t_carry[1] = integral_dsdx + integral_dtdx<<r1_software_twidth_log2
        //s_t_carry[0] = integral_dsdx + integral_dtdx<<r1_software_twidth_log2 + r1_software_texture_width

        mov dword ptr [s_t_carry+4],eax
        add eax,dword ptr [r1_software_texture_width]

        mov dword ptr [s_t_carry],eax
        mov cl,4 //loop is unrolled to 4 pixels - we want to draw 16, so loop 4 times

        mov bh,byte ptr [last_bh2] //setup the initial dither
        clc //clear the carry bit

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
        
        pop ebp
        mov byte ptr [last_bh2],bh //save it

        //store right_s and right_t in left_s and left_t
        //right_s is what left_s starts at on the next 16 pixel span
        //right_t is what left_t starts at on the next 16 pixel span

        mov eax,dword ptr [right_s_t] //right_s
        mov ebx,dword ptr [right_s_t+4] //right_t

        mov dword ptr [left_s_t],eax //left_s
        mov dword ptr [left_s_t+4],ebx //left_t
      }
          
      _asm dec dword ptr [num_subdivisions]
    }
    
    //store these so that the C code below actually works
    _asm mov word ptr [left_l],dx
    _asm mov dword ptr [start_pixel],edi
  }
    
  if (num_leftover)
  {        
    if (num_leftover > 1)
    {      
      if (had_subdivisions==0)
      {
        //calculate the right_z for the end of span
        //ooz_right = left->ooz + (cur_grads.doozdx * num_leftover);
        //soz_right = left->soz + (cur_grads.dsozdx * num_leftover);
        //toz_right = left->toz + (cur_grads.dtozdx * num_leftover);

        _asm
        {
          movd mm2,dword ptr [num_leftover]
          lea ebx,dword ptr [cur_grads]
          
          movd mm3, dword ptr [ebx]tri_gradients.dsozdx
          mov edi,dword ptr [left]

          movd mm4, dword ptr [ebx]tri_gradients.dtozdx
          pi2fd (m2, m2)
          
          movd mm5, dword ptr [ebx]tri_gradients.doozdx
          pfmul (m3, m2)
          
          movq mm0, qword ptr [edi]perspective_span.soz
          pfmul (m4, m2)

          movd mm1, dword ptr [edi]perspective_span.ooz
          pfmul (m5, m2)          
          
          pfacc (m3, m4) //gets dtozdx*num_leftover into high 32 bits of m3
          
          pfadd (m1, m5) //ooz += doozdx*num_leftover
          pfadd (m0, m3) //soz += dsozdx*num_leftover AND toz += dtozdx*num_leftover

          //calculate the z at the right endpoint in mm7
          movq mm7, mm1
          pfrcp (m6, m1)

          punpckldq mm7, mm7 //duplicate low 32bits of mm7 into high 32 bits of mm7

          pfrcpit1 (m7, m6) //terrible stalls. oh well
       
          pfrcpit2 (m7, m6)
        }
      }
      else
      {
        //the correct ending right_z is already being calculated
        //(see the if (num_subdivisions!=1) case above
      }

      _asm
      {
        //calculate starting fractional and integral values for s and t           
        
        //calculate the right endpoint
        //right_s = qftoi(soz_right * right_z) + cur_grads.s_adjust;
        //right_t = qftoi(toz_right * right_z) + cur_grads.t_adjust;
        
        //soz_right and toz_right are in mm0
        //right_z is in mm7
        pfmul (m7, m0) //calculate right_s and right_t
        mov edi,dword ptr [start_pixel]

        mov esi,dword ptr [r1_software_texture_ptr]
        mov eax,dword ptr [left_s_t] //left_s

        shr esi,1
        pf2id (m7, m7) //truncate right_s and right_t
        
        sar eax,16
        mov ebx,dword ptr [left_s_t+4] //left_t
            
        sar ebx,16
        movq qword ptr [right_s_t],mm7

        mov edx,dword ptr [left_s_t+4] //left_t
        add esi,eax
        
        mov cl,byte ptr [r1_software_twidth_log2]
        shl ebx,cl
      
        sal edx,16
        mov ecx,dword ptr [left_s_t] //left_s
      
        sal ecx,16
        add esi,ebx

        mov eax,dword ptr [right_s_t] //right_s
        mov ebx,dword ptr [right_s_t+4] //right_t
        
        add eax,dword ptr [cur_grads]tri_gradients.s_adjust
        add ebx,dword ptr [cur_grads]tri_gradients.t_adjust
 
        //cap the right s and t
        cmp eax,0
        jge cmp_eax_high_2

        mov eax,0
        jmp cmp_ebx_low_2

      cmp_eax_high_2:
        cmp eax,dword ptr [s_mask]
        jle cmp_ebx_low_2

        mov eax,dword ptr [s_mask]

      cmp_ebx_low_2:
        cmp ebx,0
        jge cmp_ebx_high_2

        mov ebx,0
        jmp done_compare_2
      
      cmp_ebx_high_2:
        cmp ebx,dword ptr [t_mask]
        jle done_compare_2

        mov ebx,dword ptr [t_mask]

      done_compare_2:
            
        //calculate the deltas (left to right)
        //temp_dsdx = qftoi((float)(right_s - left_s) * inverse_leftover_lookup[num_leftover]);
        //temp_dtdx = qftoi((float)(right_t - left_t) * inverse_leftover_lookup[num_leftover]);

        sub eax,dword ptr [left_s_t] //left_s
        sub ebx,dword ptr [left_s_t+4] //left_t

        movd mm0,eax //temp_dsdx
        push ebp
        
        movd mm1,ebx //temp_dtdx
        mov ebp, dword ptr [num_leftover]
        
        pi2fd (m0, m0)
        movd mm2, dword ptr [inverse_leftover_lookup + ebp*4]
        
        pi2fd (m1, m1)        
        pfmul (m0, m2)

        pfmul (m1, m2) //bad stalls here
        pf2id (m0, m0)

        pf2id (m1, m1)

        movd eax, mm0 //temp_dsdx
        movd ebx, mm1 //temp_dtdx

        //calculate the fractional and integral delta vars
        //s_t_carry[0] = (temp_dsdx>>16) + ((temp_dtdx>>16)<<r1_software_twidth_log2) + r1_software_texture_width;
        //s_t_carry[1] = (temp_dsdx>>16) + ((temp_dtdx>>16)<<r1_software_twidth_log2);
        //dsdx_frac    = (temp_dsdx<<16);
        //dtdx_frac    = (temp_dtdx<<16);

        mov word ptr [dsdx_frac+2],ax
        mov word ptr [dtdx_frac+2],bx

        sar eax,16
        mov dx,word ptr [left_l]

        sar ebx,16
        mov cl,byte ptr [r1_software_twidth_log2]
        
        shl ebx,cl

        add eax,ebx
        mov ebx,0 //clear high bits

        mov dword ptr [s_t_carry+4],eax
        add eax,dword ptr [r1_software_texture_width]
        
        mov dword ptr [s_t_carry],eax
        mov cl, byte ptr [num_leftover]
        
        mov ch,dl //setup the initial lighting error
        mov bh,byte ptr [last_bh2] //setup the initial dither

        clc //clear the carry bit

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

        pop ebp
      }
    }
    else
    {
      register w16 texel;
      register w32 l_lookup;

      //highly unoptimized single pixel drawer   //left_s         //left_t
      texel = *(r1_software_texture_ptr + (left_s_t[0]>>16) + ((left_s_t[1]>>16)<<r1_software_twidth_log2));
          
      l_lookup = left_l & (NUM_LIGHT_SHADES<<8);
  
      //                                        low bits                                  high bits
      *start_pixel = (w16)(((w32 *)(0xDEADBEEF))[l_lookup + (texel & 0xFF)] + ((w32 *)(0xDEADBEEF)+ctable_size)[l_lookup + (texel>>8)]);
    }
  }
  
  return;

  _asm
  {
  dumpmmxregs:
    movq qword ptr [mmx0],mm0
    movq qword ptr [mmx1],mm1
    movq qword ptr [mmx2],mm2
    movq qword ptr [mmx3],mm3
    movq qword ptr [mmx4],mm4
    movq qword ptr [mmx5],mm5
    movq qword ptr [mmx6],mm6
    movq qword ptr [mmx7],mm7
    ret
  }

}

w32 *texture_perspective_lit_sentinel_amd3d()
{
  bogus_label:
  
  w32 returnval;
  _asm
  {
    lea eax,bogus_label
    mov dword ptr [returnval],eax
  }
  return (w32 *)returnval;
}

void insert_color_modify_address_low(w32 *address);
void insert_color_modify_address_high(w32 *address);
extern w32 color_modify_list[];
extern sw32 num_color_modifies;

void setup_color_modify_perspective_lit_amd3d()
{
  w32 *stop = texture_perspective_lit_sentinel_amd3d();

  w32 *search = texture_perspective_lit_starter_amd3d();
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
