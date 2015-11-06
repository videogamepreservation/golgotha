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
#include "maxcomm.hh"
#include "file/file.hh"
#include "string/string.hh"
#include "max_object.hh"
#include "memory/array.hh"
#include "debug.hh"

m1_mail_slot_class slot;

HINSTANCE my_instance;
int       my_nCmdShow;
 

#define G3DF_VERSION 3

#define MAXANIMATIONS 2
#define MAXFRAMES     300
#define MAXVERT       100
#define MAXTOTALVERT  (MAXFRAMES*MAXVERT)
#define MAXCOUNTS     10000
#define MAXQUAD       150

#define UTILTEST_CLASS_ID   0x99bb61a5

int m1_cur_trans=31;

class m1_utility_class : public UtilityObj 
{
protected:
  class property_modifier
  {
    class prop_list
    {
    public:
      int rotation,flip,reverse,trans,tint;

      prop_list() : rotation(0), flip(0), reverse(0), trans(31) {}
    };
  
  protected:
    void modify_node(INode *node);
  public:
    void modify_selected(Interface *ip);

    virtual void modify(prop_list &p) = 0;
  };
  
  class rotate_left_modifier : public property_modifier
  {
  public:
    virtual void modify(prop_list &p)
    {
      p.rotation = (p.rotation+11)%12;
    }
  };
  class rotate_right_modifier : public property_modifier
  {
  public:
    virtual void modify(prop_list &p)
    {
      p.rotation = (p.rotation+1)%12;
    }
  };
  class flip_normal_modifier : public property_modifier
  {
  public:
    virtual void modify(prop_list &p)
    {
      p.flip = !p.flip;
    }
  };
  class reverse_texture_modifier : public property_modifier
  {
  public:
    virtual void modify(prop_list &p)
    {
      p.reverse = !p.reverse;
    }
  };

  class tint_texture_modifier : public property_modifier
  {
  public:
    virtual void modify(prop_list &p)
    {
      p.tint = !p.tint;
    }
  };


  class set_trans_texture_modifier : public property_modifier
  {
  public:
    virtual void modify(prop_list &p)
    {
      p.trans=m1_cur_trans;
    }
  };
  
  char *set_g1_name(char *dest, char *name);

  int add_vert(i4_array<g1_vert_class> &vert_list,
               w32 vert_start,
               g1_vert_class &vt);
public:
  IUtil *iu;
  TimeValue time;
  Interface *ip;
  HWND hPanel;
  //  HWND hwnd;

  m1_poly_object_class *max_obj;

  m1_utility_class();
  void BeginEditParams(Interface *ip,IUtil *iu);
  void EndEditParams(Interface *ip,IUtil *iu);
  void DeleteThis() {}

  void Init(HWND hWnd);
  void Destroy(HWND hWnd);

  void create_objects();
  void grab_model_instance(int level, INode *node,
                           m1_poly_object_class *o,
                           w16 anim, w16 frame);
  void grab_model_instance2(int level, INode *node,
                            m1_poly_object_class *o,
                            w16 anim, w16 frame);

  void clean_objects();

  void restart_fly();
  void render();
  void rotate_selected_left()
  {
    rotate_left_modifier tmp;
    tmp.modify_selected(ip);
  }
  void rotate_selected_right()
  {
    rotate_right_modifier tmp;
    tmp.modify_selected(ip);
  }
  void tint_toggle()
  {
    tint_texture_modifier tmp;
    tmp.modify_selected(ip);
  }

  void flip_normal()
  {
    flip_normal_modifier tmp;
    tmp.modify_selected(ip);
  }
  void reverse_texture()
  {
    reverse_texture_modifier tmp;
    tmp.modify_selected(ip);
  }
  void set_trans()
  {
    if (m1_cur_trans>=1 && m1_cur_trans<=31)
    {      
      set_trans_texture_modifier tmp;
      tmp.modify_selected(ip);
    }
  }

};
static m1_utility_class m1_utility;

