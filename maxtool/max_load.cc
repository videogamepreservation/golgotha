/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "max_load.hh"
#include "load3d.hh"
#include "loaders/dir_load.hh"
#include "tupdate.hh"
#include "m1_info.hh"
#include "saver_id.hh"
#include "error/alert.hh"

class m1_poly_object_loader_class : public g1_quad_object_loader_class
{
public:
  m1_poly_object_class *mobj() const { return (m1_poly_object_class*)obj; }

  virtual void set_num_animations(w16 anims)
  //{{{
  {
    mobj()->anim_store.add_many(anims);
    mobj()->animation_names.add_many(anims);

    mobj()->num_animations = mobj()->anim_store.size();
    mobj()->animation = &mobj()->anim_store[0];
  }
  //}}}

  virtual void create_animation(w16 anim, const i4_const_str &name, w16 frames)
  //{{{
  { 
    mobj()->animation[anim].num_frames=frames;
    mobj()->animation[anim].vertex=mobj()->vert_store.add_many(frames*mobj()->num_vertex);
    mobj()->animation_names[anim]=new i4_str(name);
  }
  //}}}
  
  virtual void set_num_quads(w16 num_quads)
  //{{{
  {
    mobj()->quad_store.add_many(num_quads);
    mobj()->texture_names.add_many(num_quads);
    mobj()->num_quad = mobj()->quad_store.size();
    mobj()->quad = &mobj()->quad_store[0];
  }
  //}}}

  virtual void create_quad(w16 quad, int verts, w16 *ref, w32 flags)
  //{{{
  {
    g1_quad_object_loader_class::create_quad(quad,verts,ref,flags);

    for (int i=1; i<verts; i++)
      if (ref[i]==0xffff)
        ref[i] = ref[i-1];
  }
  //}}}

  virtual void store_texture_name(w32 quad, const i4_const_str &name)
  //{{{
  {
    char buf[512], *p=buf;

    i4_os_string(name, buf, sizeof(buf));
    while (*p) p++;
    --p;
    if (p>buf && *p!='.') --p;
    if (p>buf && *p!='.') --p;
    if (p>buf && *p!='.') --p;

    if (*p=='.')
    {
      ++p;
      while (*p)
      {
        if (*p>='A' && *p<='Z')
          *p = *p - 'A' + 'a';
        p++;
      }
    }

    g1_quad_object_loader_class::store_texture_name(quad, buf);
    mobj()->texture_names[quad]=new i4_str(buf);
  }
  //}}}
  
  virtual void set_num_mount_points(w16 num_mounts)
  //{{{
  {
    mobj()->mount_store.add_many(num_mounts);
    mobj()->mount_id_store.add_many(num_mounts);
    mobj()->mount_names.add_many(num_mounts);

    mobj()->num_mounts = mobj()->mount_store.size();
    if (mobj()->num_mounts>0)
    {
      mobj()->mount_id = &mobj()->mount_id_store[0];
      mobj()->mount = &mobj()->mount_store[0];
    }
  }
  //}}}

  virtual void create_mount_point(w32 index, const i4_const_str &name, const i4_3d_vector &off)
  //{{{
  {
    g1_quad_object_loader_class::create_mount_point(index, name, off);
    mobj()->mount_names[index]=new i4_str(name);
  }
  //}}}

  virtual void set_num_texture_animations(w16 num_textures)
  //{{{
  {
    mobj()->special_store.add_many(num_textures);

    mobj()->num_special = mobj()->special_store.size();
    if (mobj()->num_special>0)
      mobj()->special = &mobj()->special_store[0];
  }
  //}}}

  virtual g1_quad_object_class *allocate_object()
  //{{{
  {
    m1_poly_object_class *o = new m1_poly_object_class;

    if (o) o->init();

    return o;
  }
  //}}}

  m1_poly_object_class *load(i4_loader_class *fp, const i4_const_str &_error_name,
                             r1_texture_manager_class *_tman)
  //{{{
  {
    error_name = &_error_name;
    tman = _tman;
    heap = 0;
    
    return (m1_poly_object_class *)g1_base_object_loader_class::load(fp);
  }
  //}}}
};

void m1_load_model_dir(const i4_const_str &object_filename,
                       r1_texture_manager_class *tman)
{
  tman->reset();
  m1_info.textures_loaded=i4_F;

  i4_array<i4_str *> tname_a(128,16);

  i4_file_class *fp=i4_open(object_filename);
  if (fp)
  {
    i4_loader_class *gfp=i4_open_save_file(fp);
    if (gfp)
    {
      if (gfp->goto_section(G1_SECTION_MODEL_TEXTURE_NAMES))
      {
        w16 quads=gfp->read_16();
        for (int i=0; i<quads; i++)
        {
          i4_str *s=gfp->read_counted_str();
          if (!s->null())
            tname_a.add(s); 
          else
            delete s;
        }
      }
      delete gfp;
    }
  }
  else i4_alert(i4gets("obj_missing"),100,&object_filename);


  for (int i=0; i<tname_a.size(); i++)
    delete tname_a[i];
}

m1_poly_object_class *m1_load_model(const i4_const_str &name,
                                    r1_texture_manager_class *tman)
//{{{
{
  m1_info.recalcing_textures=i4_T;
  m1_update_object_textures(name);

  tman->reset();
  m1_info.textures_loaded=i4_F;

  m1_load_model_dir(name, tman);
  m1_info.recalcing_textures=i4_F;


  i4_file_class *fp=i4_open(name);
  if (fp)
  {
    i4_loader_class *gfp=i4_open_save_file(fp);

    if (gfp)
    {
      m1_poly_object_class *o;
      m1_poly_object_loader_class mo_loader;
      o = mo_loader.load(gfp, name, tman);
      delete gfp;

      return o;    
    } 
  }
  return 0;
}
//}}}


//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}
