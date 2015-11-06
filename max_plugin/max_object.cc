/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "max_object.hh"
#include "file/file.hh"
#include "loaders/dir_save.hh"
#include "saver_id.hh"
#include "debug.hh"

#define G3DF_VERSION 3

w32 current_anim=0, current_frame=0;       // these reference model1
m1_poly_object_class *current_model_1=0, *current_model_2=0;

i4_grow_heap_class *m1_object_heap=0;


i4_bool m1_quad_class::set(w16 a, w16 b, w16 c, w16 d)
{
  // check for degenerate cases
  if (c==d || b==d || a==d ) { d=0xffff; }
  else if (b==c || a==c)     { c=d; d=0xffff; }
  else if (a==b)             { b=c; c=d; d=0xffff; }

  g1_quad_class::set(a,b,c,d);

  return (c!=0xffff);
}

void m1_quad_class::calc_texture(int reverse, 
                                 i4_array<g1_vert_class> &verts, w16 vert_off,
                                 char *tname)
{
  i4_float 
    tmp_u[4], tmp_v[4], 
    max_val, min_val,
    scale, translate;
  int p;

  strcpy(texture_name, tname);

  // determine quadgon texture space coordinate system
  i4_3d_point_class i,j,k;

  i = verts[vertex_ref[1]+vert_off].v;
  i -= verts[vertex_ref[0]+vert_off].v;
  i.normalize();

  j = verts[vertex_ref[num_verts()-1]+vert_off].v;
  j -= verts[vertex_ref[0]+vert_off].v;
  k.cross(i,j);
  k.normalize();

  j.cross(k,i);

  // find quadgon vertex coordinates in texture space
  for (p=0; p<num_verts(); p++)
  {
    tmp_u[p] = verts[vertex_ref[p]+vert_off].v.dot( i );
    tmp_v[p] = verts[vertex_ref[p]+vert_off].v.dot( j );
  }

  // scale u values to fit in interval [0,1]
  max_val = min_val = tmp_u[0];
  for (p=1; p<num_verts(); p++)
  {
    max_val = (tmp_u[p] > max_val) ? tmp_u[p] : max_val;
    min_val = (tmp_u[p] < min_val) ? tmp_u[p] : min_val;
  }

  texture_scale = max_val - min_val;

  scale = 1.0/texture_scale;
  translate = -min_val;

  if (!reverse)
    for (p=0; p<num_verts(); p++)
      u[p] = (tmp_u[p]+translate)*scale;
  else
    for (p=0; p<num_verts(); p++)
      u[p] = 1.0 - (tmp_u[p]+translate)*scale;

  // scale v values to fit in interval [0,1]
  max_val = min_val = tmp_v[0];
  for (p=1; p<num_verts(); p++)
  {
    max_val = (tmp_v[p] > max_val) ? tmp_v[p] : max_val;
    min_val = (tmp_v[p] < min_val) ? tmp_v[p] : min_val;
  }

  scale = 1.0/(max_val - min_val);
  translate = -min_val;
    
  for (p=0; p<num_verts(); p++)
    v[p] = 1.0 - (tmp_v[p]+translate)*scale;
}

void m1_quad_class::set_texture(char *tname)
{
  strcpy(texture_name, tname);
}

void m1_quad_class::calc_texture_scale(i4_array<g1_vert_class> &verts, w16 vert_off)
{
  i4_float 
    tmp_u[4], tmp_v[4], 
    max_val, min_val,
    scale, translate;
  int p;

  // determine quadgon texture space coordinate system
  i4_3d_point_class i,j,k;

  i = verts[vertex_ref[1]+vert_off].v;
  i -= verts[vertex_ref[0]+vert_off].v;
  i.normalize();

  j = verts[vertex_ref[num_verts()-1]+vert_off].v;
  j -= verts[vertex_ref[0]+vert_off].v;
  k.cross(i,j);
  k.normalize();

  j.cross(k,i);

  // find quadgon vertex coordinates in texture space
  for (p=0; p<num_verts(); p++)
  {
    tmp_u[p] = verts[vertex_ref[p]+vert_off].v.dot( i );
    tmp_v[p] = verts[vertex_ref[p]+vert_off].v.dot( j );
  }

  // scale u values to fit in interval [0,1]
  max_val = min_val = tmp_u[0];
  for (p=1; p<num_verts(); p++)
  {
    max_val = (tmp_u[p] > max_val) ? tmp_u[p] : max_val;
    min_val = (tmp_u[p] < min_val) ? tmp_u[p] : min_val;
  }

  texture_scale = max_val - min_val;
}


void m1_poly_object_class::save_quads(i4_saver_class *fp)
{
  int i,j;

  fp->mark_section(G1_SECTION_MODEL_QUADS);

  fp->write_16(quad_a.size());
  for (i=0; i<quad_a.size(); i++)
  {
    for (j=0; j<quad[i].max_verts(); ++j)
    {
      fp->write_16(quad_a[i]->vertex_ref[j]);
      fp->write_float(quad_a[i]->u[j]);
      fp->write_float(quad_a[i]->v[j]);
    }
    fp->write_float(quad_a[i]->texture_scale);
    fp->write_16(quad_a[i]->flags);

    fp->write_float(quad_a[i]->normal.x);
    fp->write_float(quad_a[i]->normal.y);
    fp->write_float(quad_a[i]->normal.z);
  }
}

