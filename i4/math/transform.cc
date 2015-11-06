/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "math/transform.hh"

i4_transform_class& i4_transform_class::multiply(const i4_transform_class &a, const i4_transform_class &b)
{
  // expanded transform multiply
  x.x = a.x.x*b.x.x + a.y.x*b.x.y + a.z.x*b.x.z;
  x.y = a.x.y*b.x.x + a.y.y*b.x.y + a.z.y*b.x.z;
  x.z = a.x.z*b.x.x + a.y.z*b.x.y + a.z.z*b.x.z;
  y.x = a.x.x*b.y.x + a.y.x*b.y.y + a.z.x*b.y.z;
  y.y = a.x.y*b.y.x + a.y.y*b.y.y + a.z.y*b.y.z;
  y.z = a.x.z*b.y.x + a.y.z*b.y.y + a.z.z*b.y.z;
  z.x = a.x.x*b.z.x + a.y.x*b.z.y + a.z.x*b.z.z;
  z.y = a.x.y*b.z.x + a.y.y*b.z.y + a.z.y*b.z.z;
  z.z = a.x.z*b.z.x + a.y.z*b.z.y + a.z.z*b.z.z;
  t.x = a.x.x*b.t.x + a.y.x*b.t.y + a.z.x*b.t.z + a.t.x;
  t.y = a.x.y*b.t.x + a.y.y*b.t.y + a.z.y*b.t.z + a.t.y;
  t.z = a.x.z*b.t.x + a.y.z*b.t.y + a.z.z*b.t.z + a.t.z;
  /*
    _asm
    {
      mov     eax,this
      mov     ecx,a
      mov     edx,b

      fld dword ptr   [ecx+0]
      fld             st
      fmul dword ptr  [edx+0]
      fld dword ptr   [ecx+4]
      fld             st
      fmul dword ptr  [edx+0]
      fld dword ptr   [ecx+8]
      fld             st
      fmul dword ptr  [edx+0]
    }

*/
  return *this;
}

i4_transform_class& i4_transform_class::translate(const i4_3d_vector& offset)
{
  x.set(i4_float(1),i4_float(0),i4_float(0));
  y.set(i4_float(0),i4_float(1),i4_float(0));
  z.set(i4_float(0),i4_float(0),i4_float(1));
  t = offset;

  return *this;
}


i4_transform_class& i4_transform_class::mult_translate(const i4_3d_vector& offset)
{
  t.x += x.x*offset.x + y.x*offset.y + z.x*offset.z;
  t.y += x.y*offset.x + y.y*offset.y + z.y*offset.z;
  t.z += x.z*offset.x + y.z*offset.y + z.z*offset.z;

  return *this;
}

i4_transform_class& i4_transform_class::rotate_z(i4_angle th)
{
  i4_float cos_th=cos(th);
  i4_float sin_th=sin(th);

  x.set( cos_th,      sin_th,      i4_float(0));
  y.set( -sin_th,     cos_th,      i4_float(0));
  z.set( i4_float(0), i4_float(0), i4_float(1));
  t.set( i4_float(0), i4_float(0), i4_float(0));
  return *this;
}

i4_transform_class& i4_transform_class::mult_rotate_z(i4_angle th)
{
  i4_float cos_th=cos(th);
  i4_float sin_th=sin(th);
  i4_float xx, xy, xz, yx, yy, yz;

  xx = y.x*sin_th + x.x*cos_th;
  xy = y.y*sin_th + x.y*cos_th;
  xz = y.z*sin_th + x.z*cos_th;
  yx = y.x*cos_th - x.x*sin_th;
  yy = y.y*cos_th - x.y*sin_th;
  yz = y.z*cos_th - x.z*sin_th;

  x.set(xx,xy,xz);
  y.set(yx,yy,yz);
  return *this;
}

i4_transform_class& i4_transform_class::rotate_y(i4_angle th)
{
  i4_float cos_th=cos(th);
  i4_float sin_th=sin(th);

  x.set( cos_th,      i4_float(0), -sin_th     );
  y.set( i4_float(0), i4_float(1), i4_float(0) );
  z.set( sin_th,      i4_float(0), cos_th      );
  t.set( i4_float(0), i4_float(0), i4_float(0) );
  return *this;
}


i4_transform_class& i4_transform_class::mult_rotate_y(i4_angle th)
{
  i4_float cos_th=cos(th);
  i4_float sin_th=sin(th);
  i4_float xx, xy, xz, zx, zy, zz;

  xx = x.x*cos_th - z.x*sin_th;
  xy = x.y*cos_th - z.y*sin_th;
  xz = x.z*cos_th - z.z*sin_th;
  zx = x.x*sin_th + z.x*cos_th;
  zy = x.y*sin_th + z.y*cos_th;
  zz = x.z*sin_th + z.z*cos_th;

  x.set(xx,xy,xz);
  z.set(zx,zy,zz);
  return *this;
}

i4_transform_class& i4_transform_class::rotate_x(i4_angle th)
{
  i4_float cos_th=cos(th);
  i4_float sin_th=sin(th);

  x.set( i4_float(1), i4_float(0), i4_float(0) );
  y.set( i4_float(0), cos_th,      sin_th      );
  z.set( i4_float(0), -sin_th,     cos_th      );
  t.set( i4_float(0), i4_float(0), i4_float(0) );
  return *this;
}

i4_transform_class& i4_transform_class::mult_rotate_x(i4_angle th)
{
  i4_float cos_th=cos(th);
  i4_float sin_th=sin(th);
  i4_float yx, yy, yz, zx, zy, zz;

  yx = z.x*sin_th + y.x*cos_th;
  yy = z.y*sin_th + y.y*cos_th;
  yz = z.z*sin_th + y.z*cos_th;
  zx = z.x*cos_th - y.x*sin_th;
  zy = z.y*cos_th - y.y*sin_th;
  zz = z.z*cos_th - y.z*sin_th;

  y.set(yx,yy,yz);
  z.set(zx,zy,zz);
  return *this;
}

i4_transform_class& i4_transform_class::scale(const i4_3d_vector& scale)
{
  x.set( scale.x,     i4_float(0), i4_float(0) );
  y.set( i4_float(0), scale.y,     i4_float(0) );
  z.set( i4_float(0), i4_float(0), scale.z     );
  t.set( i4_float(0), i4_float(0), i4_float(0) );

  return *this;
}
