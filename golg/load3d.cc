/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "r1_api.hh"
#include "tmanage.hh"
#include "obj3d.hh"
#include "file/file.hh"
#include "error/error.hh"
#include "error/alert.hh"
#include "load3d.hh"
#include "math/num_type.hh"
#include "loaders/dir_load.hh"
#include "string/str_checksum.hh"
#include "saver_id.hh"

#define G3DF_VERSION 3

g1_quad_object_class *g1_quad_object_loader_class::allocate_object()
{
  g1_quad_object_class *o =(g1_quad_object_class*)heap->malloc(sizeof(g1_quad_object_class),
                                                               "quad_object");

  if (o) o->init();

  return o;
}

void g1_quad_object_loader_class::set_num_vertex(w16 num_vertex)
{
  obj->num_vertex=num_vertex;
}

void g1_quad_object_loader_class::set_num_animations(w16 anims)
{
  obj->num_animations=anims;

  int size=sizeof(g1_quad_object_class::animation_class) * anims;
  obj->animation=(g1_quad_object_class::animation_class *)heap->malloc(size, "animation");
}

void g1_quad_object_loader_class::create_animation(w16 anim, const i4_const_str &name, w16 frames)
{
  obj->animation[anim].num_frames=frames;
  obj->animation[anim].vertex=
    (g1_vert_class *)heap->malloc(sizeof(g1_vert_class)*frames*obj->num_vertex, "num frames");
}

void g1_quad_object_loader_class::create_vertex(w16 anim, w16 frame, 
                                                w16 index, const i4_3d_vector& v)
{
  g1_vert_class *vert=obj->get_verts(anim,frame)+index;
  vert->v = v;
}

void g1_quad_object_loader_class::store_vertex_normal(w16 anim, w16 frame, w16 index, 
                                               const i4_3d_vector& normal)
{
  g1_vert_class *vert=obj->get_verts(anim,frame)+index;
  vert->normal = normal;
}

void g1_quad_object_loader_class::set_num_quads(w16 quads)
{
  obj->num_quad = quads;
  obj->quad=(g1_quad_class *)heap->malloc(sizeof(g1_quad_class) * quads, "obj_quads");
}

void g1_quad_object_loader_class::create_quad(w16 quad, int verts, w16 *ref, w32 flags)
{
  g1_quad_class *q = &obj->quad[quad];

  q->set(ref[0], ref[1], ref[2], (verts>3)?ref[3]:0xffff);
  q->flags = flags;
}

void g1_quad_object_loader_class::store_texture_name(w32 quad, const i4_const_str &name)
{
  g1_quad_class *q = &obj->quad[quad];

  if (name.null())
  {
    i4_alert(i4gets("no_texture"), 100, error_name);
    q->material_ref = 0;
  }
  else
    q->material_ref = tman->register_texture(name, *error_name);
}

void g1_quad_object_loader_class::store_texture_params(w32 quad, i4_float scale, 
                                                       i4_float *u, i4_float *v)
{
  g1_quad_class *q = &obj->quad[quad];

  q->texture_scale = scale;
  q->u[0] = u[0];
  q->u[1] = u[1];
  q->u[2] = u[2];
  q->u[3] = u[3];
  q->v[0] = v[0];
  q->v[1] = v[1];
  q->v[2] = v[2];
  q->v[3] = v[3];
}

void g1_quad_object_loader_class::store_quad_normal(w16 quad, const i4_3d_vector& normal)
{
  g1_quad_class *q = &obj->quad[quad];

  q->normal = normal;
}

void g1_quad_object_loader_class::set_num_mount_points(w16 mounts)
{
  obj->num_mounts = mounts;
  obj->mount_id = (w32 *)heap->malloc(sizeof(w32) * mounts, "mount_ids");
  obj->mount = (i4_3d_vector *)heap->malloc(sizeof(i4_3d_vector) * mounts, "mount_vectors");
}

void g1_quad_object_loader_class::create_mount_point(w32 index, const i4_const_str &name, 
                                                     const i4_3d_vector &off)
{
  obj->mount[index] = off;
  obj->mount_id[index] = i4_str_checksum(name);
}

void g1_quad_object_loader_class::set_num_texture_animations(w16 num_textures)
{
  obj->num_special = num_textures;
  obj->special = (g1_texture_animation *)heap
    ->malloc(sizeof(g1_texture_animation) * num_textures, "texture_animations");
}

void g1_quad_object_loader_class::create_texture_animation(w32 index, w16 quad, 
                                                           w8 max_frames,
                                                           w8 frames_x,
                                                           i4_float du, i4_float dv, 
                                                           i4_float speed)
{
  g1_texture_animation *p = &obj->special[index];
  p->quad_number = quad;
  p->max_frames = max_frames;
  p->frames_x = frames_x;
  p->du = du;
  p->dv = dv;
  p->speed = speed;

  g1_quad_class *q = &obj->quad[quad];
  for (int i=0; i<q->num_verts(); i++)
  {
    p->u[i] = q->u[i];
    p->v[i] = q->v[i];
  }
}

void g1_quad_object_loader_class::create_texture_pan(w32 index, w16 quad,
                        i4_float du, i4_float dv, i4_float speed)
{
  g1_texture_animation *p = &obj->special[index];
  p->quad_number = quad;
  p->max_frames = 0;
  p->frames_x = 0;
  p->du = du;
  p->dv = dv;
  p->speed = speed;

  g1_quad_class *q = &obj->quad[quad];
  for (int i=0; i<q->num_verts(); i++)
  {
    p->u[i] = q->u[i];
    p->v[i] = q->v[i];
  }
}

void g1_quad_object_loader_class::finish_object()
{
  obj->calc_extents();
}

g1_quad_object_class *g1_quad_object_loader_class::load(i4_loader_class *fp, 
                                                        const i4_const_str &_error_name,
                                                        r1_texture_manager_class *_tman)
{
  tman = _tman;
  error_name = &_error_name;

  return g1_base_object_loader_class::load(fp);
}
