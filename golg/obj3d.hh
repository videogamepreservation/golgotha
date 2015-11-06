/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef OBJ3D_HH
#define OBJ3D_HH

#include "arch.hh"
#include "math/point.hh"
#include "tex_id.hh"
#include "r1_vert.hh"
#include "g1_vert.hh"
#include "error/error.hh"

class g1_texture_animation
{
public:
  w16 quad_number;              // quad number in model
  w8 max_frames;                // frames of animation (0 == panning texture)
  w8 frames_x;                  // frames of animation in X direction
  i4_float speed;               // speed of animation in units/frame
  i4_float du,dv;               // total change in u & v in pan or animation
  i4_float u[4], v[4];          // base u,v coordinates from the polygon
};

class g1_quad_class
{
public:
  enum { MAX_VERTS=4 };

  typedef w16 vertex_ref_type;

  w32 max_verts() const { return MAX_VERTS; }
  w32 num_verts() const { return (vertex_ref[MAX_VERTS-1]==0xffff)? MAX_VERTS-1 : MAX_VERTS; }
  vertex_ref_type vertex_ref[MAX_VERTS];
  i4_float u[4],v[4], texture_scale;
  i4_3d_vector normal;

  r1_texture_handle material_ref;
  w32 flags;

  enum
  {
    TRANSLUCENCY = 0x1f,
    TINT =         (1<<5),
    SELECTED =     (1<<6),
    ON =           0xff
  };

  void set_flags(w8 mask, w8 value = ON) { flags = (flags&~mask) | (value&mask); }
  w8 get_flags(w8 mask) const { return flags&mask; }

  void set(w32 a, w32 b, w32 c, w32 d = 0xffff)
  //{{{
  { 
    vertex_ref[0] = (w16)a;
    vertex_ref[1] = (w16)b;
    vertex_ref[2] = (w16)c; 
    vertex_ref[3] = (w16)d;
  }
  //}}}

  void set_material(r1_texture_handle _material_ref)
  //{{{
  {
    material_ref=_material_ref;
  }
  //}}}
};

class g1_quad_object_class
{
public:
  class animation_class
  {
  public:
    w16 num_frames;
    g1_vert_class *vertex;        // num_vert * num_frame entries
  };
  w16 num_vertex;

  g1_quad_class   *quad;
  w16 num_quad;

  animation_class *animation;
  w16 num_animations;

  w32    *mount_id;
  i4_3d_vector    *mount;
  w16 num_mounts;

  g1_texture_animation *special;
  w16 num_special;

  i4_bool get_mount_point(char *name, i4_3d_vector& vect) const;

  g1_vert_class *get_verts(int anim, int frame)
  //{{{
  {
    return animation[anim].vertex + num_vertex * frame;
  }
  //}}}

  i4_float extent;

  void scale(i4_float value);
  void translate(i4_float xadd, i4_float yadd, i4_float zadd);

  void calc_extents(void);

  void init() { num_vertex=0; num_quad=0; num_mounts=0; num_animations=0; num_special=0; }
  ~g1_quad_object_class() {}

  i4_bool intersect(const i4_3d_vector &point,
                    const i4_3d_vector &ray,
                    int anim, int frame,
                    i4_float *t=0,
                    int *poly_hit=0,
                    i4_3d_vector *normal_hit=0);

  void update(i4_float frame);
};

class i4_loader_class;
class g1_base_object_loader_class
{
public:
  g1_quad_object_class *obj;

  virtual g1_quad_object_class *allocate_object() = 0;

  virtual void set_num_vertex(w16 num_vertex) = 0;

  virtual void set_num_animations(w16 anims) = 0;
  virtual void create_animation(w16 anim, const i4_const_str &name, w16 frames) = 0;

  virtual void create_vertex(w16 anim, w16 frame, w16 index, const i4_3d_vector& v) = 0;
  virtual void store_vertex_normal(w16 anim, w16 frame, w16 index, const i4_3d_vector& normal) {}

  virtual void set_num_quads(w16 num_quads) = 0;
  virtual void create_quad(w16 quad, int verts, w16 *ref, w32 flags) = 0;
  virtual void store_texture_name(w32 quad, const i4_const_str &name) {}
  virtual void store_texture_params(w32 quad, i4_float scale, i4_float *u, i4_float *v) {}
  virtual void store_quad_normal(w16 quad, const i4_3d_vector& v) {}

  virtual void set_num_mount_points(w16 num_mounts) {}
  virtual void create_mount_point(w32 index, const i4_const_str &name, const i4_3d_vector &off) {}

  virtual void set_num_texture_animations(w16 num_textures) {}
  virtual void create_texture_animation(w32 index, w16 quad, w8 max_frames, w8 frames_x,
                                        i4_float du, i4_float dv, i4_float speed) {}
  virtual void create_texture_pan(w32 index, w16 quad,
                                  i4_float du, i4_float dv, i4_float speed) {}

  virtual void finish_object() {}

  virtual g1_quad_object_class *load(i4_loader_class *fp);
};

#endif

//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}