class m1_utility_descriptor_class : public ClassDesc 
{
public:
  int           IsPublic()     {return 1;}
  void *        Create(BOOL loading = FALSE) {return &m1_utility;}
  const TCHAR * ClassName()    {return _T("Crack Utilities");}
  SClass_ID     SuperClassID() {return UTILITY_CLASS_ID;}
  Class_ID      ClassID()      { return Class_ID(0x34713748, 0x5b4c1417); }
  const TCHAR*  Category()     {return _T("");}
};
static m1_utility_descriptor_class m1_utility_descriptor;
ClassDesc* GetCrackUtilDesc() {return &m1_utility_descriptor;}

i4_bool accel_disabled=i4_F;
HWND dlg_wnd;

static BOOL CALLBACK UtilTestDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch (msg) 
  {
    case WM_INITDIALOG:
      dlg_wnd=hWnd;
      m1_utility.Init(hWnd);      
      SetFocus(hWnd);
      return FALSE;
      break;
    
    case WM_DESTROY:
      m1_utility.Destroy(hWnd);
      break;

    case WM_KILLFOCUS :
    {
      if (accel_disabled)
      {
        accel_disabled=i4_F;
        EnableAccelerators(); 
      }
    } break;

    case WM_COMMAND:
      switch (LOWORD(wParam)) 
      {
        case 0x3f6 :  // not sure what command this is, but it works.. :) jc
        {
          if (!accel_disabled)
          {
            accel_disabled=i4_T;
            DisableAccelerators();  // turn 3ds's keyboard accelerators so we can type text
          }
          else
          {
            accel_disabled=i4_F;
            EnableAccelerators();  // turn on the accels 
          }

          return FALSE;
        } break;

        case IDC_CRKUTIL_APPLY :
        {
          char buf[100];
          GetDlgItemText(hWnd, IDC_CRKUTIL_EDIT, buf, 99);
          m1_cur_trans=atoi(buf);
          m1_utility.set_trans();
        } break;

        case IDC_CRACKUTIL_RENDER:
          m1_utility.render();
          break;


        case IDC_CRACKUTIL_ROTLEFT:
          m1_utility.rotate_selected_left();
          break;

        case IDC_CRACKUTIL_ROTRIGHT:
          m1_utility.rotate_selected_right();
          break;

        case IDC_CRACKUTIL_FLIP:
          m1_utility.flip_normal();
          break;

        case IDC_CRKUTIL_TINT:
          m1_utility.tint_toggle();
          break;

        case IDC_CRACKUTIL_REVERSE:
          m1_utility.reverse_texture();
          break;
      }
      break;

    case WM_LBUTTONDOWN :
    case WM_LBUTTONUP:
    case WM_MOUSEMOVE:
      m1_utility.ip->RollupMouseMessage(hWnd,msg,wParam,lParam); 
      break;

    default:
      return FALSE;
  } 
  return TRUE; 
}


m1_utility_class::m1_utility_class()
{
}


void m1_utility_class::BeginEditParams(Interface *_ip,IUtil *_iu)
{
  iu = _iu;
  ip = _ip;
  hPanel = ip->AddRollupPage(my_instance,
                             MAKEINTRESOURCE(IDD_CRACKUTIL),
                             (DLGPROC)UtilTestDlgProc,
                             _T("Golgotha Utilities"),
                             0);
}


void m1_utility_class::EndEditParams(Interface *_ip,IUtil *_iu)
{ 
  iu = NULL;
  ip = NULL;
  _ip->DeleteRollupPage(hPanel);
  hPanel = NULL;
}


void m1_utility_class::Init(HWND hWnd)
{
  max_obj = 0;

  initialize_i4();
}


void m1_utility_class::Destroy(HWND hWnd)
{
  //  clean_objects();
}


