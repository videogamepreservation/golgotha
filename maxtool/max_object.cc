/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "max_object.hh"
#include "saver_id.hh"
#include "loaders/dir_save.hh"

// Save section

void m1_poly_object_class::save_quads(i4_saver_class *fp)
//{{{
{
  int i,j;

  fp->mark_section(G1_SECTION_MODEL_QUADS);

  int tq=num_quad;

  fp->write_16(tq);
  for (i=0; i<tq; i++)
  {
    for (j=0; j<quad[i].max_verts(); ++j)
    {
      fp->write_16(quad[i].vertex_ref[j]);
      fp->write_float(quad[i].u[j]);
      fp->write_float(quad[i].v[j]);
    }
    fp->write_float(quad[i].texture_scale);
    fp->write_16(quad[i].flags);

    fp->write_float(quad[i].normal.x);
    fp->write_float(quad[i].normal.y);
    fp->write_float(quad[i].normal.z);
  }
}
//}}}

void m1_poly_object_class::save_texture_names(i4_saver_class *fp)
//{{{
{
  fp->mark_section(G1_SECTION_MODEL_TEXTURE_NAMES);
  int tq=num_quad;
  fp->write_16(tq);
  for (int i=0; i<tq; i++)
    fp->write_counted_str(*texture_names[i]);
}
//}}}

void m1_poly_object_class::save_vert_animations(i4_saver_class *fp)
//{{{
{
  int i,j,k;

  fp->mark_section(G1_SECTION_MODEL_VERT_ANIMATION);

  fp->write_16(num_vertex);

  int ta=num_animations;

  fp->write_16(ta);
  for (i=0; i<ta; i++)
  {
    fp->write_counted_str(*animation_names[i]);
    int nf=animation[i].num_frames;
    fp->write_16(nf);

    for (j=0; j<nf; j++)
    {
      g1_vert_class *v=get_verts(i,j);

      for (k=0; k<num_vertex; k++)        
      {
        fp->write_float(v[k].v.x);
        fp->write_float(v[k].v.y);
        fp->write_float(v[k].v.z);

        fp->write_float(v[k].normal.x);
        fp->write_float(v[k].normal.y);
        fp->write_float(v[k].normal.z);
      }
    }
  }
}
//}}}

void m1_poly_object_class::save_mount_points(i4_saver_class *fp)
//{{{
{
  int i,j;

  if (num_mounts==0)
    return;

  fp->mark_section("GMOD Mounts");

  fp->write_16(num_mounts);
  for (i=0; i<num_mounts; i++)
  {
    fp->write_counted_str(*mount_names[i]);
    
    fp->write_float(mount[i].x);
    fp->write_float(mount[i].y);
    fp->write_float(mount[i].z);
  }
}
//}}}

void m1_poly_object_class::save_specials(i4_saver_class *fp)
//{{{
{
  int i,j;

  if (num_special==0)
    return;

  fp->mark_section("GMOD Texture Animations");

  fp->write_16(num_special);
  for (i=0; i<num_special; i++)
  {
    fp->write_format("211fff",
                     &special[i].quad_number,
                     &special[i].max_frames,
                     &special[i].frames_x,
                     &special[i].speed,
                     &special[i].du,
                     &special[i].dv);
  }
}
//}}}

void m1_poly_object_class::calc_quad_normal(g1_vert_class *v,
                                            g1_quad_class &q)
//{{{
{
  // find the surface normal used in lighting
  i4_3d_point_class p[4];

  for (int i=0; i<q.num_verts();  i++)
    p[i]=v[q.vertex_ref[i]].v;

  p[1] -= p[0];
  p[2] -= p[0];

  q.set_flags(INVALID_QUAD,0);
  if (q.num_verts()==3)
    q.normal.cross(p[1], p[2]);
  else
  {
    i4_3d_vector normal1, normal2;

    p[3] -= p[0];
    normal1.cross(p[1], p[2]);
    normal2.cross(p[2], p[3]);

    i4_float len1 = normal1.length();
    i4_float len2 = normal2.length();

    if (len1>len2)
      q.normal = normal1;
    else
      q.normal = normal2;

    if (len1==0.0 || len2==0.0)
      // bad normal lengths
      q.set_flags(INVALID_QUAD);
    else
    {
      normal1 /= len1;
      normal2 /= len2;

      if (normal1.dot(normal2)<0.5)
        q.set_flags(INVALID_QUAD);
    }
  }

  if (q.normal.x==0 && q.normal.y==0 && q.normal.z==0)
  {
    // 0 check - invalid polygon!
    q.normal=i4_3d_vector(0,0,1);
    q.set_flags(INVALID_QUAD);
    // i4_warning("very invalid polygon detected!");
  }
  else
    q.normal.normalize();
}
//}}}

