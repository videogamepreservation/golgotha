/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "math/spline.hh"
#include "memory/lalloc.hh"
#include "math/matrix.hh"
#include "error/error.hh"
#include "file/file.hh"

i4_linear_allocator *i4_spline_class::p_alloc=0;
w32 i4_spline_class::point_refs=0;

void i4_spline_class::move(i4_float x_add, i4_float y_add, i4_float z_add)
{
  for (point *p=points; p; p=p->next)  
  {
    p->x+=x_add;
    p->y+=y_add;
    p->z+=z_add;
  }
}

void i4_spline_class::save(i4_file_class *fp)
{
  fp->write_32(t_points);
  for (point *p=points; p; p=p->next)
  {
    fp->write_32(p->frame);
    fp->write_8(p->selected);
    fp->write_float(p->x);
    fp->write_float(p->y);
    fp->write_float(p->z);
  }
}

void i4_spline_class::load(i4_file_class *fp)
{
  cleanup();
  t_points=fp->read_32();
  point *last=0;

  for (w32 i=0; i<t_points; i++)
  {
    point *p=(point *)p_alloc->alloc(); 
    p->frame=fp->read_32();
    p->selected=fp->read_8();
    p->x=fp->read_float();
    p->y=fp->read_float();
    p->z=fp->read_float();
    p->next=0;

    if (last)
      last->next=p;
    else points=p;
    last=p;
  }
}

class i4_1x4_vector
{
public:
  i4_float x,y,z,t;
  i4_1x4_vector() { ; }
  i4_1x4_vector(i4_float x, i4_float y, i4_float z, i4_float t) : x(x), y(y), z(z), t(t) { ; }
  i4_float dot(const i4_1x4_vector &b) { return b.x*x + b.y*y + b.z*z + b.t*t; }
  i4_1x4_vector& operator*=(i4_float b)
  {
    x *= b;
    y *= b;
    z *= b;
    t *= b;
    return *this;
  }
};

class i4_bspline_basis : public i4_4x4_matrix_class
{
public:
  i4_bspline_basis()
  {
    sw16 b[16]= { -1,3,-3,1,3,-6,3,0,-3,0,3,0,1,4,1,0};
    for (w32 i=0; i<16; i++)
      elt[i]=(i4_float)b[i];    
  }

  void transform(i4_1x4_vector &b, i4_1x4_vector &result)
  {
    i4_float vx,vy,vz,vt;
    vx=b.x;
    vy=b.y;
    vz=b.z;
    vt=b.t;
          
    result.x = vx*elt[0] + vy*elt[1] + vz*elt[2] + vt*elt[3];
    result.y = vx*elt[4] + vy*elt[5] + vz*elt[6] + vt*elt[7];
    result.z = vx*elt[8] + vy*elt[9] + vz*elt[10] + vt*elt[11];
    result.t = vx*elt[12] + vy*elt[13] + vz*elt[14] + vt*elt[15];
  }
  
};

void i4_spline_class::delete_selected()
{
  point *p,*last=0;
  for (p=points; p; )
  {
    if (p->selected)
    {
      if (last)
        last->next=p->next;
      else points=p->next;
      point *q=p;
      p=p->next;

      p_alloc->free(q);

      t_points--;
    }
    else 
    {      
      last=p;
      p=p->next;
    }
  }
}

i4_spline_class::i4_spline_class()
{
  if (!p_alloc)
    p_alloc=new i4_linear_allocator(sizeof(point), 512, 512, "spline point allocator");

  point_refs++;
  points=0;
  t_points=0;
}

void i4_spline_class::cleanup()
{
  while (points)
  {
    point *q=points;
    points=points->next;

    t_points--;
    p_alloc->free(q);
  }
}

i4_spline_class::~i4_spline_class()
{
  cleanup();

  point_refs--;
  if (point_refs==0)
  {
    delete p_alloc;
    p_alloc=0;
  }
}

i4_spline_class::point *i4_spline_class::add_control_point(i4_float x, 
                                                           i4_float y, 
                                                           i4_float z, 
                                                           w32 frame)
{
  point *p=points,*last=0;
  while (p && p->frame<=frame)
  {
    last=p;
    p=p->next;
  }

  if (last && last->frame==frame)      // you can't add two different points at the same time
  {
    last->x=x;
    last->y=y;
    last->z=z;
    return last;
  }


  point *n=(point *)p_alloc->alloc();
  n->x=x;
  n->y=y;
  n->z=z;
  n->frame=frame;
  n->selected=i4_F;

  if (last)
  {
    n->next=last->next;
    last->next=n;
  }
  else
  {
    n->next=points;
    points=n;
  }  
  t_points++;

  return n;
}

i4_spline_class::point *i4_spline_class::get_control_point(w32 p)
{
  point *q=points;
  while (p && q)
  {
    p--;
    q=q->next;
  }
  return q;
}

i4_bool i4_spline_class::get_point(w32 frame, i4_float &x, i4_float &y, i4_float &z)
{
  if (!points || points->frame>frame)
    return i4_F;
  
  if (!points->next)
  {
    x=points->x;
    y=points->y;
    z=points->z;
    return i4_T;
  }

  point *l1=points, *l2=points, *l3=points, *l4=points;
  while (l4->next && l2->frame<=frame)
  {
    l4=l3;
    l3=l2;
    l2=l1;
    if (l1->next)
      l1=l1->next;
  }

  i4_bspline_basis b;
  i4_float u;

  if (l2->frame==l3->frame)
    u=0;
  else
    u=(frame-l3->frame)/(i4_float)(l2->frame-l3->frame);

  i4_1x4_vector px=i4_1x4_vector(l4->x, l3->x, l2->x, l1->x),rx;
  i4_1x4_vector py=i4_1x4_vector(l4->y, l3->y, l2->y, l1->y),ry;
  i4_1x4_vector pz=i4_1x4_vector(l4->z, l3->z, l2->z, l1->z),rz;
  
  i4_1x4_vector u_vector=i4_1x4_vector(u*u*u, u*u, u, 1);

  b.transform(px,rx);  
  b.transform(py,ry);  
  b.transform(pz,rz);  
  
  rx*=1/6.0;
  ry*=1/6.0;
  rz*=1/6.0;

  x=u_vector.dot(rx);
  y=u_vector.dot(ry);
  z=u_vector.dot(rz);

  return i4_T;
}


w32 i4_spline_class::last_frame()
{
  if (!points) return 0;
  point *p;

  for (p=points; p->next; p=p->next);
  return p->frame;
}


void i4_spline_class::insert_control_points()
{
  for (point *p=points; p; p=p->next)
  {
    if (p->selected && p->next && p->next->frame>p->frame+1)
    {
      point *n=(point *)p_alloc->alloc();
      n->x=(p->x + p->next->x)/2.0;
      n->y=(p->y + p->next->y)/2.0;
      n->z=(p->z + p->next->z)/2.0;

      n->frame=(p->frame + p->next->frame)/2;
      n->selected=i4_F;
      n->next=p->next;
      p->next=n;
      p=p->next;
      t_points++;
    }
  }
}

i4_spline_class::point *i4_spline_class::get_control_point_previous_to_frame(w32 frame)
{
  point *p=points;
  for (; p && p->frame<frame; p=p->next);
  return p;
}


