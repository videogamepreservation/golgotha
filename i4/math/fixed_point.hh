/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef FIXED_POINT_HH
#define FIXED_POINT_HH

#include "arch.hh"

template <class ca, class cb>
inline ca i4_fixed_point_convert(ca &a, cb b)
// convert between fixed point precisions
//{{{
{
  return (a = ca(b.real(), b.fraction()));
}
//}}}

template <int precision>
class i4_fixed_point
{
protected:
  i4_fixed_point(sw32 a, int dummy, int dummy2) : value(a) {}
  //{{{ Notes:
  //  dummy parameters used to allow for standard constructors
  //  I'm hoping that the compiler will be smart enough to drop code for the dummies
  //}}}

#define DIRECT_CAST(a) i4_fixed_point(a,0,0)
public:
  sw32 value;

  i4_fixed_point() {}
  i4_fixed_point(const sw32 r, const w32 f) : value((r<<precision) | (f>>(32-precision))) {}

  //{{{ Standard Type conversions
  i4_fixed_point(const int a) : value(sw32(a)<<precision) {}
  i4_fixed_point(const unsigned int a) : value(sw32(a)<<precision) {}
  i4_fixed_point(const sw8 a) : value(sw32(a)<<precision) {}
  i4_fixed_point(const w8 a) : value(sw32(a)<<precision) {}
  i4_fixed_point(const sw16 a) : value(sw32(a)<<precision) {}
  i4_fixed_point(const w16 a) : value(sw32(a)<<precision) {}
  i4_fixed_point(const sw32 a) : value(a<<precision) {}
  i4_fixed_point(const w32 a) : value(a<<precision) {}
  i4_fixed_point(const float a) : value(sw32(a*float(1<<precision))) {}
  i4_fixed_point(const double a) : value(sw32(a*double(1<<precision))) {}

  operator int() const { return int(value>>precision); }
  operator unsigned int() const { return int(((w32)value)>>precision); }
  operator sw8() const { return sw8(value>>precision); }
  operator w8() const { return w8(((w32)value)>>precision); }
  operator sw16() const { return sw16(value>>precision); }
  operator w16() const { return w16(((w32)value)>>precision); }
  operator sw32() const { return value>>precision; }
  operator w32() const { return ((w32)value)>>precision; }
  operator double() const { return double(value)/double(1<<precision); }
  operator float() const { return float(value)/float(1<<precision); }
  //}}}

  i4_fixed_point operator+(const i4_fixed_point b) const { return DIRECT_CAST(value + b.value); }
  i4_fixed_point operator-(const i4_fixed_point b) const { return DIRECT_CAST(value - b.value); }
  i4_fixed_point operator*(const i4_fixed_point b) const 
  //{{{
  { 
    return DIRECT_CAST((value * b.value)>>precision); 
  }
  //}}}
  i4_fixed_point operator/(const i4_fixed_point b) const 
  //{{{
  {
    return DIRECT_CAST((value<<precision) / b.value); 
  }
  //}}}

  i4_fixed_point divide(const i4_fixed_point b, const int div_imprec = 0) const 
  //{{{
  //  imprecise divide to avoid overflows
  {
    return DIRECT_CAST((value<<(precision-div_imprec)) / (b.value>>div_imprec));
  }
  //}}}

  i4_fixed_point operator= (const i4_fixed_point b) { return DIRECT_CAST(value =  b.value); }
  i4_fixed_point operator+=(const i4_fixed_point b) { return DIRECT_CAST(value += b.value); }
  i4_fixed_point operator-=(const i4_fixed_point b) { return DIRECT_CAST(value -= b.value); }
  i4_fixed_point operator*=(const i4_fixed_point b) 
  //{{{
  { 
    return DIRECT_CAST(value = (value*b.value)>>precision); 
  }
  //}}}
  i4_fixed_point operator/=(const i4_fixed_point b)
  //{{{
  { 
    return DIRECT_CAST(value = (value<<precision) / b.value); 
  }
  //}}}

  i4_bool operator==(const i4_fixed_point b) const { return value == b.value; }
  i4_bool operator!=(const i4_fixed_point b) const { return value != b.value; }
  i4_bool operator> (const i4_fixed_point b) const { return value > b.value;  }
  i4_bool operator< (const i4_fixed_point b) const { return value < b.value;  }
  i4_bool operator>=(const i4_fixed_point b) const { return value >= b.value; }
  i4_bool operator<=(const i4_fixed_point b) const { return value <= b.value; }

  sw32 real() const { return value>>precision; }
  w32 fraction() const { return w32(value<<(32 - precision)); }
  w32 precision() const { return precision; }
};

#endif

//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}
