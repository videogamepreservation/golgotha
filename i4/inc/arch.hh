/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

//{{{ Interface 4 
//
//  System dependant #defines and macros
//  Endianess macros
//    short_swap,long_swap  - exchange endianess
//    lstl, lltl            - little short to local, little long to local (swaps to x86 endianess)
//

#ifndef __ARCH_HPP_
#define __ARCH_HPP_

#ifdef __MAC__
#include <Types.h>
#include <Files.h>
#include <Errors.h>
#endif


// 32 bit architectures
#if (__sgi || __linux || __WATCOMC__ || SUN4 || _WINDOWS || __MAC__)
typedef unsigned long   w32;
typedef unsigned short  w16;
typedef unsigned char   w8;
typedef signed long     sw32;
typedef signed short    sw16;
typedef signed char     sw8;

#if (__linux || __sgi || SUN4)         // gcc has long long
typedef unsigned long long  w64;
typedef long long          sw64;
#elif (_WINDOWS)                      // visual c has __int64
typedef unsigned __int64    w64;
typedef __int64            sw64;
#else
#error please define w64
#endif


#else
// 16 and 64 bit architectures are not defined yet
// but can be added here as long as the compiler supports 32 bit words
#error unknown architecture, please define basic types
#endif


#define w16_swap(x) (((((w16) (x)))<<8)|((((w16) (x)))>>8))
#define w32_swap(x) \
    ((( ((w32)(x)) )>>24)|((( ((w32)(x)) )&0x00ff0000)>>8)| \
    ((( ((w32)(x)) )&0x0000ff00)<<8)|(( ((w32)(x)) )<<24))


#if (__linux || __WATCOMC__ || _WINDOWS)

enum { i4_bigend=0, i4_litend=1 };

#define s_to_lsb(x) (x)
#define l_to_lsb(x) (x)
#define s_to_msb(x) w16_swap(x)
#define l_to_msb(x) w32_swap(x)

#else

enum { i4_bigend=1, i4_litend=0 };

#define s_to_lsb(x) w16_swap(x)
#define l_to_lsb(x) w32_swap(x)
#define s_to_msb(x) (x)
#define l_to_msb(x) (x)

#endif


typedef sw16 i4_coord;  // use this type for all x & y positions withing images
typedef w32  i4_color;  // reserved as 32bits in case we expand to 32 bit color

#define i4_null 0

#ifdef _WINDOWS
#define I4_FAST_CALL __fastcall
#else
#define I4_FAST_CALL
#endif

typedef w8 i4_bool;

enum  { i4_F=0, 
        i4_T=1 };


// use this mainly for events, to cast to a known event type
#define CAST_PTR(var_name, type, source_obj) type *var_name=(type *)(source_obj)

inline void *ALIGN_FORWARD(void *addr)
{
  return (void*)(((w32)addr+3)&~3);
}

inline void *ALIGN_BACKWARD(void *addr)
{
  return (void*)(((w32)addr)&~3);
}

#endif