char *m1_utility_class::set_g1_name(char *dest, char *name)
{
  char s[256];
#if 0
  static char *remap[][2] = 
  { 
    {"f:/", "/u/"},
    {"//alpha1/", "/u/"},
    { 0, 0 }
  };
#endif

  char *p,*q;

  p = name;
  q = s;
  while (*p)
  {
    if (*p == '\\')
      *q = '/';
    else if (*p>='A' && *p<='Z')
      *q = *p + 'a' - 'A';
    else
      *q = *p;

    ++p;
    ++q;
  }
  *q = 0;

  p = s;
  q = dest;
#if 0
  for (int i=0; remap[i][0]; i++)
  {
    int len = strlen(remap[i][0]);

    if (strncmp(p, remap[i][0], len) == 0)
    {
      p += len;
      strcpy(q, remap[i][1]);
      q += strlen(q);
    }
  }
#endif

  strcpy(q,p);

  return dest;
}


int m1_utility_class::add_vert(i4_array<g1_vert_class> &vert_list,
                               w32 vert_start,
                               g1_vert_class &vt)
{
  float E_DELTA=0.001;
  for (int i=vert_start; i<vert_list.size(); i++)
    if (fabs(vt.v.x-vert_list[i].v.x)<E_DELTA &&
        fabs(vt.v.y-vert_list[i].v.y)<E_DELTA &&
        fabs(vt.v.z-vert_list[i].v.z)<E_DELTA)      
      return i;

  vert_list.add(vt);
  return vert_list.size()-1;
}

