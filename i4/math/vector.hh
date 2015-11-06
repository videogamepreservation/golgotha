/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef __VECTOR_HPP_
#define __VECTOR_HPP_

#include "math/num_type.hh"
#include <math.h>

#ifdef _WINDOWS
//#define G1_WIN32_VECTOR_ASM
#endif


template <class Coord>
class i4_vector3_template
{
public:
  Coord x,y,z;

  Coord& operator()(int pos) const { return ((Coord*)&x)[pos]; }
  Coord& operator[](int pos) const { return ((Coord*)&x)[pos]; }

  i4_vector3_template() {}
  i4_vector3_template(const i4_vector3_template& p) : x(p.x), y(p.y), z(p.z) {}
  i4_vector3_template(Coord _x,Coord _y,Coord _z) : x(_x),y(_y),z(_z) {}
  i4_vector3_template& set(Coord px,Coord py,Coord pz)
  {
    x = px;
    y = py;
    z = pz;

    return *this;
  }
  i4_vector3_template& operator=(const i4_vector3_template& p)
  {
    x = p.x;
    y = p.y;
    z = p.z;

    return *this;
  }
  
  i4_vector3_template& operator+=(const i4_vector3_template& b)
  {
    x += b.x;
    y += b.y;
    z += b.z;

    return *this;
  }
  i4_vector3_template& operator-=(const i4_vector3_template& b)
  {
    x -= b.x;
    y -= b.y;
    z -= b.z;

    return *this;
  }

  i4_vector3_template& operator*=(Coord b)
  {
    x *= b;
    y *= b;
    z *= b;

    return *this;
  }

  i4_vector3_template operator*(Coord b)
  {
    i4_vector3_template tmp;
    tmp.x = x * b;
    tmp.y = y * b;
    tmp.z = z * b;

    return tmp;
  }


  i4_vector3_template operator+(Coord b)
  {
    i4_vector3_template tmp;
    tmp.x = x + b;
    tmp.y = y + b;
    tmp.z = z + b;

    return tmp;
  }

  i4_vector3_template operator+(i4_vector3_template v)
  {
    i4_vector3_template tmp;
    tmp.x = x + v.x;
    tmp.y = y + v.y;
    tmp.z = z + v.z;

    return tmp;
  }

  i4_vector3_template operator*(i4_vector3_template v)
  {
    i4_vector3_template tmp;
    tmp.x = x * v.x;
    tmp.y = y * v.y;
    tmp.z = z * v.z;

    return tmp;
  }

  i4_vector3_template& operator/=(Coord b)
  {
    x /= b;
    y /= b;
    z /= b;

    return *this;
  }

  i4_vector3_template& interpolate(const i4_vector3_template& from, const i4_vector3_template& to, 
                                   i4_float ratio)
  {
    x = i4_interpolate(from.x,to.x,ratio);
    y = i4_interpolate(from.y,to.y,ratio);
    z = i4_interpolate(from.z,to.z,ratio);
    return *this;
  }
  
  Coord length() const
  {
    return (Coord)sqrt(dot(*this));
  }
  void normalize()
  {
    *this /= length();
  }

  i4_vector3_template& reverse()
  {
    x=-x; y=-y; z=-z;

    return *this;
  }

  Coord dot(const i4_vector3_template& b) const
  {
#ifdef G1_WIN32_VECTOR_ASM
    i4_float dotret;
    _asm
    {
      mov     ecx, b
      mov     eax, this

      ;optimized dot product; 15 cycles
      fld dword ptr   [eax+0]     ;starts & ends on cycle 0
      fmul dword ptr  [ecx+0]     ;starts on cycle 1
      fld dword ptr   [eax+4]     ;starts & ends on cycle 2
      fmul dword ptr  [ecx+4]     ;starts on cycle 3
      fld dword ptr   [eax+8]     ;starts & ends on cycle 4
      fmul dword ptr  [ecx+8]     ;starts on cycle 5
      fxch            st(1)       ;no cost
      faddp           st(2),st(0) ;starts on cycle 6, stalls for cycles 7-8
      faddp           st(1),st(0) ;starts on cycle 9, stalls for cycles 10-12
      fstp dword ptr  [dotret]    ;starts on cycle 13, ends on cycle 14
    }
    return dotret;
#else
    return x*b.x + y*b.y + z*b.z;
#endif
  }

