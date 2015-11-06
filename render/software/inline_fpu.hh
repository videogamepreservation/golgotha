/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef INLINE_FPU_HH
#define INLINE_FPU_HH

#include "software/r1_software_globals.hh"

#define bit(i) (1<<i)

inline void start_ceil()
{
  //keep these bits, set all others to 0
  w32 fpuand = bit(15) | bit(14) | bit(13) | bit(12) | bit(9) | bit(8) | 255;
  
  //then set bit 11 (round up, ceil for positive numbers)
  w32 fpuor  = bit(11); 
  
  _asm
  {
    xor     eax,eax
    fstcw   word ptr [old_ceil_word]
    mov     ax, word ptr [old_ceil_word]
    and     eax, dword ptr [fpuand]
    or      eax, dword ptr [fpuor]
    mov     word ptr [new_ceil_word],ax
    fldcw   word ptr [new_ceil_word]
  }
}

inline void start_trunc()
{
  //keep these bits, set all others to 0
  w32 fpuand = bit(15) | bit(14) | bit(13) | bit(12) | bit(9) | bit(8) | 255;
  
  //then set bit 11 and bit 10 (chop, true truncate)
  w32 fpuor  = bit(11) | bit(10); 
  
  _asm
  {
    xor     eax,eax
    fstcw   word ptr [old_trunc_word]
    mov     ax, word ptr [old_trunc_word]
    and     eax, dword ptr [fpuand]
    or      eax, dword ptr [fpuor]
    mov     word ptr [new_trunc_word],ax
    fldcw   word ptr [new_trunc_word]
  }
}

//this doesnt interfere with the truncation or ceil control words,
//so it can be used in conjunction (start_ceil(), start_single(), stop_single(), stop_ceil())

inline void start_single()
{
  //keep these bits, set all others to 0
  w32 fpuand = bit(15) | bit(14) | bit(13) | bit(12) | bit(11) | bit(10) | 255;
  
  //leave bits 8 and 9 off
  w32 fpuor  = 0;
  
  _asm
  {
    xor     eax,eax
    fstcw   word ptr [old_single_word]
    mov     ax, word ptr [old_single_word]
    and     eax, dword ptr [fpuand]
    or      eax, dword ptr [fpuor]
    mov     word ptr [new_single_word],ax
    fldcw   word ptr [new_single_word]
  }
}

inline void stop_single()
{
  _asm
  {
    fldcw dword ptr [old_single_word]
  }
}

inline void stop_trunc()
{
  _asm
  {
    fldcw dword ptr [old_trunc_word]
  }
}

inline void stop_ceil()
{
  _asm
  {
    fldcw dword ptr [old_ceil_word]
  }
}

inline sw32 qftoi(float a)
{
  sw32 res;
  
  _asm 
  {
    fld dword ptr [a]
    fistp dword ptr [res]
  }
  
  return res;
}

#endif