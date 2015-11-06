/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

//{{{ 3D File Loader
//
//  Not final version
//
//$Id: load3d.hh,v 1.18 1998/05/01 22:33:45 oliy Exp $

#ifndef G1_LOAD3D_HH
#define G1_LOAD3D_HH

#include "memory/growheap.hh"
#include "string/string.hh"
#include "math/vector.hh"
#include "obj3d.hh"

class r1_texture_manager_class;
class r1_render_api_class;

class g1_quad_object_loader_class : public g1_base_object_loader_class
{
protected:
  g1_quad_object_loader_class() : heap(0) {}
public:
  i4_grow_heap_class *heap;
  r1_texture_manager_class *tman;
  const i4_const_str *error_name;

  g1_quad_object_loader_class(i4_grow_heap_class *_heap) : heap(_heap) {}

  virtual g1_quad_object_class *allocate_object();

  virtual void set_num_vertex(w16 num_vertex);

  virtual void set_num_animations(w16 anims);
  virtual void create_animation(w16 anim, const i4_const_str &name, w16 frames);

  virtual void create_vertex(w16 anim, w16 frame, w16 index, const i4_3d_vector& v);
  virtual void store_vertex_normal(w16 anim, w16 frame, w16 index, const i4_3d_vector& normal);

  virtual void set_num_quads(w16 num_quads);
  virtual void create_quad(w16 quad, int verts, w16 *ref, w32 flags);
  virtual void store_texture_name(w32 quad, const i4_const_str &name);
  virtual void store_texture_params(w32 quad, i4_float scale, i4_float *u, i4_float *v);
  virtual void store_quad_normal(w16 quad, const i4_3d_vector& v);

  virtual void set_num_mount_points(w16 num_mounts);
  virtual void create_mount_point(w32 index, const i4_const_str &name, const i4_3d_vector &off);

  virtual void set_num_texture_animations(w16 num_textures);
  virtual void create_texture_animation(w32 index, w16 quad, w8 max_frames, w8 frames_x,
                                        i4_float du, i4_float dv, i4_float speed);
  virtual void create_texture_pan(w32 index, w16 quad,
                                  i4_float du, i4_float dv, i4_float speed);

  virtual void finish_object();

  g1_quad_object_class *load(i4_loader_class *fp, const i4_const_str &_error_name,
                             r1_texture_manager_class *_tman);
};

#endif