void m1_utility_class::grab_model_instance2(int level, 
                                            INode *node,
                                            m1_poly_object_class *max_obj,
                                            w16 anim, w16 frame)
{
  dbg("grab2\n");

  w32 vert_start = 0;

  int quad_offset = max_obj->quad_a.size();

  TSTR str;
  char *mat_name[1024], *mat_uv[1024];
  int mat_size=0;

  char *name = node->GetName();

  node->GetUserPropBuffer(str);
  char *p = str;

  if (strncmp(p, gmod_sig, strlen(gmod_sig))==0)
  {
    // skip first signature line
    while (*p && *p!='\n') p++;
    if (*p) p++;
    
    dbg("previously assigned polygon parameters\n");
    
    char *mat, *uv;
    while (*p)
    {
      max_obj->quad_a.add(0);
      mat = p+1;

      while (*p && *p!='\n' && *p!=' ') p++;
      if (*p==' ')
      {
        // tokenize all the texture names
        *p = 0;
        p++;
        uv = p;
      }
      while (*p && *p!='\n') p++;
      if (*p=='\n')
      {
        // tokenize UV stuff
        *p = 0;
        p++;
      }

      mat_name[mat_size] = mat;
      mat_uv[mat_size] = uv;
      mat_size++;
    }

    dbg("grabbed %d polygon materials\n", mat_size);
  }

  // Get base state of object
  Object *obj = node->EvalWorldState(0).obj;
  if (!obj)
  {
    // do nothing
  }
  else if (obj->ClassID() == Class_ID(POINTHELP_CLASS_ID,0))
  {
    Matrix3 tm = node->GetObjectTM(time);
    Point3 o(0,0,0),v;
    v = o * tm;

    max_obj->mount_a.add(new m1_mount_point_class(new i4_str(name), 
                                                  i4_3d_vector(v.x, v.y, v.z)));
  }
  else if (obj->CanConvertToType(triObjectClassID)) 
  {
    TriObject *tri = (TriObject *)obj->ConvertToType(time, triObjectClassID);
    Mesh &mesh = tri->mesh;
    Matrix3 tm = node->GetObjectTM(time);
    w16 vert_id[1024];
    int i,j;

    int verts = mesh.getNumVerts();
    // Add vertexes to object
    dbg("getting %d verts\n", verts);

    for(i=0; i<verts; ++i) 
    {
      Point3 v = mesh.verts[i] * tm;
      g1_vert_class vt;
      vt.v.x = v.x;
      vt.v.y = v.y;
      vt.v.z = v.z;
      vert_id[i] = add_vert(max_obj->anim_a[anim]->vertex_a, vert_start, vt);
    }
    
    int faces = mesh.getNumFaces();
    dbg("getting %d faces\n", faces);
    for (i=0; i<faces; i++)
    {
      Face face = mesh.faces[i];
      w16 tri[3];
      int found, textured=0;

      // find index of missing edge
      found = -1;
      for (j=0; j<3 && found<0; j++)
        if (!face.getEdgeVis(j))
          found = j;

      dbg("%s(%+1d) ", found>=0? "Quad":"Tri ", found);

      // order vertex from missing edge
      tri[0] = vert_id[face.v[(found+1)%3]];
      tri[1] = vert_id[face.v[(found+2)%3]];
      tri[2] = vert_id[face.v[(found+3)%3]];

      if (tri[0]==tri[1] || tri[0]==tri[2] || tri[1]==tri[2])
      {
        dbg("Malformed!\n");
        continue;
      }

      if (found>=0)
      {
        // this is part of a quad

        // search for matching triangle
        found=-1;
        for (j=quad_offset; j<max_obj->quad_a.size() && found<0; j++)
        {
          if (max_obj->quad_a[j] &&
              max_obj->quad_a[j]->vertex_ref[0] == tri[2] &&
              max_obj->quad_a[j]->vertex_ref[2] == tri[0])
            found = j;
        }

        if (found>=0)
        {
          m1_quad_class *q = max_obj->quad_a[found];

          dbg("Part of %d, added [%d]", found, tri[1]);
          // found other half.  add the remaining vertex
          q->vertex_ref[3] = tri[1];
          q->calc_texture_scale(max_obj->anim_a[anim]->vertex_a, vert_start);
        }
      }

      if (found<0)
      {
        dbg("Unallocated ");

        // need to add this triangle
        m1_quad_class *q = new m1_quad_class(tri[0], tri[1], tri[2]);

        dbg("Created [%d, %d, %d]", tri[0], tri[1], tri[2]);

        if (face.getMatID()>=MAX_MATERIAL_OFFSET)
        {
          // has a preassigned material id?
          found = face.getMatID() - MAX_MATERIAL_OFFSET;
          if (found>=mat_size)
          {
            dbg("Missing Material ");
            found = -1;
          }
          else
          {
            if (!max_obj->quad_a[found+quad_offset])
            {
              dbg("Preassigned at %d ", found);
              max_obj->quad_a[found+quad_offset] = q;
            }
            else
            {
              dbg("Repeated ");
              // a repeat id, assume the latter ones are wrong.
              //              found = -1;
            }
            
            q->set_texture(mat_name[found]);

            sscanf(mat_uv[found],"%f %f %f %f %f %f %f %f", 
                   &q->u[0],&q->v[0],&q->u[1],&q->v[1],&q->u[2],&q->v[2],&q->u[3],&q->v[3]);
            
            dbg(" = [%s] <%f,%f> <%f,%f> <%f,%f> <%f,%f>",
                q->texture_name, 
                q->u[0],q->v[0],q->u[1],q->v[1],q->u[2],q->v[2],q->u[3],q->v[3]);
            
            q->calc_texture_scale(max_obj->anim_a[anim]->vertex_a, vert_start);
            textured = 1;
          }
        }

        if (!textured)
        {
          q->set_texture("");
          q->u[0] = 0.0; q->v[0] = 0.0;
          q->u[1] = 1.0; q->v[1] = 0.0;
          q->u[2] = 1.0; q->v[2] = 1.0;
          q->u[3] = 0.0; q->v[3] = 1.0;
          q->calc_texture_scale(max_obj->anim_a[anim]->vertex_a,vert_start);
        }
        
        if (found<0)
        {
          dbg("New ");
          // add in a new quad
          found = max_obj->quad_a.add(q);
          dbg("at %d", found);
        }
      }
      dbg("\n");
    }

    // compact quad array
    i=quad_offset;
    while (i<max_obj->quad_a.size())
      if (max_obj->quad_a[i])
        i++;
      else
        max_obj->quad_a.remove(i);

    if (max_obj->quad_a.size()-quad_offset==1 && mat_size==0)
      // assume old style poly
      grab_model_instance(level, node, max_obj, anim, frame);

    dbg("Final Quads %d\n", max_obj->quad_a.size());

    // Delete the working object, if necessary
    if(obj != (Object *)tri)
      tri->DeleteThis();
  }

  // Dump children info
  for (int i=0; i<node->NumberOfChildren(); i++)
    grab_model_instance2(level+1, 
                         node->GetChildNode(i),
                         max_obj, anim, frame);

  dbg("Got the model!  Yes!\n");
}