void m1_poly_object_class::save_texture_names(i4_saver_class *fp)
{
  fp->mark_section(G1_SECTION_MODEL_TEXTURE_NAMES);

  fp->write_16(quad_a.size());
  for (int i=0; i<quad_a.size(); i++)
  {
    w16 len=strlen(quad_a[i]->texture_name);
    fp->write_16(len);
    fp->write(quad_a[i]->texture_name, len);
  }
}


void m1_poly_object_class::save_vert_animations(i4_saver_class *fp)
{
  int i,j;

  fp->mark_section(G1_SECTION_MODEL_VERT_ANIMATION);

  fp->write_16(num_vertex);

  fp->write_16(anim_a.size());
  for (i=0; i<anim_a.size(); i++)
  {
    fp->write_counted_str(*anim_a[i]->animation_name);
    
    fp->write_16(anim_a[i]->vertex_a.size() / num_vertex);

    for (j=0; j<anim_a[i]->vertex_a.size(); j++)
    {
      g1_vert_class *v=&anim_a[i]->vertex_a[j];

      fp->write_float(v->v.x);
      fp->write_float(v->v.y);
      fp->write_float(v->v.z);

      fp->write_float(v->normal.y);
      fp->write_float(v->normal.z);
      fp->write_float(v->normal.z);
    }
  }
}

void m1_poly_object_class::save_mount_points(i4_saver_class *fp)
{
  int i,j;

  fp->mark_section("GMOD Mounts");

  fp->write_16(mount_a.size());
  for (i=0; i<mount_a.size(); i++)
  {
    fp->write_counted_str(*mount_a[i]->name);
    
    fp->write_float(mount_a[i]->offset.x);
    fp->write_float(mount_a[i]->offset.y);
    fp->write_float(mount_a[i]->offset.z);
  }
}

void m1_poly_object_class::calc_quad_normal(i4_array<g1_vert_class> &v, 
                                            m1_quad_class &q)
{
  // find the surface normal used in lighting
  i4_3d_point_class p[4];

  for (int i=0; i<q.num_verts();  i++)
    p[i]=v[q.vertex_ref[i]].v;

  p[1] -= p[0];
  p[2] -= p[0];

  if (q.num_verts()==3)
    q.normal.cross(p[1], p[2]);
  else
  {
    i4_3d_vector normal1, normal2;

    p[3] -= p[0];
    normal1.cross(p[1], p[2]);
    normal2.cross(p[2], p[3]);

    if (normal1.length()>normal2.length())
      q.normal = normal1;
    else
      q.normal = normal2;
  }

  if (q.normal.x==0 && q.normal.y==0 && q.normal.z==0)
    q.normal=i4_3d_vector(0,0,1);
  else
    q.normal.normalize();
}

void m1_poly_object_class::calc_vert_normals()
{
  for (int a=0; a<anim_a.size(); a++)
  {
    // calculate normal for each face
    for (w32 i=0; i<quad_a.size(); i++)
      calc_quad_normal(anim_a[a]->vertex_a, *quad_a[i]);

    for (w32 v=0; v<num_vertex; v++)
    {
      w32 t=0;
      i4_3d_vector sum=i4_3d_vector(0,0,0);

      for (w32 j=0; j<quad_a.size(); j++)
      {             
        if (quad_a[j]->vertex_ref[0]==v ||
            quad_a[j]->vertex_ref[1]==v ||
            quad_a[j]->vertex_ref[2]==v ||
            quad_a[j]->vertex_ref[3]==v)
        {
          t++;
          sum+=quad_a[j]->normal;
        }      
      }

      if (sum.x!=0 || sum.y!=0 || sum.z!=0)
        sum.normalize();
      else
        sum.set(0,0,1);
      anim_a[a]->vertex_a[v].normal=sum;
    }
  }
}


void m1_poly_object_class::save(i4_saver_class *fp)
{
  int i,j;
  calc_vert_normals();

  dbg("saving textures...");
  save_texture_names(fp);
  dbg("saving quads...");
  save_quads(fp);
  dbg("saving verts...");
  save_vert_animations(fp);
  dbg("saving mount points...");
  save_mount_points(fp);
  dbg("done.\n");
}

void m1_poly_object_class::cleanup()
{
  w32 i;

  dbg("Zapping quads...");
  for (i=0; i<quad_a.size(); i++)
    delete quad_a[i];
  
  dbg("Zapping verts...");
  for (i=0; i<anim_a.size(); i++)
    delete anim_a[i];
  
  dbg("Final clearing...");
  quad_a.clear();
  anim_a.clear();

  dbg("Cleanup Done.\n");
}

