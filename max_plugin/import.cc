/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include <windows.h>
#include "crkutil.hh"
#include "obj3d.hh"
#include <utilapi.h>
#include <stdmat.h>
#include <stdio.h>
#include "file/file.hh"
#include "string/string.hh"
#include "max_object.hh"
#include "memory/array.hh"
#include "loaders/dir_load.hh"
#include "saver_id.hh"
#include "debug.hh"

#define G3DF_VERSION 3

#define MAXANIMATIONS 2
#define MAXFRAMES     300
#define MAXVERT       100
#define MAXTOTALVERT  (MAXFRAMES*MAXVERT)
#define MAXCOUNTS     10000
#define MAXQUAD       150

class m1_import_class : public SceneImport 
{
protected:
public:
  m1_import_class() {}
  ~m1_import_class() {}

  int ExtCount() { return 1; }
  const TCHAR * Ext(int n) { return _T("gmod"); }

  const TCHAR * LongDesc() { return _T("Imports Golgotha MODel files"); }
  const TCHAR * ShortDesc()  { return _T("Golgotha MODel files"); }
  const TCHAR * AuthorName() { return _T("Oliver Yu"); }
  const TCHAR * CopyrightMessage() { return _T("Copyright 1998 Crack dot Com"); }
  const TCHAR * OtherMessage1() { return _T(""); }
  const TCHAR * OtherMessage2() { return _T(""); }
  unsigned int Version() { return 10; }
  void ShowAbout(HWND hWnd) {}

  int DoImport(const TCHAR *name, ImpInterface *im, Interface *gi);
};

class m1_import_descriptor_class : public ClassDesc 
{
public:
  int           IsPublic()     {return 1;}
  void *        Create(BOOL loading = FALSE) {return new m1_import_class;}
  const TCHAR * ClassName()    {return _T("Crack Importer");}
  SClass_ID     SuperClassID() {return SCENE_IMPORT_CLASS_ID;}
  Class_ID      ClassID()      { return Class_ID(0xa2341, 0); }
  const TCHAR*  Category()     {return _T("");}
};
static m1_import_descriptor_class m1_import_descriptor;
ClassDesc* GetCrackImportDesc() {return &m1_import_descriptor;}