void m1_utility_class::grab_model_instance(int level, 
                                           INode *node,
                                           m1_poly_object_class *max_obj,
                                           w16 anim, w16 frame)
{
  w32 vert_start = 0;

  char st[256],matname[256];

  memset(st,' ',level*2);
  st[level*2] = 0;
  strcat(st,node->GetName());
  dbg("%-40s", st);

  matname[0] = 0;

  // Get material
  Mtl *mat = node->GetMtl();

  if (mat)
  {
    // Check for a diffuse bitmap as real texture name
    if (mat->NumSubTexmaps())
    {
      Texmap *tex = mat->GetSubTexmap(ID_DI);

      if (tex && tex->ClassID()==Class_ID(BMTEX_CLASS_ID,0))
      {
        BitmapTex *bmt = (BitmapTex*)tex;
        set_g1_name(matname,bmt->GetMapName());
      }
    }
  }

  m1_quad_class *q = max_obj->quad_a[max_obj->quad_a.size()-1];

  int quad_off=0,flip=0,reverse=0,trans=31,tint=0;
  TSTR tst;
  
  node->GetUserPropBuffer(tst);
  sscanf(tst,"%d %d %d %d %d",&quad_off,&flip,&reverse,&trans,&tint);
  
  w16 quad[4];
  quad[0] = q->vertex_ref[0];
  quad[1] = q->vertex_ref[1];
  quad[2] = q->vertex_ref[2];
  quad[3] = q->vertex_ref[3];

  // Add quad to object
  if (q->num_verts()==4)
    if (!flip)
      q->set(quad[(0+quad_off)%4],quad[(3+quad_off)%4],quad[(2+quad_off)%4],quad[(1+quad_off)%4]);
    else
      q->set(quad[(0+quad_off)%4],quad[(1+quad_off)%4],quad[(2+quad_off)%4],quad[(3+quad_off)%4]);
  else
    if (!flip)
      q->set(quad[(0+quad_off)%3],quad[(2+quad_off)%3],quad[(1+quad_off)%3]);
    else
      q->set(quad[(0+quad_off)%3],quad[(1+quad_off)%3],quad[(2+quad_off)%3]);
  
  q->calc_texture(reverse,max_obj->anim_a[anim]->vertex_a,vert_start,matname);
  
  q->set_flags(g1_quad_class::TINT, tint?g1_quad_class::TINT:0);
  q->set_flags(g1_quad_class::TRANSLUCENCY, trans);
  
  dbg("\n");
}


void m1_utility_class::create_objects()
//Note:
//  Normally, root level groups are separate objects
//  For now, the whole scene is one object
{
  int i;

  dbg.restart();

  if (max_obj)
    delete max_obj;

  max_obj = new m1_poly_object_class;

  time = ip->GetTime();
  
  Interval interval = ip->GetAnimRange();
  
  for (time=interval.Start(); time<interval.End(); time+=160)
  {

    if (time==interval.Start())
    {              
      max_obj->anim_a.add(new m1_animation_class(i4gets("default_animation_name")));     
      grab_model_instance2(0,
                           ip->GetRootNode(), 
                           max_obj, 0 ,0);
      max_obj->num_vertex=max_obj->anim_a[0]->vertex_a.size();
    }
    else if (IsDlgButtonChecked(dlg_wnd, IDC_CRKUTIL_ANIM))
    {
      i4_array<m1_quad_class *> quad_list(100,100);

      grab_model_instance2(0,
                           ip->GetRootNode(), 
                           max_obj, 0, 0);

      for (i=0; i<quad_list.size(); i++)
        delete quad_list[i];
    }
  }
}


