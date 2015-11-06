/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef __VECTOR_TEMPLATE_HPP_
#define __VECTOR_TEMPLATE_HPP_

#include "math/num_type.hh"
#include <math.h>

template <int dimension, class T>
class i4_vector_template
{
public:
  T elt[dimension];

  i4_vector_class() {}

  i4_vector_class(const T v)  
  {
    for (int i=0; i<dimension; i++)
      elt[i] = v;
  }

  i4_vector_class(const i4_vector_class &b)
  {
    memcpy(elt,b.elt,sizeof(*this));
  }

  i4_vector_class& operator=(const i4_vector_class &b)
  {
    for (int i=0; i<dimension; i++)
      elt[i] = b.elt[i];

    return *this;
  }

  i4_vector_class& operator+=(const i4_vector_class &b)
  {
    for (int i=0; i<dimension; i++)
      elt[i] += b.elt[i];

    return *this;
  }


  i4_vector_class& operator-=(const i4_vector_class &b)
  {
    for (int i=0; i<dimension; i++)
      elt[i] -= b.elt[i];

    return *this;
  }


  i4_vector_class& operator*=(const T b)
  {
    for (int i=0; i<dimension; i++)
      elt[i] *= b;

    return *this;
  }


  i4_vector_class& operator/=(const T b)
  {
    for (int i=0; i<dimension; i++)
      elt[i] /= b;

    return *this;
  }


  T dot(const i4_vector_class b) const
  {
    T ret = 0;

    for (int i=0; i<dimension; i++)
      ret += elt[i]*b.elt[i];

    return ret;
  }


  T length() const
  {
    return (sqrt(dot(*this)));
  }


  i4_vector_class& normalize()
  {
    T len = length();

    for (int i=0; i<dimension; i++)
      elt[i] /= len;

    return *this;
  }

};

template <class T>
class i4_vector3_class : public i4_vector_class<3,T>
{
  enum { x,y,z };
public:
  i4_vector3_class() {};
  i4_vector3_class(i4_float _x, i4_float _y, i4_float _z) 
  {
    elt[x]=_x;
    elt[y]=_y;
    elt[z]=_z;
  }

  const i4_vector3_class& cross(i4_vector3_class b)
  {
    i4_vector3_class temp(*this);

    elt[x] = temp.elt[y]*b.elt[z] - temp.elt[z]*b.elt[y];
    elt[y] = temp.elt[z]*b.elt[x] - temp.elt[x]*b.elt[z];
    elt[z] = temp.elt[x]*b.elt[y] - temp.elt[y]*b.elt[x];

    return *this;
  }

  void set(i4_float _x, i4_float _y, i4_float _z)
  {
    elt[x]=_x;
    elt[y]=_y;
    elt[z]=_z;
  }
};



typedef i4_vector3_class<i4_float>  i4_3d_vector;
typedef i4_vector_class<4,i4_float> i4_4d_vector;


#endif
