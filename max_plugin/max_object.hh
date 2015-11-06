/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef M1_MAX_OBJECT_HH
#define M1_MAX_OBJECT_HH

#include "obj3d.hh"
#include "load3d.hh"
#include "memory/array.hh"
#include "loaders/dir_save.hh"

class i4_grow_heap_class;
extern i4_grow_heap_class *m1_object_heap;

// derive off of g1 quad so we can store the string texture name name
class m1_quad_class : public g1_quad_class
{
public:
  char texture_name[256];
  m1_quad_class() {}
  m1_quad_class(w16 a, w16 b, w16 c, w16 d=0xffff) { set(a,b,c,d); flags = 0; }

  i4_bool set(w16 a, w16 b, w16 c, w16 d=0xffff);

  void set_texture(char *name);
  void calc_texture(int reverse, 
                    i4_array<g1_vert_class> &verts, w16 vert_off,
                    char *texture_name);

  void calc_texture_scale(i4_array<g1_vert_class> &verts, w16 vert_off);
}; 
  


// derive off of g1 animation so we can store the string animation name
class m1_animation_class : public g1_quad_object_class::animation_class
{
public:
  i4_str *animation_name;
  i4_array<g1_vert_class>                     vertex_a;

  void set_name(const i4_const_str &animation_n)
  {
    if (animation_name) delete animation_name;
    animation_name=new i4_str(animation_n);
  }

  m1_animation_class(const i4_const_str &name) 
    : animation_name(new i4_str(name)),
      vertex_a(1,20) {}

  m1_animation_class() 
    : animation_name(0),
      vertex_a(1,20) {}

  ~m1_animation_class() 
  {
    if (animation_name)
      delete animation_name; 
  }
};

class m1_mount_point_class
{
public:
  i4_str *name;
  i4_3d_vector offset;

  m1_mount_point_class(i4_str *name, const i4_3d_vector &offset): name(name), offset(offset) {}

  ~m1_mount_point_class()
  {
    if (name)
      delete name; 
  }
};

class m1_poly_object_class : public g1_quad_object_class
{
  void save_quads(i4_saver_class *fp);
  void save_texture_names(i4_saver_class *fp);
  void save_vert_animations(i4_saver_class *fp);
  void save_mount_points(i4_saver_class *fp);
  
  void calc_quad_normal(i4_array<g1_vert_class> &v, 
                       m1_quad_class &q);
  void calc_vert_normals();
public:
  i4_array<m1_quad_class *>        quad_a;
  i4_array<m1_animation_class *>   anim_a;
  i4_array<m1_mount_point_class *> mount_a;

  void cleanup();
  void save(i4_saver_class *fp);

  m1_poly_object_class() : quad_a(256,10), anim_a(256,100), mount_a(10,10) {}
  ~m1_poly_object_class() { cleanup(); }
};

#endif

