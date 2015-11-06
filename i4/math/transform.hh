/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef __MATRIX_HPP_
#define __MATRIX_HPP_

#include <string.h>
#include "math/point.hh"

#ifdef _WINDOWS
//#define G1_WIN32_TRANSFORM_ASM
#endif

class i4_transform_class 
{
public:
  i4_3d_vector x,y,z,t;

  i4_3d_vector& operator[](int pos) const
  { 
    return (i4_3d_vector&)(((i4_3d_vector*)(&x))[pos]);
  }
  
  i4_transform_class() {}
  i4_transform_class(const i4_transform_class &m)
  {
    memcpy(this,&m,sizeof(i4_transform_class));
  }
  i4_transform_class& operator=(const i4_transform_class &m)
  {
    memcpy(this,&m,sizeof(i4_transform_class));

    return *this;
  }
  
  i4_transform_class& multiply(const i4_transform_class &a, const i4_transform_class &b);  
  i4_transform_class& identity()
  {
    return uniscale(i4_float(1));
  }
  i4_transform_class& translate(const i4_3d_vector& offset);  
  i4_transform_class& mult_translate(const i4_3d_vector& offset);

  i4_transform_class& translate(i4_float tx, i4_float ty, i4_float tz)
  {
    return translate(i4_3d_vector(tx,ty,tz));
  }
  i4_transform_class& mult_translate(i4_float tx, i4_float ty, i4_float tz)
  {
    return mult_translate(i4_3d_vector(tx,ty,tz));
  }

  i4_transform_class& rotate_x_y_z(i4_float x, i4_float y, i4_float z,i4_bool use_lookup=i4_F);

  i4_transform_class& rotate_z(i4_angle th);
  i4_transform_class& mult_rotate_z(i4_angle th);
  i4_transform_class& rotate_y(i4_angle th);
  i4_transform_class& mult_rotate_y(i4_angle th);
  i4_transform_class& rotate_x(i4_angle th);
  i4_transform_class& mult_rotate_x(i4_angle th);  
  i4_transform_class& scale(const i4_3d_vector& scale);
  
  i4_transform_class& mult_scale(const i4_3d_vector& scale)
  {
    x *= scale.x;
    y *= scale.y;
    z *= scale.z;

    return *this;
  }
  i4_transform_class& scale(i4_float tx, i4_float ty, i4_float tz)
  {
    return scale(i4_3d_vector(tx,ty,tz));
  }
  i4_transform_class& mult_scale(i4_float tx, i4_float ty, i4_float tz)
  {
    return mult_scale(i4_3d_vector(tx,ty,tz));
  }
  i4_transform_class& uniscale(i4_float scale)
  {
    x.set( scale,       i4_float(0), i4_float(0) );
    y.set( i4_float(0), scale,       i4_float(0) );
    z.set( i4_float(0), i4_float(0), scale       );
    t.set( i4_float(0), i4_float(0), i4_float(0) );

    return *this;
  }
  i4_transform_class& mult_uniscale(i4_float scale)
  {
    x *= scale;
    y *= scale;
    z *= scale;

    return *this;
  }
  
  // this version of multiple store the result in this
  i4_transform_class& multiply(const i4_transform_class &b)
  {
    i4_transform_class tmp(*this);
    multiply(tmp, b);

    return *this;
  }
  
  i4_3d_vector& transform(const i4_3d_vector& src, i4_3d_vector& dst)
  {
#ifdef G1_WIN32_TRANSFORM_ASM
    trans_4x3(src, dst);
#else
    dst.x = x.x*src.x + y.x*src.y + z.x*src.z + t.x;
    dst.y = x.y*src.x + y.y*src.y + z.y*src.z + t.y;
    dst.z = x.z*src.x + y.z*src.y + z.z*src.z + t.z;
#endif
    return dst;
  }
  
  i4_3d_vector& transform_3x3(const i4_3d_vector& src, i4_3d_vector& dst)
  {
#ifdef G1_WIN32_TRANSFORM_ASM
    trans_3x3(src, dst);
#else
    dst.x = x.x*src.x + y.x*src.y + z.x*src.z;
    dst.y = x.y*src.x + y.y*src.y + z.y*src.z;
    dst.z = x.z*src.x + y.z*src.y + z.z*src.z;
#endif
    return dst;
  }
  
  i4_3d_vector& inverse_transform(const i4_3d_vector& src, i4_3d_vector& dst) const
  {
    i4_3d_vector tmp = src;

    tmp -= t;
    dst.x = tmp.dot(x);
    dst.y = tmp.dot(y);
    dst.z = tmp.dot(z);

    return dst;
  }