void m1_poly_object_class::calc_vert_normals()
//{{{
{
  for (int a=0; a<num_animations; a++)
  {
    // calculate normal for each face
    for (w32 i=0; i<num_quad; i++)
      calc_quad_normal(get_verts(0,0), quad[i]);

    for (w32 v=0; v<num_vertex; v++)
    {
      w32 t=0;
      i4_3d_vector sum=i4_3d_vector(0,0,0);

      for (w32 j=0; j<num_quad; j++)
      {             
        if (quad[j].vertex_ref[0]==v ||
            quad[j].vertex_ref[1]==v ||
            quad[j].vertex_ref[2]==v ||
            quad[j].vertex_ref[3]==v)
        {
          t++;
          sum+=quad[j].normal;
        }      
      }

      if (sum.x==0 && sum.y==0 && sum.z==0)
        sum.set(0,0,1);
      else
        sum.normalize();
      (get_verts(a,0)+v)->normal=sum;
    }
  }
}
//}}}

void m1_poly_object_class::save(i4_saver_class *fp)
//{{{
{
  int i,j;
  calc_vert_normals();

  save_texture_names(fp);
  save_quads(fp);
  save_vert_animations(fp);
  save_mount_points(fp);
  save_specials(fp);
}
//}}}

void m1_poly_object_class::add_frame(w32 anim, w32 frame)
//{{{
{
  i4_error("write me!");
}
//}}}

void m1_poly_object_class::remove_frame(w32 anim, w32 frame)
//{{{
{
  i4_error("write me!");
}
//}}}

w32 m1_poly_object_class::add_vertex()
//{{{
{
  i4_error("write me!");
  return 0;
}
//}}}

void m1_poly_object_class::remove_vertex(w32 num)
//{{{
{
  i4_error("write me!");
}
//}}}

w32 m1_poly_object_class::add_quad()
//{{{
{
  g1_quad_class tmp;
  w32 ret=quad_store.add(tmp);
  texture_names.add(new i4_str(""));

  quad = &quad_store[0];
  num_quad = quad_store.size();

  return ret;
}
//}}}

void m1_poly_object_class::remove_quad(w32 num)
//{{{
{
  int i=0;

  for (i=num_special-1; i>=0; i--)
  {
    if (special[i].quad_number == num)
      remove_special(i);
    else if (special[i].quad_number > num)
      special[i].quad_number--;
  }
  
  quad_store.remove(num);
  texture_names.remove(num);
  num_quad = quad_store.size();
}
//}}}

w32 m1_poly_object_class::add_mount()
//{{{
{
  w32 ret=mount_store.add(i4_3d_vector(0,0,0));
  mount_id_store.add(0);
  mount_names.add(new i4_str(""));

  mount = &mount_store[0];
  num_mounts = mount_store.size();

  return ret;
}
//}}}

void m1_poly_object_class::remove_mount(w32 num)
//{{{
{
  int i=0;

  mount_store.remove(num);
  mount_id_store.remove(num);
  mount_names.remove(num);
  num_mounts = mount_store.size();
}
//}}}

w32 m1_poly_object_class::add_special(w32 quad_number)
//{{{
{
  for (int i=0; i<num_special; i++)
    if (special[i].quad_number==quad_number)
      return i;

  g1_texture_animation tmp;
  w32 ret=special_store.add(tmp);

  special = &special_store[0];
  num_special = special_store.size();

  return ret;
}
//}}}

void m1_poly_object_class::remove_special(w32 num)
//{{{
{
  int i=0;

  special_store.remove(num);
  num_special = quad_store.size();
}
//}}}

void m1_poly_object_class::calc_texture_scales()
{
  g1_vert_class *src_vert = get_verts(0,0);

  for (int i=0; i<num_quad; i++)
  {
    i4_3d_point_class *p1=&src_vert[quad[i].vertex_ref[0]].v;
    i4_3d_point_class *p2=&src_vert[quad[i].vertex_ref[1]].v;


    float edge_len=sqrt((p1->z-p2->z)*(p1->z-p2->z)+
                        (p1->y-p2->y)*(p1->y-p2->y)+
                        (p1->x-p2->x)*(p1->x-p2->x));

    float uv_length = sqrt ((quad[i].u[1]-quad[i].u[0]) * (quad[i].u[1]-quad[i].u[0]) +
                            (quad[i].v[1]-quad[i].v[0]) * (quad[i].v[1]-quad[i].v[0]));
    

    quad[i].texture_scale=edge_len / uv_length;
      
  }

}


//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}
