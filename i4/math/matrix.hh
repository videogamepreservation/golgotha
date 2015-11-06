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

class i4_4x4_matrix_class
{
protected:
  i4_float elt[4*4];
public:
  
  i4_4x4_matrix_class() { ; }

  i4_4x4_matrix_class(const i4_4x4_matrix_class &copy_from)
  //{{{
  {
    memcpy(elt,copy_from.elt,sizeof(i4_4x4_matrix_class));
  }
  //}}}

  void scale(i4_float sx, i4_float sy, i4_float sz)
  //{{{
  {
    elt[0]=sx;   elt[1]=0.0;    elt[2]=0.0;    elt[3]=0.0;
    elt[4]=0.0;    elt[5]=sy;   elt[6]=0.0;    elt[7]=0.0;
    elt[8]=0.0;    elt[9]=0.0;    elt[10]=sz;  elt[11]=0.0;
    elt[12]=0.0;   elt[13]=0.0;   elt[14]=0;   elt[15]=1.0;
  }
  //}}}

  void identity()
  //{{{
  {
    scale(1,1,1);
  }
  //}}}

  void translate(i4_float tx, i4_float ty, i4_float tz)
  //{{{
  {
    elt[0] = 1.0;    elt[1]  = 0.0;    elt[2]  = 0.0;    elt[3] = tx;
    elt[4] = 0.0;    elt[5]  = 1.0;    elt[6]  = 0.0;    elt[7] = ty;
    elt[8] = 0.0;    elt[9]  = 0.0;    elt[10] = 1.0;   elt[11] = tz;
    elt[12]= 0.0;    elt[13] = 0.0;    elt[14] = 0.0;   elt[15] = 1.0;
  }
  //}}}

  void rotate_x(i4_angle ax)
  //{{{
  {
    i4_float cos_ax=cos(ax);
    i4_float sin_ax=sin(ax);

    elt[0]=1.0;        elt[1]=0.0;           elt[2]=0.0;        elt[3]=0.0;
    elt[4]=0.0;        elt[5]=cos_ax;        elt[6]=-sin_ax;    elt[7]=0.0;
    elt[8]=0.0;        elt[9]=sin_ax;        elt[10]=cos_ax;    elt[11]=0.0;
    elt[12]=0.0;       elt[13]=0.0;          elt[14]=0.0;       elt[15]=1.0;
  }
  //}}}

  void rotate_y(i4_angle ay)
  //{{{
  {
    i4_float cos_ay=cos(ay);
    i4_float sin_ay=sin(ay);

    elt[0]=cos_ay;   elt[1]=0.0;             elt[2]=sin_ay;    elt[3]=0.0;
    elt[4]=0.0;      elt[5]=1.0;             elt[6]=0.0;       elt[7]=0.0;
    elt[8]=-sin_ay;  elt[9]=0.0;             elt[10]=cos_ay;   elt[11]=0.0;
    elt[12]=0.0;     elt[13]=0.0;            elt[14]=0.0;      elt[15]=1.0;
  }
  //}}}

  void rotate_z(i4_angle az)
  //{{{
  {
    i4_float cos_az=cos(az);
    i4_float sin_az=sin(az);

    elt[0]=cos_az;   elt[1]=-sin_az;       elt[2]=0.0;          elt[3]=0.0;
    elt[4]=sin_az;   elt[5]=cos_az;        elt[6]=0.0;          elt[7]=0.0;
    elt[8]=0.0;      elt[9]=0.0;           elt[10]=1.0;         elt[11]=0.0;
    elt[12]=0.0;     elt[13]=0.0;          elt[14]=0.0;         elt[15]=1.0;
  }
  //}}}
  
  i4_4x4_matrix_class &multiply(const i4_4x4_matrix_class &a, const i4_4x4_matrix_class &b)
  //{{{
  {
    int i,j,k;
    i4_float sum;

    for (i=0;i<4;i++)
      for (j=0;j<4;j++)
      {   
        sum=0;
        for (k=0; k<4; k++)
          sum += a.elt[i*4+k] * b.elt[k*4+j];
        elt[i*4+j] = sum;
      }
    return *this;
  }
  //}}}

  // this version of multiple store the result in this
  void multiply(const i4_4x4_matrix_class &b)
  //{{{
  {
    i4_4x4_matrix_class result;
    multiply(b,result);
    *this=result;
  }
  //}}}

  void transform(const i4_3d_vector &b, i4_3d_vector &result)
  //{{{
  { 
    i4_float vx,vy,vz;
    vx=b.x;
    vy=b.y;
    vz=b.z;
          
    result.x = vx*elt[0] + vy*elt[1] + vz*elt[2] + elt[3];
    result.y = vx*elt[4] + vy*elt[5] + vz*elt[6] + elt[7];
    result.z = vx*elt[8] + vy*elt[9] + vz*elt[10] + elt[11];
  }
  //}}}
};

#endif

//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}