void m1_utility_class::clean_objects()
{
  if (max_obj)
    delete max_obj;
  dbg("Zapped\n");
  max_obj = 0;
}


void m1_utility_class::restart_fly()
{
  if (!slot.open(i4gets("slot_name")))
  {
//     PROCESS_INFORMATION piProcInfo; 
//     STARTUPINFO siStartInfo; 
    
//     /* Set up members of STARTUPINFO structure. */ 
    
//     siStartInfo.cb = sizeof(STARTUPINFO); 
//     siStartInfo.lpReserved = NULL; 
//     siStartInfo.lpReserved2 = NULL; 
//     siStartInfo.cbReserved2 = 0; 
//     siStartInfo.lpDesktop = NULL; 
//     siStartInfo.dwFlags = 0; 
    
//     /* Create the child process. */ 
    
//     CreateProcess(NULL, 
//                   "f:\\crack\\maxtool\\maxtool.exe",   /* command line                       */ 
//                   NULL,          /* process security attributes        */ 
//                   NULL,          /* primary thread security attributes */ 
//                   TRUE,          /* handles are inherited              */ 
//                   0,             /* creation flags                     */ 
//                   NULL,          /* use parent's environment           */ 
//                   NULL,          /* use parent's current directory     */ 
//                   &siStartInfo,  /* STARTUPINFO pointer                */ 
//                   &piProcInfo);  /* receives PROCESS_INFORMATION       */ 

//     Sleep(1000);

    slot.open(i4gets("slot_name"));
  }
}


void m1_utility_class::render()
{
  dbg("Create Objects\n");
  create_objects();
  if (max_obj->quad_a.size() && max_obj->num_vertex)
  {
    dbg("Save Objects\n");
    i4_file_class *f = i4_open(i4gets("fly_name"),I4_WRITE);
    i4_saver_class *g1_saver=new i4_saver_class(f);

    if (f)
    {
      max_obj->save(g1_saver);
      g1_saver->begin_data_write();
      max_obj->save(g1_saver);
      delete g1_saver;
    }

    if (!slot.initialized())
      restart_fly();

    char *name;

    if (slot.initialized())
    {
      dbg("Contacting maxtool\n");
      char buf[256];
      name = i4_os_string(i4gets("fly_name"),buf, sizeof(buf));
      if (slot.write(name, strlen(name) + 1) == 0)
      {
        restart_fly();
        name = i4_os_string(i4gets("fly_name"),buf, sizeof(buf));
        slot.write(name, strlen(name) + 1);
      }
    }
  }
  dbg("Cleaning up!\n");
  clean_objects();
  dbg("Done!\n");
}


void m1_utility_class::property_modifier::modify_selected(Interface *ip)
{
  for (int i=0; i<ip->GetSelNodeCount(); i++)
    modify_node(ip->GetSelNode(i));
}


void m1_utility_class::property_modifier::modify_node(INode *node)
{
  prop_list p;
  TSTR tst;
  char str[1024];

  node->GetUserPropBuffer(tst);
  sscanf(tst,"%d %d %d %d %d",&p.rotation, &p.flip, &p.reverse, &p.trans, &p.tint);
  
  modify(p);

  sprintf(str,"%d %d %d %d %d",p.rotation, p.flip, p.reverse, p.trans, p.tint);
  node->SetUserPropBuffer(TSTR(str));
}


