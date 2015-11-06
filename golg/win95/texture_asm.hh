/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef _WIN95_ASM_HH_
#define _WIN95_ASM_HH_

//June 7th... Jonathon finally talked me into inline...
//globals...  These are referenced as memory offsets
//so they need to be global 'c' type things
//when these are totally finalized, re-arrange these in
//tmapper soft so that the variables used together are in
//the same cache line

//external references
extern "C" w32 cTable[2*256*32];   //low and high bits of color
extern "C" w16 *r1_software_texture_ptr;
extern "C" w8  r1_software_twidth_log2;
extern "C" w16 *pixel_on;

//external declarations
extern "C"
{
  i4_float FixedScale  = 65536.f;
  i4_float FixedScale8 = 8192.f;
  i4_float LightScale  = 2031616.f;
  w16 zBuffer[640*480];   //low and high bits of color  
  i4_float One = 1.f;
  i4_float FloatTemp;
  w32 UVintVfracStepV[2];
  w32 DeltaVFrac;
  w32 DeltaUFrac;
  w32 LVal;
  w32 LDelta;
  w32 pTex;
}


void draw_scanline_white_pc(edgeGrad *left, edgeGrad *right)
{
  //locals on the stack
  w32 Subdivisions;
  w32 WidthModLength;
  w32 DeltaU;
  w32 DeltaV;
  w32 UFixed;
  w32 VFixed;
  w16 FPUCW;
  w16 OldFPUCW;
  sw32 blarg;

  if (((sw32)r1_software_texture_ptr) & 1)
  {
    _asm
    {
      mov Subdivisions,0
    }    
    return;
  }

  _asm {
    //this should be moved out
    ; put the FPU in 32 bit mode
    fstcw   [OldFPUCW]                    ; store copy of CW
    mov     ax,OldFPUCW                   ; get it in ax
    and     eax,0ffh                      ; 24 bit precision
    mov     [FPUCW],ax                    ; store it
    fldcw   [FPUCW]                       ; load the FPU

    mov     ebx,left                      ; 
    mov     esi,r1_software_pixel_on      ; grab screen pointer
    mov     edx,right                     ;
    mov     eax,[ebx]edgeGrad.X           ; left x
    mov     ecx,[edx]edgeGrad.X           ; edx = right x

    sub     ecx,eax                       ; edx = width
    jle     whtp_Return                   ; no pixels to draw, get out

    mov     [pTex],ecx                    ; just for a temp so it can be fild'ed

//note below the fdiv... if more values need to be interpolated based on the scanline
//width... you should do 1/WID and use muls... this is faster cuz theres only one

    ; calc this scanlines light step      ; FPU Stack
                                          ; st0  st1  st2  st3  st4  st5  st6  st7
    fild    dword ptr [pTex]              ; WID
    fld     dword ptr [edx]edgeGrad.r     ; LR   WID
    fld     dword ptr [ebx]edgeGrad.r     ; LL   LR   WID
    fld     st                            ; LL   LL   LR   WID
    fmul    [LightScale]                  ; LL16 LL   LR   WID
    fistp   [LVal]                        ; LL   LR   WID
    fsubp   st(1),st                      ; LD   WID
    fdiv    st,st(1)                      ; LS   WID
    mov     ebx,eax

//heres an unused scanline z fill (inaccurate)
;    mov     ax,word ptr [sortKey]
;    mov     edi,zBuffer

    mov     edx,ecx
    mov     edi,r1_software_texture_ptr

//rest of the z fill thing that isn't used
;    rep     stosw 

    mov     cl,r1_software_twidth_log2
    mov     byte ptr whtp_twidth_log2_patch0+2, cl
    shr     edi,1                         ; pray that it's aligned!
    mov     byte ptr whtp_twidth_log2_patch1+2, cl
    mov     pTex,edi
    mov     byte ptr whtp_twidth_log2_patch2+2, cl
    mov     edi,esi                       ; edi = dest pointer
    mov     byte ptr whtp_twidth_log2_patch3+2, cl

//is pixel_on.add being used?  If not the shl is needed
;    shl     ebx,1

    mov     eax,edx                       ; eax and edx = width

//this is also needed if pixel_on.add isn't used
;    add     edi,ebx

    ; edi = pointer to start pixel in dest dib
    ; edx = spanwidth

    shr     edx,3                         ; ecx = width / subdivision length
    and     eax,7                         ; eax = width mod subdivision length

    //the next line is avoiding a far jump which was the
    //only opcode inline would generate... it simply skips down to
    //to mov [Subdivisions],edx two instructions down
    __asm _emit 0x75 __asm _emit 0x06     ;jnz     @f  any leftover?

    dec     edx                           ; no, so special case last span
    mov     eax,8                         ; it's 8 pixels long
    mov     [Subdivisions],edx            ; store widths
    mov     [WidthModLength],eax

    fmul    [LightScale]                  
    fxch    st(1)
    fistp   [LDelta]
    fistp   [LDelta]


    mov     ebx,left                      ; get left edge pointer

    //try not to play around with the ordering here later...
    //this is the most optimal stack ordering possible
    ; calculate ULeft and VLeft           ; FPU Stack (ZL = ZLeft)
                                          ; st0  st1  st2  st3  st4  st5  st6  st7
    fld     [ebx]edgeGrad.VOverZ          ; V/ZL 
    fld     [ebx]edgeGrad.UOverZ          ; U/ZL V/ZL 
    fld     [ebx]edgeGrad.OneOverZ        ; 1/ZL U/ZL V/ZL 
    fld1                                  ; 1    1/ZL U/ZL V/ZL 
    fdiv    st,st(1)                      ; ZL   1/ZL U/ZL V/ZL 

    //here there is room for alot of integer ops, but this is only done
    //once... This would be a good spot for the zbuffer shitcan method

    fld     st                            ; ZL   ZL   1/ZL U/ZL V/ZL 
    fmul    st,st(4)                      ; VL   ZL   1/ZL U/ZL V/ZL 
    fxch    st(1)                         ; ZL   VL   1/ZL U/ZL V/ZL 
    fmul    st,st(3)                      ; UL   VL   1/ZL U/ZL V/ZL 
    fstp    st(5)                         ; VL   1/ZL U/ZL V/ZL UL
    fstp    st(5)                         ; 1/ZL U/ZL V/ZL UL   VL

    //the dword ptrs below shut up compiler warnings (though they don't do qwords)
    //notice the adds... that simply steps to the end of the next span ahead of time
    ; calculate right side OverZ terms    ; st0  st1  st2  st3  st4  st5  st6  st7
    fadd    dword ptr grads.dOneOverZdX8  ; 1/ZR U/ZL V/ZL UL   VL
    fxch    st(1)                         ; U/ZL 1/ZR V/ZL UL   VL
    fadd    dword ptr grads.dUOverZdX8    ; U/ZR 1/ZR V/ZL UL   VL
    fxch    st(2)                         ; V/ZL 1/ZR U/ZR UL   VL
    fadd    dword ptr grads.dVOverZdX8    ; V/ZR 1/ZR U/ZR UL   VL
  

    //here's the equivlent of the fpu loop at the bottom, only theres an fdiv
    //here that has plenty of room for int ops between
    ; calculate right side coords       ; st0  st1  st2  st3  st4  st5  st6  st7

    fld1                                ; 1    V/ZR 1/ZR U/ZR UL   VL
    ; @todo overlap this guy
    fdiv    st,st(2)                    ; ZR   V/ZR 1/ZR U/ZR UL   VL
    fld     st                          ; ZR   ZR   V/ZR 1/ZR U/ZR UL   VL
    fmul    st,st(2)                    ; VR   ZR   V/ZR 1/ZR U/ZR UL   VL
    fxch    st(1)                       ; ZR   VR   V/ZR 1/ZR U/ZR UL   VL
    fmul    st,st(4)                    ; UR   VR   V/ZR 1/ZR U/ZR UL   VL

    test    edx,edx                     ; check for any full spans
    jz      whtp_HandleLeftoverPixels

whtp_SpanLoop:

    ; at this point the FPU contains      ; st0  st1  st2  st3  st4  st5  st6  st7
                                          ; UR   VR   V/ZR 1/ZR U/ZR UL   VL

    ; convert left side coords

    fld     st(5)                         ; UL   UR   VR   V/ZR 1/ZR U/ZR UL   VL
    fmul    [FixedScale]                  ; UL8 UR   VR   V/ZR 1/ZR U/ZR UL   VL
    fistp   [UFixed]                      ; UR   VR   V/ZR 1/ZR U/ZR UL   VL

    fld     st(6)                         ; VL   UR   VR   V/ZR 1/ZR U/ZR UL   VL
    fmul    [FixedScale]                  ; VL8 UR   VR   V/ZR 1/ZR U/ZR UL   VL
    fistp   [VFixed]                      ; UR   VR   V/ZR 1/ZR U/ZR UL   VL

    ; calculate deltas                    ; st0  st1  st2  st3  st4  st5  st6  st7

    fsubr   st(5),st                      ; UR   VR   V/ZR 1/ZR U/ZR dU   VL
    fxch    st(1)                         ; VR   UR   V/ZR 1/ZR U/ZR dU   VL
    fsubr   st(6),st                      ; VR   UR   V/ZR 1/ZR U/ZR dU   dV
    fxch    st(6)                         ; dV   UR   V/ZR 1/ZR U/ZR dU   VR

    fmul    [FixedScale8]                 ; dV8  UR   V/ZR 1/ZR U/ZR dU   VR
    fistp   [DeltaV]                      ; UR   V/ZR 1/ZR U/ZR dU   VR

    fxch    st(4)                         ; dU   V/ZR 1/ZR U/ZR UR   VR
    fmul    [FixedScale8]                 ; dU8  V/ZR 1/ZR U/ZR UR   VR
    fistp   [DeltaU]                      ; V/ZR 1/ZR U/ZR UR   VR

    ; increment terms for next span       ; st0  st1  st2  st3  st4  st5  st6  st7
    ; Right terms become Left terms---->  ; V/ZL 1/ZL U/ZL UL   VL

    fadd    dword ptr grads.dVOverZdX8              ; V/ZR 1/ZL U/ZL UL   VL
    fxch    st(1)                         ; 1/ZL V/ZR U/ZL UL   VL
    fadd    dword ptr grads.dOneOverZdX8            ; 1/ZR V/ZR U/ZL UL   VL
    fxch    st(2)                         ; U/ZL V/ZR 1/ZR UL   VL
    fadd    dword ptr grads.dUOverZdX8              ; U/ZR V/ZR 1/ZR UL   VL
    fxch    st(2)                         ; 1/ZR V/ZR U/ZR UL   VL
    fxch    st(1)                         ; V/ZR 1/ZR U/ZR UL   VL

    //here's the span fdiv... avoid any fpu ops after this, or any imuls
    ; calculate right side coords         ; st0  st1  st2  st3  st4  st5  st6  st7
    fld1                                  ; 1    V/ZR 1/ZR U/ZR UL   VL
    fdiv    st,st(2)                      ; ZR   V/ZR 1/ZR U/ZR UL   VL

    //see hecker's articles for an explaination of below... it's too big
    //to comment in
    ; do pre span delta setup and start modify

    mov     esi,[DeltaV]                  ; get v 16.16 step
    mov     ebx,esi                       ; split up into int and frac
    sar     esi,16
    shl     ebx,16
    mov     [DeltaVFrac],ebx
whtp_twidth_log2_patch0:
    shl     esi,3

    mov     ebx,[DeltaU]                  ; get u 16.16 step
    mov     eax,ebx
    shl     eax,16
    sar     ebx,16
    mov     [DeltaUFrac],eax
    add     esi,ebx
    mov     ebx,1
    mov     [4+UVintVfracStepV],esi       ; non V carry slot
whtp_twidth_log2_patch1:
    shl     ebx,3                         ; this is to get the texwidth
    add     esi,ebx
    mov     [UVintVfracStepV],esi         ; V carry slot

    ; setup initial coordinates

    //see the file gradients.txt in hecker's code pack for an explaination
    //of the modifiers... it's pretty long
    mov     esi,[UFixed]                  ; get u 16.16 fixedpoint coordinate
    add     esi,dword ptr grads.dUdXModifier
    mov     ecx,[VFixed]                  ; and v... before ebp gets pushed
    add     ecx,dword ptr grads.dVdXModifier
    
    mov     blarg,ebp

    push    ebp
    mov     ebp,esi
    sar     esi,16
    mov     edx,ecx
    shl     ebp,16

    sar     edx,16
    shl     ecx,16
whtp_twidth_log2_patch2:
    shl     edx,3

    add     esi,edx                       ; move texture to offset
    xor     eax,eax                       ; clear eax
    mov     edx,[LVal]
    add     esi,pTex

    ; edi = dest dib bits at current pixel
    ; esi = texture pointer at current texel
    ; ebp = u fraction 0.32
    ; ecx = v fraction 0.32
    ; edx = Light value and bit storage
    ; ebx = v carry scratch
    ; 
    ; edx rolls to act as temp storage, and also to
    ; make the light value an index into the ctable

    //theres a stall in this loop... it's only one cycle but it might be
    //somehow avoided... I ran outta time.  It's the V pipe add below rol edx, 16
    mov   al,[1+esi*2]

    rol   edx,16                          ; get the low word usable
    add   ecx,[DeltaVFrac]

    sbb   ebx,ebx                         ; U pipe only
    mov   ah,dl

    mov   dh,[2*esi]
    add   ebp,[DeltaUFrac]

    adc   esi,[4+UVintVfracStepV+ebx*4]   ; U pipe only
    mov   bl,dh

    mov   bh,dl
    mov   eax,dword ptr[cTable+8000h+eax*4]

    and   ebx,0ffffh
    mov   dh,0

    rol   edx,16
    add   eax,dword ptr[cTable+ebx*4]     ; One cycle stall here U exp flow i think 

    add   edx,[LDelta]
    mov   [edi],al

    rol   edx,16
    mov   [edi+1],ah

    add   ecx,[DeltaVFrac]
    mov   ah,dl

    sbb   ebx,ebx                         ; U pipe only
    mov   dh,[esi*2]

    add   ebp,[DeltaUFrac]
    mov   al,[1+esi*2]

    adc   esi,[4+UVintVfracStepV+ebx*4]      ; U pipe only
    mov   bl,dh

    mov   eax,dword ptr[cTable+8000h+eax*4]
    mov   bh,dl

    mov   dh,0
    and   ebx,0ffffh

    rol   edx,16
    add   eax,dword ptr[cTable+ebx*4]

    add   edx,[LDelta]
    mov   [edi+2],al

    rol   edx,16
    mov   [edi+3],ah

    add   ecx,[DeltaVFrac]
    mov   ah,dl

    sbb   ebx,ebx                         ; U pipe only
    mov   dh,[esi*2]

    add   ebp,[DeltaUFrac]
    mov   al,[1+esi*2]

    adc   esi,[4+UVintVfracStepV+ebx*4]      ; U pipe only
    mov   bl,dh

    mov   eax,dword ptr[cTable+8000h+eax*4]
    mov   bh,dl

    mov   dh,0
    and   ebx,0ffffh

    rol   edx,16
    add   eax,dword ptr[cTable+ebx*4]

    add   edx,[LDelta]
    mov   [edi+4],al

    rol   edx,16
    mov   [edi+5],ah

    add   ecx,[DeltaVFrac]
    mov   ah,dl

    sbb   ebx,ebx                         ; U pipe only
    mov   dh,[esi*2]

    add   ebp,[DeltaUFrac]
    mov   al,[1+esi*2]

    adc   esi,[4+UVintVfracStepV+ebx*4]      ; U pipe only
    mov   bl,dh

    mov   eax,dword ptr[cTable+8000h+eax*4]
    mov   bh,dl

    mov   dh,0
    and   ebx,0ffffh

    rol   edx,16
    add   eax,dword ptr[cTable+ebx*4]

    add   edx,[LDelta]
    mov   [edi+6],al

    rol   edx,16
    mov   [edi+7],ah

    add   ecx,[DeltaVFrac]
    mov   ah,dl

    sbb   ebx,ebx                         ; U pipe only
    mov   dh,[esi*2]

    add   ebp,[DeltaUFrac]
    mov   al,[1+esi*2]

    adc   esi,[4+UVintVfracStepV+ebx*4]      ; U pipe only
    mov   bl,dh

    mov   eax,dword ptr[cTable+8000h+eax*4]
    mov   bh,dl

    mov   dh,0
    and   ebx,0ffffh

    rol   edx,16
    add   eax,dword ptr[cTable+ebx*4]

    add   edx,[LDelta]
    mov   [edi+8],al

    rol   edx,16
    mov   [edi+9],ah

    add   ecx,[DeltaVFrac]
    mov   ah,dl

    sbb   ebx,ebx                         ; U pipe only
    mov   dh,[esi*2]

    add   ebp,[DeltaUFrac]
    mov   al,[1+esi*2]

    adc   esi,[4+UVintVfracStepV+ebx*4]      ; U pipe only
    mov   bl,dh

    mov   eax,dword ptr[cTable+8000h+eax*4]
    mov   bh,dl

    mov   dh,0
    and   ebx,0ffffh

    rol   edx,16
    add   eax,dword ptr[cTable+ebx*4]

    add   edx,[LDelta]
    mov   [edi+10],al

    rol   edx,16
    mov   [edi+11],ah

    add   ecx,[DeltaVFrac]
    mov   ah,dl

    sbb   ebx,ebx                         ; U pipe only
    mov   dh,[esi*2]

    add   ebp,[DeltaUFrac]
    mov   al,[1+esi*2]

    adc   esi,[4+UVintVfracStepV+ebx*4]      ; U pipe only
    mov   bl,dh

    mov   eax,dword ptr[cTable+8000h+eax*4]
    mov   bh,dl

    mov   dh,0
    and   ebx,0ffffh

    rol   edx,16
    add   eax,dword ptr[cTable+ebx*4]

    add   edx,[LDelta]
    mov   [edi+12],al

    rol   edx,16
    mov   [edi+13],ah

    add   ecx,[DeltaVFrac]
    mov   ah,dl

    sbb   ebx,ebx                         ; U pipe only
    mov   dh,[esi*2]

    add   ebp,[DeltaUFrac]
    mov   al,[1+esi*2]

    adc   esi,[4+UVintVfracStepV+ebx*4]      ; U pipe only
    mov   bl,dh

    mov   eax,dword ptr[cTable+8000h+eax*4]
    mov   bh,dl

    mov   dh,0
    and   ebx,0ffffh

    rol   edx,16
    add   eax,dword ptr[cTable+ebx*4]

    add   edx,[LDelta]
    mov   [edi+14],al

    mov   [LVal],edx
    mov   [edi+15],ah

    pop   ebp

    ; ************** Okay to Access Stack Frame ****************
    ; ************** Okay to Access Stack Frame ****************
    ; ************** Okay to Access Stack Frame ****************


    ; the fdiv is done, finish right    ; st0  st1  st2  st3  st4  st5  st6  st7
                                        ; ZR   V/ZR 1/ZR U/ZR UL   VL

    fld     st                          ; ZR   ZR   V/ZR 1/ZR U/ZR UL   VL
    fmul    st,st(2)                    ; VR   ZR   V/ZR 1/ZR U/ZR UL   VL
    fxch    st(1)                       ; ZR   VR   V/ZR 1/ZR U/ZR UL   VL
    fmul    st,st(4)                    ; UR   VR   V/ZR 1/ZR U/ZR UL   VL

    add     edi,16                      ; increment to next span
    dec     [Subdivisions]              ; decrement span count
    jnz     whtp_SpanLoop                    ; loop back

whtp_HandleLeftoverPixels:

    mov     esi,pTex

    ; edi = dest dib bits
    ; esi = texture dib bits
    ; at this point the FPU contains    ; st0  st1  st2  st3  st4  st5  st6  st7
    ; inv. means invalid numbers        ; inv. inv. inv. inv. inv. UL   VL

    cmp     [WidthModLength],0          ; are there remaining pixels to draw?
    jz      whtp_FPUReturn              ; nope, pop the FPU and bail

    mov     ebx,right                   ; get right edge pointer

    ; convert left side coords          ; st0  st1  st2  st3  st4  st5  st6  st7

    fld     st(5)                       ; UL   inv. inv. inv. inv. inv. UL   VL
    fmul    [FixedScale]                ; UL8 inv. inv. inv. inv. inv. UL   VL
    fistp   [UFixed]                    ; inv. inv. inv. inv. inv. UL   VL

    fld     st(6)                       ; VL   inv. inv. inv. inv. inv. UL   VL
    fmul    [FixedScale]                ; VL8 inv. inv. inv. inv. inv. UL   VL
    fistp   [VFixed]                    ; inv. inv. inv. inv. inv. UL   VL

    cmp     [WidthModLength],1          ; calc how many steps to take
    jz      whtp_OnePixelSpan           ; just one, don't do deltas

    ; calculate right edge coordinates  ; st0  st1  st2  st3  st4  st5  st6  st7
    ; r -> R+1

    ; @todo rearrange things so we don't need these two instructions
    fstp    [FloatTemp]                 ; inv. inv. inv. inv. UL   VL
    fstp    [FloatTemp]                 ; inv. inv. inv. UL   VL

    //notice the fsubs... hecker doesn't explain this at all (damn him)
    //These are the compensation for the dudxmodifiers, the fill convention
    //is left biased so this makes the edge perfect

    fld     [ebx]edgeGrad.VOverZ        ; V/Zr inv. inv. inv. UL   VL
    fsub    dword ptr grads.dVOverZdX   ; V/ZR inv. inv. inv. UL   VL
    fld     [ebx]edgeGrad.UOverZ        ; U/Zr V/ZR inv. inv. inv. UL   VL
    fsub    dword ptr grads.dUOverZdX   ; U/ZR V/ZR inv. inv. inv. UL   VL
    fld     [ebx]edgeGrad.OneOverZ      ; 1/Zr U/ZR V/ZR inv. inv. inv. UL   VL
    fsub    dword ptr grads.dOneOverZdX ; 1/ZR U/ZR V/ZR inv. inv. inv. UL   VL

    fdivr   [One]                       ; ZR   U/ZR V/ZR inv. inv. inv. UL   VL

    fmul    st(1),st                    ; ZR   UR   V/ZR inv. inv. inv. UL   VL
    fmulp   st(2),st                    ; UR   VR   inv. inv. inv. UL   VL

    ; calculate deltas                  ; st0  st1  st2  st3  st4  st5  st6  st7

    fsubr   st(5),st                    ; UR   VR   inv. inv. inv. dU   VL
    fxch    st(1)                       ; VR   UR   inv. inv. inv. dU   VL
    fsubr   st(6),st                    ; VR   UR   inv. inv. inv. dU   dV
    fxch    st(6)                       ; dV   UR   inv. inv. inv. dU   VR

    fidiv   [WidthModLength]            ; dv   UR   inv. inv. inv. dU   VR
    fmul    [FixedScale]                ; dv8 UR   inv. inv. inv. dU   VR
    fistp   [DeltaV]                    ; UR   inv. inv. inv. dU   VR

    fxch    st(4)                       ; dU   inv. inv. inv. UR   VR
    fidiv   [WidthModLength]            ; du   inv. inv. inv. UR   VR
    fmul    [FixedScale]                ; du8 inv. inv. inv. UR   VR
    fistp   [DeltaU]                    ; inv. inv. inv. UR   VR

    //this is hecker's klooge for keeping his fpu stack aligned (ught)
    ; @todo gross!  these are to line up with the other loop
    fld     st(1)                       ; inv. inv. inv. inv. UR   VR
    fld     st(2)                       ; inv. inv. inv. inv. inv. UR   VR

whtp_OnePixelSpan:

    mov     esi,[UFixed]                ; get starting coordinates
    add     esi,dword ptr grads.dUdXModifier
    mov     ecx,[VFixed]                  ; and v... before ebp gets pushed
    add     ecx,dword ptr grads.dVdXModifier

    ; leftover pixels loop
    ; edi = dest dib bits
    ; esi = texture dib bits

    ; esi = u 16.16
    ; ecx = v 16.16

    xor     eax,eax

    //this loop isn't too good... didn't get time to tune it well
    //but it's only for the end of the scanline if there are <8 pixels
whtp_LeftoverLoop:
    mov     edx,ecx                     ; copy v
    mov     ebx,esi

    sar     edx,16			                ; int(v)
    add     edi,2

whtp_twidth_log2_patch3:
    sal     edx,3
    add     esi,[DeltaU]                ; increment u coordinate

    sar     ebx,16			                ; int(u)
    add     edx,pTex

    add     ecx,[DeltaV]                ; increment v coordinate
    add     ebx,edx

    mov     edx,[LVal]
    mov     al,byte ptr[1+ebx*2]        ; get source texel

    rol     edx,16
    mov     bl,byte ptr[ebx*2]

    mov     ah,dl
    mov     eax,dword ptr[cTable+8000h+eax*4]

    mov     bh,dl
    and     ebx,0ffffh

    rol     edx,16
    add     eax,dword ptr[cTable+ebx*4]

    add     edx,[LDelta]
    mov     [edi-2],al

    mov     [LVal],edx
    mov     [edi-1],ah
    
    dec     [WidthModLength]            ; decrement loop count
    jnz     whtp_LeftoverLoop                ; finish up

whtp_FPUReturn:

    ; busy FPU registers:               ; st0  st1  st2  st3  st4  st5  st6  st7
                                        ; xxx  xxx  xxx  xxx  xxx  xxx  xxx
    ffree   st(0)
    ffree   st(1)
    ffree   st(2)
    ffree   st(3)
    ffree   st(4)
    ffree   st(5)
    ffree   st(6)

whtp_Return:

    fldcw   [OldFPUCW]                  ; restore the FPU
  }
}
#endif