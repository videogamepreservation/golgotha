/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "software/r1_software_globals.hh"
#include "software/inline_fpu.hh"

extern sw32 had_subdivisions;

void texture_scanline_perspective_unlit(w16 *start_pixel,
                                        sw32 start_x,
                                        void *_left,//perspective_span *left,
                                        sw32 width)
{
  start_pixel = (w16 *)((w8 *)start_pixel + start_x);

  perspective_span *left = (perspective_span *)_left;

  _asm
  {
    //left_z = 1.f / left->ooz;
    //left_s = qftoi(left->soz * left_z) + cur_grads.s_adjust;
    //left_t = qftoi(left->toz * left_z) + cur_grads.t_adjust;
    
    //sw32 had_subdivisions = width & (~15);
    //num_subdivisions = width >> 4;
    //num_leftover     = width & 15;
    
    mov esi,dword ptr [left]
    mov eax,dword ptr [width]

    fld1
    fdiv qword ptr [esi]perspective_span.ooz

    mov ebx,eax
    and eax,15

    shr ebx,4
    mov ecx,width

    and ecx,(~15)
    mov dword ptr [num_leftover],eax
    
    mov dword ptr [num_subdivisions],ebx
    mov dword ptr [had_subdivisions],ecx
    
    fld st(0)
        
    fmul dword ptr [esi]perspective_span.soz
    fxch st(1)
        
    fmul dword ptr [esi]perspective_span.toz
    fxch st(1)

    fistp dword ptr [left_s]
    fistp dword ptr [left_t]

    mov eax,dword ptr [cur_grads].s_adjust
    mov ebx,dword ptr [cur_grads].t_adjust

    add eax,dword ptr [left_s]
    add ebx,dword ptr [left_t]

    mov dword ptr [left_s],eax
    mov dword ptr [left_t],ebx
    
    //clear these out
    mov dword ptr [dsdx_frac],0
    mov dword ptr [dtdx_frac],0
  }

  if (num_subdivisions)
  {
    _asm
    {
      //ooz_right = left->ooz + (cur_grads.doozdxspan);
      //soz_right = left->soz + (cur_grads.dsozdxspan);
      //toz_right = left->toz + (cur_grads.dtozdxspan);

      mov esi,dword ptr [left]
      mov edi,dword ptr [start_pixel]

      fld qword ptr [esi]perspective_span.ooz
      fld dword ptr [esi]perspective_span.soz
      fld dword ptr [esi]perspective_span.toz

      //t s o
      fadd dword ptr [cur_grads]tri_gradients.dtozdxspan
      fxch st(2)

      //o s t

      fadd qword ptr [cur_grads]tri_gradients.doozdxspan
      fxch st(1)

      //s o t

      fadd dword ptr [cur_grads]tri_gradients.dsozdxspan
      fxch st(2)

      //t o s

      fstp dword ptr [toz_right]
      fxch st(1)

      //s o

      fstp dword ptr [soz_right]

      fstp dword ptr [ooz_right]
      
      //calculate the 1st right_z
      fld1
      fdiv dword ptr [ooz_right]

      //calculate starting fractional and integral values for s and t
      //esi = starting_s_coordinate >> 16 + starting_t_coordinate >> 16 << r1_software_twidth_log2
      //ecx = starting_s_coordinate << 16
      //edx = starting_t_coordinate << 16
      //dx  = starting_light_value

      mov esi,dword ptr [r1_software_texture_ptr]
      mov eax,dword ptr [left_s]

      shr esi,1
      mov ebx,dword ptr [left_t]
    
      sar eax,16
      mov edx,dword ptr [left_t]

      sar ebx,16
      add esi,eax

      mov cl,byte ptr [r1_software_twidth_log2]
      shl ebx,cl
      
      sal edx,16
      mov ecx,dword ptr [left_s]
    
      sal ecx,16
      add esi,ebx
    }

    while (num_subdivisions)
    {
      _asm
      {
        //right_s = qftoi(soz_right * right_z);
        //right_t = qftoi(toz_right * right_z);
        
        //right_z is in st0
        fld st(0)
        
        fmul dword ptr [soz_right]
        fxch st(1)
        
        fmul dword ptr [toz_right]
        fxch st(1)

        fistp dword ptr [right_s]
        fistp dword ptr [right_t]

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
          
          fld dword ptr [ooz_right]
          fadd qword ptr [cur_grads]tri_gradients.doozdxspan

          fld dword ptr [soz_right]
          fadd dword ptr [cur_grads]tri_gradients.dsozdxspan
                                   
          fld dword ptr [toz_right]
          fadd dword ptr [cur_grads]tri_gradients.dtozdxspan

          fxch st(2)
          fstp dword ptr [ooz_right]

          fstp dword ptr [soz_right]

          fstp dword ptr [toz_right]

          fld1
          fdiv dword ptr [ooz_right]

          jmp not_last_subdivision
      //}
      //else
      //if (num_leftover > 1)
      //{

      last_subdivision:
          cmp dword ptr [num_leftover],1
          jle not_last_subdivision
        
          //calculate the right_z for the end of the leftover span
          //ooz_right += (cur_grads.doozdx * num_leftover);
          //soz_right += (cur_grads.dsozdx * num_leftover);
          //toz_right += (cur_grads.dtozdx * num_leftover);

          fild dword ptr [num_leftover]
          
          //todo: pipeline these fpu ops
          fld  qword ptr [cur_grads]tri_gradients.doozdx
          fmul st(0),st(1)
          fadd dword ptr [ooz_right]
          fstp dword ptr [ooz_right]

          fld  dword ptr [cur_grads]tri_gradients.dsozdx
          fmul st(0),st(1)
          fadd dword ptr [soz_right]
          fstp dword ptr [soz_right]

          fld  dword ptr [cur_grads]tri_gradients.dtozdx
          fmul st(0),st(1)
          fadd dword ptr [toz_right]
          fstp dword ptr [toz_right]

          fstp st(0) //nifty thing i found, a 1 cycle fpu pop
        
          fld1
          fdiv dword ptr [ooz_right]
      //}
            
      not_last_subdivision:
        //cap the right_s and right_t's so that they're valid

        mov eax,dword ptr [right_s]
        mov ebx,dword ptr [right_t]
        
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
        
        mov dword ptr [right_s],eax
        mov dword ptr [right_t],ebx

        sub eax,dword ptr [left_s]
        push ebp

        sar eax,4
        sub ebx,dword ptr [left_t]
        
        sar ebx,4
        mov word ptr [dsdx_frac+2],ax //this sets the upper 16 bits of dword ptr [dsdx_frac] to ((right_s-left_s)>>4)<<16
        
        sar eax,16
        mov word ptr [dtdx_frac+2],bx //this sets the upper 16 bits of dword ptr [dtdx_frac] to ((right_t-left_t)>>4)<<16
        
        sar ebx,16
        mov cl,byte ptr [r1_software_twidth_log2]
        
        shl ebx,cl

        add eax,ebx
        mov ebx,0

        //s_t_carry[1] = integral_dsdx + integral_dtdx<<r1_software_twidth_log2
        //s_t_carry[0] = integral_dsdx + integral_dtdx<<r1_software_twidth_log2 + r1_software_texture_width

        mov dword ptr [s_t_carry+4],eax
        add eax,dword ptr [r1_software_texture_width]
                
        mov dword ptr [s_t_carry],eax        
        mov eax,0 //must make sure the high bits of these are zeroed out

        mov cl,4 //loop is unrolled to 4 pixels - we want to draw 16, so loop 4 times
        ALIGN 16

        //high 16 bits of ecx is the fractional s component
        //high 16 bits of edx is the fractional t component

        //eax is used to lookup the texel as well as the low 8-bits of the lit texel
        //ebx is used to lookup the high 8-bits of the lit texel
        //ebp is used to detect a t-carry as well as lookup the lit texel
        //cl  is the loop count variable

    looper1:
        add edx,dword ptr [dtdx_frac]
        nop
 
        sbb ebp,ebp
        add edi,8 //the only convenient place for the stepping of edi was way up here

        mov ax,word ptr [esi*2]
        add ecx,dword ptr [dsdx_frac]

        adc esi,dword ptr [4+s_t_carry+ebp*4]
        add edx,dword ptr [dtdx_frac]
        
        sbb ebp,ebp
        mov word ptr [edi-8],ax //1

        mov ax,word ptr [esi*2]
        add ecx,dword ptr [dsdx_frac]

        adc esi,dword ptr [4+s_t_carry+ebp*4]
        add edx,dword ptr [dtdx_frac]
        
        sbb ebp,ebp
        mov word ptr [edi-6],ax //2

        mov ax,word ptr [esi*2]
        add ecx,dword ptr [dsdx_frac]

        adc esi,dword ptr [4+s_t_carry+ebp*4]
        add edx,dword ptr [dtdx_frac]
        
        sbb ebp,ebp
        mov word ptr [edi-4],ax //3

        mov ax,word ptr [esi*2]
        add ecx,dword ptr [dsdx_frac]

        adc esi,dword ptr [4+s_t_carry+ebp*4]
        mov word ptr [edi-2],ax //4
        
        dec cl //thank god this doesnt modify the carry flag (the above add ch,dl needs to effect the adc bh,dh at the top of the loop)
      
        jnz looper1
        pop ebp

        //store right_s and right_s in left_s and left_t
        //right_s is what left_s starts at on the next 16 pixel span
        //right_t is what left_t starts at on the next 16 pixel span

        mov eax,dword ptr [right_s]
        mov ebx,dword ptr [right_t]

        mov dword ptr [left_s],eax
        mov dword ptr [left_t],ebx
      }
          
      _asm dec dword ptr [num_subdivisions]
    }
    
    //store these so that the C code below actually works
    _asm mov dword ptr [start_pixel],edi
  }
    
  if (num_leftover)
  {        
    if (num_leftover > 1)
    {      
      if (had_subdivisions==0)
      {
        //calculate the right_z for the end of span
        ooz_right = left->ooz + (cur_grads.doozdx * num_leftover);
        soz_right = left->soz + (cur_grads.dsozdx * num_leftover);
        toz_right = left->toz + (cur_grads.dtozdx * num_leftover);
        
        //calculate the z at the right endpoint
        _asm fld1
        _asm fdiv dword ptr [ooz_right]
      }
      else
      {
        //the correct ending right_z is already being calculated
        //(see the if (num_subdivisions!=1) case above
      }

      _asm
      {
        //calculate starting fractional and integral values for s and t           
        
        mov esi,dword ptr [r1_software_texture_ptr]
        mov eax,dword ptr [left_s]

        shr esi,1
        mov ebx,dword ptr [left_t]
    
        sar eax,16
        mov edx,dword ptr [left_t]

        sar ebx,16
        add esi,eax

        mov cl,byte ptr [r1_software_twidth_log2]
        shl ebx,cl
      
        sal edx,16
        mov ecx,dword ptr [left_s]
      
        sal ecx,16
        add esi,ebx
        
        mov edi,dword ptr [start_pixel]

        //calculate the right endpoint
        //right_s = qftoi(soz_right * right_z) + cur_grads.s_adjust;
        //right_t = qftoi(toz_right * right_z) + cur_grads.t_adjust;
        
        //right_z is in st0
        fld st(0)
        
        fmul dword ptr [soz_right]
        fxch st(1)
        
        fmul dword ptr [toz_right]
        fxch st(1)

        fistp dword ptr [right_s]
        fistp dword ptr [right_t]

        mov eax,dword ptr [right_s]
        mov ebx,dword ptr [right_t]
        
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
        
        push ebp
        mov ebp,num_leftover

        sub eax,dword ptr [left_s]
        sub ebx,dword ptr [left_t]

        mov dword ptr [temp_dsdx],eax
        mov dword ptr [temp_dtdx],ebx

        fild dword ptr [temp_dsdx]
        fild dword ptr [temp_dtdx]

        fmul dword ptr [inverse_leftover_lookup + ebp*4]
        fxch st(1)

        fmul dword ptr [inverse_leftover_lookup + ebp*4]
        fxch st(1)

        fistp dword ptr [temp_dtdx]
        fistp dword ptr [temp_dsdx]
        
        //calculate the fractional and integral delta vars
        //s_t_carry[0] = (temp_dsdx>>16) + ((temp_dtdx>>16)<<r1_software_twidth_log2) + r1_software_texture_width;
        //s_t_carry[1] = (temp_dsdx>>16) + ((temp_dtdx>>16)<<r1_software_twidth_log2);
        //dsdx_frac    = (temp_dsdx<<16);
        //dtdx_frac    = (temp_dtdx<<16);

        mov eax,dword ptr [temp_dsdx]
        mov ebx,dword ptr [temp_dtdx]
        
        mov word ptr [dsdx_frac+2],ax
        mov word ptr [dtdx_frac+2],bx

        sar eax,16
        nop //mov dx,word ptr [left_l]

        sar ebx,16
        mov cl,byte ptr [r1_software_twidth_log2]
        
        shl ebx,cl

        add eax,ebx
        nop //mov ebx,0

        mov dword ptr [s_t_carry+4],eax
        add eax,dword ptr [r1_software_texture_width]
        
        mov dword ptr [s_t_carry],eax
        mov cl, byte ptr [num_leftover]
        
        //mov eax,0
        //mov ch,dl //setup the initial lighting error 

        //mov bh,byte ptr [last_bh2] //setup the initial dither
        //add ch,0 //clear the carry bit

        ALIGN 16

      looper3:
        mov ax,word ptr [esi*2]
        add edx,dword ptr [dtdx_frac]
        
        sbb ebp,ebp
        mov word ptr [edi],ax //1

        add edi,2 //the only convenient place for the stepping of edi was way up here
        add ecx,dword ptr [dsdx_frac]

        adc esi,dword ptr [4+s_t_carry+ebp*4]
        dec cl

        jnz looper3
        pop ebp
      }
    }
    else
    {
      //highly unoptimized single pixel drawer
      *start_pixel = *(r1_software_texture_ptr + (left_s>>16) + ((left_t>>16)<<r1_software_twidth_log2));
    }
  }
}