    i4_vector3_template& cross(const i4_vector3_template& a, 
                                    const i4_vector3_template& b)
  {
#ifdef UNFINISHED_WINDOWS
    _asm
    {
      mov     eax,this
      mov     ecx,a
      mov     edx,b

      ;optimized cross product; 22 cycles
      fld dword ptr   [ecx+4]     ;starts & ends on cycle 0
      fmul dword ptr  [edx+8]     ;starts on cycle 1       
      fld dword ptr   [ecx+8]     ;starts & ends on cycle 2
      fmul dword ptr  [edx+0]     ;starts on cycle 3       
      fld dword ptr   [ecx+0]     ;starts & ends on cycle 4
      fmul dword ptr  [edx+4]     ;starts on cycle 5       
      fld dword ptr   [ecx+8]     ;starts & ends on cycle 6
      fmul dword ptr  [edx+4]     ;starts on cycle 7       
      fld dword ptr   [ecx+0]     ;starts & ends on cycle 8
      fmul dword ptr  [edx+8]     ;starts on cycle 9         
      fld dword ptr   [ecx+4]     ;starts & ends on cycle 10 
      fmul dword ptr  [edx+0]     ;starts on cycle 11        
      fxch            st(2)       ;no cost                   
      fsubrp          st(5),st(0) ;starts on cycle 12        
      fsubrp          st(3),st(0) ;starts on cycle 13        
      fsubrp          st(1),st(0) ;starts on cycle 14        
      fxch            st(2)       ;no cost, stalls for cycle 15
      fstp dword ptr  [eax+0]     ;starts on cycle 16, ends on cycle 17  
      fstp dword ptr  [eax+4]     ;starts on cycle 18, ends on cycle 19
      fstp dword ptr  [eax+8]     ;starts on cycle 20, ends on cycle 21
    }
#else
    x = a.y*b.z - a.z*b.y;
    y = a.z*b.x - a.x*b.z;
    z = a.x*b.y - a.y*b.x;
#endif
    return *this;
  }
};


class i4_2d_vector
{
public:
  i4_float x,y;

  i4_2d_vector() { ; }

  i4_2d_vector(i4_float x, i4_float y) : x(x), y(y) {}

  i4_float length() const { return sqrt(x*x+y*y); }
  i4_float dot(const i4_2d_vector &b)  { return x*b.x + y*b.y; }
  i4_float perp_dot(const i4_2d_vector &b)  { return x*b.y-y*b.x; }

  i4_2d_vector& operator-=(const i4_2d_vector& b) { x-=b.x; y-=b.y; return *this; }
  i4_2d_vector& operator+=(const i4_2d_vector& b) { x+=b.x; y+=b.y; return *this; }
  i4_2d_vector& operator*=(const i4_2d_vector& b) { x*=b.x; y*=b.y; return *this; }
  i4_2d_vector& operator/=(const i4_2d_vector& b) { x/=b.x; y/=b.y; return *this; }
  
  i4_2d_vector operator+(const i4_2d_vector& b) { return i4_2d_vector(x+b.x, y+b.y); }
  i4_2d_vector operator-(const i4_2d_vector& b) { return i4_2d_vector(x-b.x, y-b.y); }
  i4_2d_vector operator*(const i4_2d_vector& b) { return i4_2d_vector(x*b.x, y*b.y); }
  i4_2d_vector operator/(const i4_2d_vector& b) { return i4_2d_vector(x*b.x, y*b.y); }

  void normalize() 
  {
    i4_float ool=1.0/length();
    x*=ool;
    y*=ool;
  }
};


typedef i4_vector3_template<i4_float>  i4_3d_vector;

#endif