class m1_loader : public g1_base_object_loader_class
//{{{
{
public:
  ImpInterface *im;
  i4_loader_class *loader;
  i4_file_class *f;
  TriObject *obj;
  Mesh *mesh;
  ImpNode *node;
  const TCHAR *name;

  w16 quads;
  char texture_list[800][256];

  int faces;

  m1_loader() : f(0), loader(0), obj(0), mesh(0), node(0) {}
  
  ~m1_loader()
  {
    if (f) delete f;
    if (loader) delete loader;
    if (obj) delete obj;
    if (node) delete node;
  }
  
  virtual g1_quad_object_class *allocate_object() { return (g1_quad_object_class*)0xfaced; }

  virtual void set_num_vertex(w16 num_vertex)
  {
    dbg("Getting Verts\n");
    mesh->setNumVerts(num_vertex);
  }

  virtual void set_num_animations(w16 anims)
  {
    // no animation for now
  }

  virtual void create_animation(w16 anim, const i4_const_str &name, w16 frames)
  {
    // no animation for now
  }

  virtual void create_vertex(w16 anim, w16 frame, w16 index, const i4_3d_vector& v)
  {
    if (anim==0 && frame==0)
      mesh->setVert(index,v.x,v.y,v.z);
  }

  virtual void store_vertex_normal(w16 anim, w16 frame, w16 index, const i4_3d_vector& normal) {}

  virtual void set_num_quads(w16 num_quads)
  {
    dbg("Getting Textures & Quad\n");
    quads = num_quads;
    faces = 0;
    mesh->setNumFaces(quads*2);
  }

  virtual void create_quad(w16 quad, int verts, w16 *ref, w32 flags)
  {
    if (ref[3]!=0xffff)
    {
      mesh->faces[faces].setVerts(ref[0], ref[1], ref[2]);
      mesh->faces[faces].setEdgeVisFlags(1,1,0);
      mesh->faces[faces].setMatID((MtlID)quad+MAX_MATERIAL_OFFSET);
      faces++;
      
      mesh->faces[faces].setVerts(ref[2], ref[3], ref[0]);
      mesh->faces[faces].setEdgeVisFlags(1,1,0);
      mesh->faces[faces].setMatID((MtlID)quad+MAX_MATERIAL_OFFSET);
      faces++;
    }
    else if (ref[2]!=0xffff)
    {
      mesh->faces[faces].setVerts(ref[0], ref[1], ref[2]);
      mesh->faces[faces].setEdgeVisFlags(1,1,1);
      mesh->faces[faces].setMatID((MtlID)quad+MAX_MATERIAL_OFFSET);
      faces++;
    }
  }

  virtual void store_texture_name(w32 quad, const i4_const_str &name)
  {
    i4_os_string(name, texture_list[quad+1], sizeof(texture_list[0]));
  }

  virtual void store_texture_params(w32 quad, i4_float scale, i4_float *u, i4_float *v)
  {
    sprintf(&texture_list[quad+1][strlen(texture_list[quad+1])]," %f %f %f %f %f %f %f %f\r\n",
            u[0], v[0], u[1], v[1], u[2], v[2], u[3], v[3]);
  }

  virtual void store_quad_normal(w16 quad, const i4_3d_vector& v) {}

  virtual void set_num_mount_points(w16 num_mounts)
  {
    dbg("Getting Mounts\n");
  }
  virtual void create_mount_point(w32 index, const i4_const_str &name, const i4_3d_vector &off)
  {
    // create a point helper
    HelperObject *help = (HelperObject *)CreateInstance(HELPER_CLASS_ID, 
                                                        Class_ID(POINTHELP_CLASS_ID,0));
    ImpNode *node = im->CreateNode();
    
    if (help && node)
    {
      char tmp_name[1024];
      
      i4_os_string(name, tmp_name, sizeof(tmp_name));
      node->SetName(tmp_name);
      Matrix3 tm = TransMatrix(Point3(off.x,off.y,off.z));
      node->Reference(help);
      node->SetTransform(0,tm);
      im->AddNodeToScene(node);
      
      help=0;
      node=0;
    }
    
    if (help) delete help;
    if (node) delete node;
  }

  virtual void set_num_texture_animations(w16 num_textures)
  {
    dbg("Getting Texture Animations\n");
  }
  virtual void create_texture_animation(w32 index, w16 quad, w16 max_frames,
                                        i4_float du, i4_float dv, i4_float speed)
  {
    // none for now
  }
  virtual void create_texture_pan(w32 index, w16 quad,
                                  i4_float du, i4_float dv, i4_float speed)
  {
    // none for now
  }

  virtual void finish_object()
  {
    // set actual number of triangles
    mesh->setNumFaces(faces, TRUE);

    dbg("Building Mesh\n");
    mesh->buildNormals();
    mesh->buildBoundingBox();
    mesh->InvalidateEdgeList();

    dbg("Creating Node\n");
    node = im->CreateNode();
    if (!node) { g1_base_object_loader_class::obj=0; return; }

    Matrix3 tm;
    tm.IdentityMatrix();
    node->Reference(obj);
    node->SetTransform(0,tm);

    dbg("Building Texture List\n");
    char *buff = texture_list[0], *q=buff;
    sprintf(buff, "%s\r\n", gmod_sig);
    while (*q) q++;

    for (int i=0; i<quads; i++)
    {
      char *p = texture_list[i+1];
      while (*p)
        *q++ = *p++;
    }
    *q = 0;

    dbg("String copy\n");

    TSTR tst(buff);

    dbg("Assigning Texture List\n");

    node->GetINode()->SetUserPropBuffer(tst);

    dbg("Adding Node\n");
    im->AddNodeToScene(node);
    node->SetName(name);

    im->RedrawViews();
    unlink();
  }

  void unlink()
  {
    obj = 0;
    node = 0;
  }

  int load(const TCHAR *_name, ImpInterface *_im)
  {
    im = _im;
    name = _name;

    dbg("Opening %s\n", name);
    if (!(f = i4_open(name, I4_READ))) return 0;
    dbg("Opening Loader %s\n", name);
    if (!(loader = i4_open_save_file(f))) return 0;
    dbg("Creating Object\n");
    if (!(obj = CreateNewTriObject())) return 0;
    dbg("Getting Mesh\n");
    if (!(mesh = &obj->Mesh())) return 0;

    return (g1_base_object_loader_class::load(loader)!=0);
  }

};
//}}}

int m1_import_class::DoImport(const TCHAR *name, ImpInterface *im, Interface *gi)
{
  dbg.restart();

  dbg("Loading [%s]\n", name);

  m1_loader f;
  return f.load(name,im);
}

//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}