  i4_3d_vector& inverse_transform_3x3(const i4_3d_vector &src, i4_3d_vector &dst) const
  {
    i4_3d_vector tmp = src;

    dst.x = tmp.dot(x);
    dst.y = tmp.dot(y);
    dst.z = tmp.dot(z);

    return dst;
  }
  
#ifdef G1_WIN32_TRANSFORM_ASM
  void trans_4x3(const i4_3d_vector &src_vector, i4_3d_vector &dst_vector)
  { 
    _asm 
    {
      mov     eax, this
      mov     ecx, src_vector
      mov     edx, dst_vector

      ;optimized transformation: 34 cycles                
      fld dword ptr  [ecx+0]     ;starts & ends on cycle 0
      fmul dword ptr [eax+0]     ;starts on cycle 1       
      fld dword ptr  [ecx+0]     ;starts & ends on cycle 2
      fmul dword ptr [eax+4]     ;starts on cycle 3        
      fld dword ptr  [ecx+0]     ;starts & ends on cycle 4
      fmul dword ptr [eax+8]     ;starts on cycle 5        
      fld dword ptr  [ecx+4]     ;starts & ends on cycle 6
      fmul dword ptr [eax+12]    ;starts on cycle 7      
      fld dword ptr  [ecx+4]     ;starts & ends on cycle 8
      fmul dword ptr [eax+16]    ;starts on cycle 9       
      fld dword ptr  [ecx+4]     ;starts & ends on cycle 10
      fmul dword ptr [eax+20]    ;starts on cycle 11  
      fxch           st(2)       ;no cost             
      faddp          st(5),st(0) ;starts on cycle 12  
      faddp          st(3),st(0) ;starts on cycle 13  
      faddp          st(1),st(0) ;starts on cycle 14  
      fld dword ptr  [ecx+8]     ;starts & ends on cycle 15
      fmul dword ptr [eax+24]    ;starts on cycle 16      
      fld dword ptr  [ecx+8]     ;starts & ends on cycle 17
      fmul dword ptr [eax+28]    ;starts on cycle 18       
      fld dword ptr  [ecx+8]     ;starts & ends on cycle 19
      fmul dword ptr [eax+32]    ;starts on cycle 20       
      fxch           st(2)       ;no cost                  
      faddp          st(5),st(0) ;starts on cycle 21       
      faddp          st(3),st(0) ;starts on cycle 22       
      faddp          st(1),st(0) ;starts on cycle 23       
      fxch           st(2)       ;no cost                  
      fadd dword ptr [eax+36]    ;starts on cycle 24       
      fxch           st(1)       ;starts on cycle 25       
      fadd dword ptr [eax+40]    ;starts on cycle 26       
      fxch           st(2)       ;no cost                  
      fadd dword ptr [eax+44]    ;starts on cycle 27       
      fxch           st(1)       ;no cost                  
      fstp dword ptr [edx+0]     ;starts on cycle 28, ends on cycle 29
      fstp dword ptr [edx+8]     ;starts on cycle 30, ends on cycle 31
      fstp dword ptr [edx+4]     ;starts on cycle 32, ends on cycle 33
    }  
  }

  void trans_3x3(const i4_3d_vector &src_vector, i4_3d_vector &dst_vector)
  { 
    _asm 
    {
      mov     eax, this
      mov     ecx, src_vector
      mov     edx, dst_vector

      ;optimized transformation: 34 cycles                
      fld dword ptr  [ecx+0]     ;starts & ends on cycle 0
      fmul dword ptr [eax+0]     ;starts on cycle 1       
      fld dword ptr  [ecx+0]     ;starts & ends on cycle 2
      fmul dword ptr [eax+4]     ;starts on cycle 3        
      fld dword ptr  [ecx+0]     ;starts & ends on cycle 4
      fmul dword ptr [eax+8]     ;starts on cycle 5        
      fld dword ptr  [ecx+4]     ;starts & ends on cycle 6
      fmul dword ptr [eax+12]    ;starts on cycle 7      
      fld dword ptr  [ecx+4]     ;starts & ends on cycle 8
      fmul dword ptr [eax+16]    ;starts on cycle 9       
      fld dword ptr  [ecx+4]     ;starts & ends on cycle 10
      fmul dword ptr [eax+20]    ;starts on cycle 11  
      fxch           st(2)       ;no cost             
      faddp          st(5),st(0) ;starts on cycle 12  
      faddp          st(3),st(0) ;starts on cycle 13  
      faddp          st(1),st(0) ;starts on cycle 14  
      fld dword ptr  [ecx+8]     ;starts & ends on cycle 15
      fmul dword ptr [eax+24]    ;starts on cycle 16      
      fld dword ptr  [ecx+8]     ;starts & ends on cycle 17
      fmul dword ptr [eax+28]    ;starts on cycle 18       
      fld dword ptr  [ecx+8]     ;starts & ends on cycle 19
      fmul dword ptr [eax+32]    ;starts on cycle 20       
      fxch           st(2)       ;no cost                  
      faddp          st(5),st(0) ;starts on cycle 21       
      faddp          st(3),st(0) ;starts on cycle 22       
      faddp          st(1),st(0) ;starts on cycle 23       
      fxch           st(2)       ;no cost                  
      fstp dword ptr [edx+0]
      fstp dword ptr [edx+4]
      fstp dword ptr [edx+8]
    }  
  }
#endif
};

#endif

