/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "objs/model_collide.hh"
#include "math/transform.hh"
#include "g1_object.hh"
#include "objs/model_id.hh"
#include "math/pi.hh"
#include "math/angle.hh"

i4_bool g1_model_collide_polygonal(g1_object_class *_this,
                                   g1_model_draw_parameters &params,
                                   const i4_3d_vector &start,
                                   i4_3d_vector &ray,
                                   i4_3d_vector &hit_normal,
                                   i4_float *minimum_t)
{  
  i4_transform_class transform, mini_local;
  i4_3d_vector istart, iray, normal;
  i4_float t, min_t=1.0;
  int poly;
  
  g1_quad_object_class *model = params.model;
  if (!model) return i4_F;

  _this->calc_world_transform(1.0, &transform);

  transform.inverse_transform(start, istart);
  transform.inverse_transform_3x3(ray, iray);

  if (model->intersect(istart, iray, params.animation, params.frame, &t, &poly, &normal))
    if (t<min_t)
    {
      min_t = t;
      transform.transform_3x3(normal, hit_normal);
    }

  i4_3d_vector mstart, mray;
  for (int i=0; i<_this->num_mini_objects; i++)
  {
    g1_mini_object *mini = &_this->mini_objects[i];

    if (mini->defmodeltype!=0xffff)
    {
      mini->calc_transform(1.0, &mini_local);
      mini_local.inverse_transform(istart, mstart);
      mini_local.inverse_transform_3x3(iray, mray);
      model = g1_model_list_man.get_model(mini->defmodeltype);
      if (model->intersect(mstart, mray, mini->animation, mini->frame, &t, &poly, &normal))
      {
        if (t<min_t)
        {
          min_t = t;
          i4_3d_vector local_norm;
          mini_local.transform_3x3(normal, local_norm);
          transform.transform_3x3(local_norm, hit_normal);
        }
      }
    }
  }

  if (minimum_t)
    *minimum_t = min_t;

  if (min_t<1.0)
  {
    ray *= min_t;
    return i4_T;
  }
  return i4_F;
}

i4_bool g1_model_collide_radial(g1_object_class *_this,
                                g1_model_draw_parameters &params,
                                const i4_3d_vector &start,
                                i4_3d_vector &ray)
{
      
  // intersection of a ray with a sphere from graphics gems 1  p. 388
  i4_3d_vector EO, V, O;
  float r, disc, EO_length, d, v;
  
  V=ray;
  V.normalize();

  r=_this->occupancy_radius();  
  float z_dist=_this->h - start.z;
//   if (z_dist*2<r)        // fudge the z because some vehicles are too short
//     z_dist=0;

  EO=i4_3d_vector(_this->x - start.x, _this->y - start.y, z_dist);
  v=EO.dot(V);

  EO_length=EO.length();

  disc=r*r - (EO_length*EO_length - v*v);
  
  if (disc<0)
    return i4_F;
  else
  {
    d=sqrt(disc);
    V*=(v-d);
    ray=V;
    return i4_T;
  }

  
//   i4_float a,b,c,d,r, gx,gy;

//   gx = _this->x - start.x;
//   gy = _this->y - start.y;
//   d  = gx*gx + gy*gy;
//   r  = _this->occupancy_radius();      
//   r = r*r;

//   //first bounding radius check
//   if (d < r)
//     return i4_T;
      
//   //then ray/circle intersection check
//   b  = ray.x*ray.x + ray.y*ray.y;
//   a  = ray.x*gx + ray.y*gy;
//   c  = (a*a)/b;
      
//   return (r>d-c);
}
