/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "map.hh"
#include "map_man.hh"
#include "map_vert.hh"
#include "map_cell.hh"
#include "div_table.cc"
#include "camera.hh"
#include "r1_api.hh"
#include "tmanage.hh"
#include "r1_clip.hh"
#include "g1_render.hh"
#include "tile.hh"
#include "controller.hh"
#include "statistics.hh"
#include "lisp/li_class.hh"
#include "resources.hh"
#include "time/profile.hh"
#include "light.hh"

static i4_profile_class pf_map_fast("map_fast"), 
  pf_calc_map_lod("calc_map_lod"),
  pf_gather_objects("gather_objects"),
  pf_draw_tri("draw_tri");


i4_image_class *render_map_section(int x1, int y1, int x2, int y2, int im_w, int im_h);

inline float fmin(float a, float b) { return (a<b)?a:b; }
inline float fmax(float a, float b) { return (a>b)?a:b; }

class texture_context
{
public:
  i4_float x1,y1;                                 // offset in the map
  i4_float sx, sy;                                // scale factors to get [0,1] texture coords.
  r1_texture_handle texture;                      // texture to use
};

class lod_node
{
public:
  w16 x1,y1,x2,y2;                                // bounding box for this LOD node
  i4_float z1,z2;
  i4_float metric;                                // metric for determining LOD "importance"
  lod_node *child[2];                             // orthogonal BSP children
  w8 texture_context;                             // which texture context to use or 0 for 1x1

  w8 flags;
  enum
  {
    CLIPPED    = 1<<0,                            // this node is completely out of the view
    IN_VIEW    = 1<<1,                            //                         in the view
    CLIP_FLAGS = CLIPPED | IN_VIEW,
    V_SPLIT    = 1<<2,                            // node split vertically
  };

  lod_node(w16 x1,w16 y1,w16 x2,w16 y2, i4_float metric=0)
    : x1(x1), y1(y1), x2(x2), y2(y2), metric(metric)
  {
    child[0] = 0;
    child[1] = 0;
  }
};

class g1_lod_context_class
{
public:
  // texture contexts for the large subdivided texture used to cover the whole map
  texture_context map_texture[5];
  int num_context, last_context;

  // LOD tree
  lod_node *root;

  // breadth first ordering of the lod_nodes
  lod_node *queue[150*100*2];
  int num_queued, front;
  
  // lod_nodes submitted for drawing
  lod_node *quad[150*100];
  int num_quads;

  // current controller
  g1_object_controller_class *cont;
  i4_3d_vector pos;

  g1_lod_context_class() : root(0) {}

  void init_lod()
  // make LOD tree
  {
    if (root)
      delete_lod_tree(root);

    num_context=0;
    make_lod_node(&root,0,0,0,g1_get_map()->width(),g1_get_map()->height());

    for (int i=1; i<=num_context; i++)
    {
      int x1,y1,x2,y2;

      x1 = i4_f_to_i(map_texture[i].x1);
      y1 = i4_f_to_i(map_texture[i].y1);
      x2 = x1 + i4_f_to_i(1.0/map_texture[i].sx + 0.5);
      y2 = y1 + i4_f_to_i(1.0/map_texture[i].sy + 0.5);
      i4_image_class *im = render_map_section(x1,y1,x2,y2, 256,256);
      map_texture[i].texture = g1_render.r_api->get_tmanager()->register_image(im);
      delete im;
    }
  }

  void uninit_lod()
  {
    delete_lod_tree(root);
  }

  void use_controller(g1_object_controller_class *_cont) { cont = _cont; }

  int test_clip(lod_node *p)
  // test bounding region of lod_node against the view frustrum
  {
    const i4_float MAX_FLY_HEIGHT=3.0;
    int clip = cont->test_clip(i4_3d_vector(p->x1,p->y1,p->z1), 
                               i4_3d_vector(p->x2,p->y2,p->z2+MAX_FLY_HEIGHT));

    w8 flags = 
      (clip<0)? lod_node::CLIPPED : 
      (clip>0)? lod_node::IN_VIEW :
      0;

    p->flags = (p->flags & ~lod_node::CLIP_FLAGS) | flags;

    return clip>=0;
  }

  void make_lod_node(lod_node **pp, lod_node *parent, int x1, int y1, int x2, int y2, int level=0)
  {
    int dx = x2-x1, dy = y2-y1;
    int mx = (x1+x2)/2, my = (y1+y2)/2;

    if (dx==0 || dy==0)
      return;

    lod_node *p;
    *pp = p = new lod_node(x1,y1,x2,y2);
    
    // hack to get texture contexts
    if (level<2)
      p->texture_context = 0;
    else if (level==2)
    {
      p->texture_context = ++num_context;
      map_texture[num_context].x1 = x1;
      map_texture[num_context].y1 = y1;
      map_texture[num_context].sx = 1.0*div_table[x2-x1];
      map_texture[num_context].sy = 1.0*div_table[y2-y1];
    }
    else
      p->texture_context = parent->texture_context;

    if (dx>1 || dy>1)
    {
      if (dx>=dy)
      {
        p->flags |= lod_node::V_SPLIT;
        make_lod_node(&p->child[0], p, x1,y1,mx,y2, level+1);
        make_lod_node(&p->child[1], p, mx,y1,x2,y2, level+1);
      }
      else
      {
        p->flags &= ~lod_node::V_SPLIT;
        make_lod_node(&p->child[0], p, x1,y1,x2,my, level+1);
        make_lod_node(&p->child[1], p, x1,my,x2,y2, level+1);
      }
      p->z1 = fmin(p->child[0]->z1, p->child[1]->z1);
      p->z2 = fmax(p->child[0]->z2, p->child[1]->z2);

      // area metric
      p->metric = p->child[0]->metric + p->child[1]->metric;
    }
    else
    {
      i4_float
        z11 = g1_get_map()->vertex(x1,y1)->get_height(),
        z21 = g1_get_map()->vertex(x2,y1)->get_height(),
        z12 = g1_get_map()->vertex(x1,y2)->get_height(),
        z22 = g1_get_map()->vertex(x2,y2)->get_height();
      
      p->z1 = p->z2 = z11;
      p->z1 = fmin(p->z1, z21);
      p->z2 = fmax(p->z2, z21);
      p->z1 = fmin(p->z1, z12);
      p->z2 = fmax(p->z2, z12);
      p->z1 = fmin(p->z1, z22);
      p->z2 = fmax(p->z2, z22);

      // area metric
      i4_float dz1,dz2;

      dz1 = z22 - z21;
      dz2 = z11 - z21;
      p->metric = sqrt(dz1*dz1 + dz2*dz2 + 1);
      dz1 = z22 - z12;
      dz2 = z11 - z12;
      p->metric += dz1*dz1 + dz2*dz2 + 1;
    }
  }

  void delete_lod_tree(lod_node *p)
  {
    if (!p)
      return;
    delete_lod_tree(p->child[0]);
    delete_lod_tree(p->child[1]);
    delete p;
  }

  void calculate_lod(lod_node *p)
  {
//     cont->view.get_camera_pos(pos);
    cont->get_pos(pos);

    num_quads=0;
    front = num_queued = 0;
    queue[num_queued++] = p;
    root->flags = 0;

    while (front<num_queued)
    {
      p = queue[front++];
      int x1=p->x1,y1=p->y1,x2=p->x2,y2=p->y2;
      w8 sub_flags=p->flags & lod_node::IN_VIEW;

      // do some kind of clip test
      if (!sub_flags)
        if (test_clip(p))
          sub_flags=p->flags & lod_node::IN_VIEW;
        else
          continue;

//       int dx = x2-x1, dy = y2-y1;
      int mx = (x1+x2)/2, my = (y1+y2)/2;

      i4_float fx = pos.x - mx, fy = pos.y - my, fz = pos.z - g1_get_vertex(mx,my)->get_height();
      i4_float d1 = fx*fx+fy*fy+fz*fz;
      i4_float d2 = p->metric*20;

      if (p->child[0] && d1<d2)
      {
        p->child[0]->flags = (p->child[0]->flags & ~lod_node::CLIP_FLAGS) | sub_flags;
        p->child[1]->flags = (p->child[1]->flags & ~lod_node::CLIP_FLAGS) | sub_flags;
        queue[num_queued++] = p->child[0];
        queue[num_queued++] = p->child[1];

        // count & mark t splits
        lod_node *q = p->child[0];
        if (p->flags & lod_node::V_SPLIT)
        {
          g1_get_vertex(q->x2,q->y1)->flags ^= g1_map_vertex_class::T_INTERSECTION;
          g1_get_vertex(q->x2,q->y2)->flags ^= g1_map_vertex_class::T_INTERSECTION;
        }
        else
        {
          g1_get_vertex(q->x1,q->y2)->flags ^= g1_map_vertex_class::T_INTERSECTION;
          g1_get_vertex(q->x2,q->y2)->flags ^= g1_map_vertex_class::T_INTERSECTION;
        }
      }
      else
      {
        g1_map_vertex_class *v11, *v12, *v21, *v22;
      
        v11 =  g1_get_vertex(x1,y1);
        v21 =  g1_get_vertex(x2,y1);
        v12 =  g1_get_vertex(x1,y2);
        v22 =  g1_get_vertex(x2,y2);

        v11->t_height = v11->get_height();
        v21->t_height = v21->get_height();
        v12->t_height = v12->get_height();
        v22->t_height = v22->get_height();
        quad[num_quads++] = p;
      }
    }

    // T intersection fixing

    // Map of directions
    //          dzdx2
    //    z22*  ---->  *
    //       ^         ^
    // dzdy1 |         | dzdy2
    //       |         |
    //    z12*  ---->  *
    //      z11 dzdx1 z21
    for (int test=0; test<2; test++)
      for (front = 0; front<num_quads; front++)
      {
        int 
          x1=quad[front]->x1,y1=quad[front]->y1,
          x2=quad[front]->x2,y2=quad[front]->y2;
        int i,j;
        i4_float z11,z12,z21,z22;
        i4_float dzdx1,dzdx2,dzdy1,dzdy2;

        z11 = g1_get_vertex(x1,y1)->t_height;
        z21 = g1_get_vertex(x2,y1)->t_height;
        z12 = g1_get_vertex(x1,y2)->t_height;
        z22 = g1_get_vertex(x2,y2)->t_height;

        dzdx1 = (z21 - z11)*div_table[x2 - x1];
        dzdx2 = (z22 - z12)*div_table[x2 - x1];
        dzdy1 = (z12 - z11)*div_table[y2 - y1];
        dzdy2 = (z22 - z21)*div_table[y2 - y1];

        z22 = z12;
        z12 = z11;
      
        i4_float adj1,adj2;

#define DEPIXEL_ADJ 0.1

        adj1 = (pos.x<x1)?DEPIXEL_ADJ:-DEPIXEL_ADJ;
        adj2 = (pos.x>x2)?DEPIXEL_ADJ:-DEPIXEL_ADJ;
        for (j=y1+1; j<y2; j++)
        {
          z11 += dzdy1;
          z21 += dzdy2;

//           if (g1_get_vertex(x1,j)->flags & g1_map_vertex_class::T_INTERSECTION)
            g1_get_vertex(x1,j)->t_height = z11 + adj1;
//           if (g1_get_vertex(x2,j)->flags & g1_map_vertex_class::T_INTERSECTION)
            g1_get_vertex(x2,j)->t_height = z21 + adj2;
        }
        
        adj1 = (pos.y<y1)?DEPIXEL_ADJ:-DEPIXEL_ADJ;
        adj2 = (pos.y>y2)?DEPIXEL_ADJ:-DEPIXEL_ADJ;
        for (i=x1+1; i<x2; i++)
        {
          z12 += dzdx1;
          z22 += dzdx2;
//           if (g1_get_vertex(i,y1)->flags & g1_map_vertex_class::T_INTERSECTION)
            g1_get_vertex(i,y1)->t_height = z12 + adj1;
//           if (g1_get_vertex(i,y2)->flags & g1_map_vertex_class::T_INTERSECTION)
            g1_get_vertex(i,y2)->t_height = z22 + adj2;
        }
      }
  }

} g1_lod;

void g1_map_class::init_lod()
{
  g1_lod.init_lod();
}

void g1_map_class::calc_map_lod(g1_object_controller_class *cont)
{
  pf_calc_map_lod.start();

  g1_lod.use_controller(cont);
  g1_lod.calculate_lod(g1_lod.root);

  pf_calc_map_lod.stop();
}

static r1_texture_ref g1_default_texture("tron_grid");
const w32          g1_max_objs_in_view = 256;
w32                g1_num_objs_in_view = 0;
w32 g1_objs_in_view[g1_max_objs_in_view];
r1_vert temp_buf_1[9];
r1_vert temp_buf_2[9];

i4_array<i4_transform_class> g1_obj_transforms_in_view(0, g1_max_objs_in_view);
class transform_killer_class : public i4_init_class
{
  //fix this trey!  can't have global i4_array's without something to clean it up.
  void uninit() { g1_obj_transforms_in_view.uninit(); }
} transform_killer;


i4_bool g1_draw_tri(r1_vert *points, 
                           w16 a, w16 b, w16 c,
                           r1_texture_handle texture, w16 clip=1)
//{{{
{
  pf_draw_tri.start();

  g1_stat_counter.increment(g1_statistics_counter_class::TERRAIN_POLYS);            
  g1_stat_counter.increment(g1_statistics_counter_class::TOTAL_POLYS);

  i4_bool ret = i4_F;
  r1_render_api_class *api = g1_render.r_api;
  i4_float size=0;
  sw32 num_poly_verts = 3;
  r1_vert *clipped_poly, *v;
  w16 *clipped_refs;
  static w16 clipping_refs[16] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 };
  w16 refs[3];
  i4_float near_w=0.0001;      
  int i,j;

  refs[0] = a;
  refs[1] = b;
  refs[2] = c;
  clipped_poly = points;
  clipped_refs = &refs[0];

  // hacked backface culling (should do this in worldspace before projection...)
  i4_3d_vector vt1(clipped_poly[a].v),vt2(clipped_poly[c].v),normal;

  vt1 -= clipped_poly[b].v;
  vt2 -= clipped_poly[b].v;
  normal.cross(vt2,vt1);
  if (normal.dot(clipped_poly[b].v)>0)
  {
    pf_draw_tri.stop();
    return i4_F;
  }

  if ((clipped_poly[a].outcode | clipped_poly[b].outcode | clipped_poly[c].outcode)==0)
  {
    for (j=0; j<num_poly_verts; j++)
    {
      v = &clipped_poly[clipped_refs[j]];
      if (v->w > near_w)
        near_w=v->w;
    }
  }
  else
  {
    clipped_poly = api->clip_poly(&num_poly_verts,
                                  clipped_poly,
                                  clipped_refs,
                                  temp_buf_1,
                                  temp_buf_2,
                                  R1_CLIP_NO_CALC_OUTCODE);

    if (!clipped_poly || num_poly_verts<3)      
      num_poly_verts=0;

    if (num_poly_verts)      
    {
      clipped_refs = clipping_refs;

      for (j=0; j<num_poly_verts; j++)
      {
        v = &clipped_poly[j];

        float ooz = r1_ooz(v->v.z);            
          
        v->px = v->v.x*ooz*g1_render.center_x + g1_render.center_x;
        v->py = v->v.y*ooz*g1_render.center_y + g1_render.center_y;
        v->w  = ooz;                  
        if (v->w > near_w)
          near_w=v->w;
      }
    }
  }

  if (num_poly_verts)
  {
    if (texture)
    {
      sw32 texture_size=i4_f_to_i(g1_render.center_x * near_w * 0.5 * 0.5);
      api->use_texture(texture, texture_size, 0);
    }

    if (g1_render.draw_mode==g1_render_class::WIREFRAME)
    {     
      api->set_constant_color(0x7f7f7f);
      api->disable_texture();
      api->set_shading_mode(R1_SHADE_DISABLED);

      r1_vert v[4];
      v[0]=clipped_poly[0];
      v[1]=clipped_poly[1];
      v[2]=clipped_poly[2];
      v[3]=clipped_poly[0];

      r1_clip_render_lines(3, v, g1_render.center_x, g1_render.center_y, api);
      api->set_shading_mode(R1_WHITE_SHADING);
    }
    else
      api->render_poly(num_poly_verts, clipped_poly, clipped_refs);

    ret = i4_T;
  }

  pf_draw_tri.stop();
  return ret;
}

static void gather_objects(int x1, int y1, int x2, int y2)
{
  pf_gather_objects.start();

  for (int y=y1; y<y2; y++)
  {
    g1_map_cell_class *map_cell=g1_get_cell(x1,y);

    for (int x=x1; x<x2; x++, map_cell++)
    {
      // collect objects
      for (g1_object_chain_class *o=map_cell->get_obj_list(); o; o=o->next)
      {
        g1_object_class *obj=o->object;
        
        if (!obj->get_flag(g1_object_class::SCRATCH_BIT))
        {
          if (g1_num_objs_in_view<g1_max_objs_in_view) 
          {              
          
            g1_objs_in_view[g1_num_objs_in_view] = obj->global_id;
            obj->world_transform = g1_obj_transforms_in_view.add();
          
            g1_num_objs_in_view++;
          
            //have the object update his transform
            obj->calc_world_transform(g1_render.frame_ratio);          
            obj->set_flag(g1_object_class::SCRATCH_BIT, 1);
          }   
        }
      }
    }
  }

  pf_gather_objects.stop();
}


static i4_transform_class comp_t;
int object_compare(const void *a, const void *b)
{
  g1_object_class 
    *oa = g1_global_id.get(*((w32*)a)),
    *ob = g1_global_id.get(*((w32*)b));


  // sort objects with alpha last
  if (oa->get_type()->get_flag(g1_object_definition_class::HAS_ALPHA))
  {
    if (!ob->get_type()->get_flag(g1_object_definition_class::HAS_ALPHA))
      return -1;
  }
  else if (ob->get_type()->get_flag(g1_object_definition_class::HAS_ALPHA))
    return 1;
    

  i4_float
    za = comp_t.x.z*oa->x + comp_t.y.z*oa->y + comp_t.z.z*oa->h + comp_t.t.z,
    zb = comp_t.x.z*ob->x + comp_t.y.z*ob->y + comp_t.z.z*ob->h + comp_t.t.z;

  if (za>zb)
    return 1;
  else
    return -1;
}

void g1_map_class::fast_draw_cells(g1_draw_context_class  *context)
{
  pf_map_fast.start();

  r1_render_api_class *api = g1_render.r_api;
  i4_transform_class t(*context->transform);
  i4_3d_vector pos;


//   g1_lod.cont->view.get_camera_pos(pos);
  pos = g1_lod.pos;

  g1_num_objs_in_view = 0;
  g1_obj_transforms_in_view.clear();

  if (g1_render.draw_mode==g1_render_class::SOLID)
    api->use_texture(g1_default_texture.get(), 1, 0);
  
  int i=0,j;
  g1_map_vertex_class *vt[4];
  g1_map_vertex_class *v=verts;

  g1_lod.last_context=0;
  for (j=0; j<g1_lod.num_quads; j++)
  {
    lod_node *p = g1_lod.quad[j];
    int 
      x1 = p->x1, y1 = p->y1, 
      x2 = p->x2, y2 = p->y2,
      clip = !(p->flags & lod_node::IN_VIEW);

    sw32 mw_p1=width()+1;

    vt[0]=v + x1 + y1 * mw_p1;          
    vt[1]=v + x2 + y1 * mw_p1;          
    vt[2]=v + x2 + y2 * mw_p1;          
    vt[3]=v + x1 + y2 * mw_p1;          

    vt[0]->transform(t, x1, y1, g1_render.scale_x,g1_render.scale_y);
    vt[1]->transform(t, x2, y1, g1_render.scale_x,g1_render.scale_y);
    vt[2]->transform(t, x2, y2, g1_render.scale_x,g1_render.scale_y);
    vt[3]->transform(t, x1, y2, g1_render.scale_x,g1_render.scale_y);

    if (vt[0]->calc_clip_code()==0)
      vt[0]->project(g1_render.center_x, g1_render.center_y);
    if (vt[1]->calc_clip_code()==0)
      vt[1]->project(g1_render.center_x, g1_render.center_y);
    if (vt[2]->calc_clip_code()==0)
      vt[2]->project(g1_render.center_x, g1_render.center_y);
    if (vt[3]->calc_clip_code()==0)      
      vt[3]->project(g1_render.center_x, g1_render.center_y);

    // collect objects
    i4_3d_vector mid(i4_float(x2+x1)/2.0,i4_float(y2+y1)/2.0,i4_float(p->z2+p->z1)/2.0);
    mid -= pos;


    if (mid.dot(mid) < g1_resources.lod_disappear_dist)
      gather_objects(x1,y1,x2,y2);
    else
      i++;

    if (x1+1==x2 && y1+1==y2)
    {
      // single cell
      g1_map_cell_class *map_cell=cells + x1 + y1*width();

      g1_lod.last_context=0;
      r1_texture_handle han = g1_tile_man.get_texture(map_cell->type);
      if (han && han!=g1_tile_man.get_pink())
      {
        static float u[4]={0,1,1,0},v[4]={1,1,0,0};
        int uv_on=map_cell->get_rotation();
        int uv_dir=map_cell->mirrored() ? 3 : 1;
        r1_vert poly[4];
        
        for (int i=0; i<4; i++)
        {
          vt[i]->set_r1_vert(&poly[i]);
          poly[i].s=u[uv_on];
          poly[i].t=v[uv_on];
          poly[i].a=1.0;
          uv_on=(uv_on+uv_dir)&3;
        }
        vt[0]->get_rgb(poly[0].r, poly[0].g, poly[0].b, x1, y1);
        vt[1]->get_rgb(poly[1].r, poly[1].g, poly[1].b, x2, y1);
        vt[2]->get_rgb(poly[2].r, poly[2].g, poly[2].b, x2, y2);
        vt[3]->get_rgb(poly[3].r, poly[3].g, poly[3].b, x1, y2);
        
        if (post_cell_draw)
          post_cell_draw(x1,y1, post_cell_draw_context);

        if (g1_draw_tri(poly, 0,1,2, han))
          han = 0;
        g1_draw_tri(poly, 0,2,3, han);
      }
    }
    else
    {
      // larger LOD cell

      texture_context *tc = &g1_lod.map_texture[p->texture_context];
      r1_texture_handle han;

      if (g1_lod.last_context==p->texture_context)
        han = 0;
      else
        han = tc->texture;

      r1_vert poly[4];

      for (int i=0; i<4; i++)
      {
        vt[i]->set_r1_vert(&poly[i]);
        poly[i].a=1.0;
      }
      vt[0]->get_rgb(poly[0].r, poly[0].g, poly[0].b, x1, y1);
      vt[1]->get_rgb(poly[1].r, poly[1].g, poly[1].b, x2, y1);
      vt[2]->get_rgb(poly[2].r, poly[2].g, poly[2].b, x2, y2);
      vt[3]->get_rgb(poly[3].r, poly[3].g, poly[3].b, x1, y2);

      poly[0].s=(x1 - tc->x1)*tc->sx;
      poly[0].t=(y1 - tc->y1)*tc->sy;
      poly[1].s=(x2 - tc->x1)*tc->sx;
      poly[1].t=(y1 - tc->y1)*tc->sy;
      poly[2].s=(x2 - tc->x1)*tc->sx;
      poly[2].t=(y2 - tc->y1)*tc->sy;
      poly[3].s=(x1 - tc->x1)*tc->sx;
      poly[3].t=(y2 - tc->y1)*tc->sy;

      if (g1_draw_tri(poly, 0,1,2, han))
        han = 0;
      if (g1_draw_tri(poly, 0,2,3, han))
        han = 0;
      if (han==0)
        g1_lod.last_context = p->texture_context;
    }
  }

  for (i=0; i<g1_num_objs_in_view; i++)
  {
    g1_object_class *o=g1_global_id.checked_get(g1_objs_in_view[i]);
    if (o)
      o->set_flag(g1_object_class::SCRATCH_BIT, 0);
  }


  li_class *old_this=li_this;
  //draw the objects BACK TO FRONT
  comp_t = *context->transform;
  qsort(g1_objs_in_view, g1_num_objs_in_view, sizeof(g1_objs_in_view[0]), object_compare);

  for (i=g1_num_objs_in_view-1;i>=0;i--)
  {
    g1_object_class *o=g1_global_id.checked_get(g1_objs_in_view[i]);
    if (o)
    {
      li_this=o->vars;

      o->set_flag(g1_object_class::SCRATCH_BIT, 0);
      if (o->world_transform!=0)
        o->draw(context);
      else
        i4_warning("null transform");
    }
  }

  if (context->draw_editor_stuff)
  {
    for (i=g1_num_objs_in_view-1;i>=0;i--)
    {
      g1_object_class *o=g1_global_id.checked_get(g1_objs_in_view[i]);
      if (o)
      {
        li_this=o->vars;
        o->editor_draw(context);
      }
    }
  }  
  
  li_this=old_this;
    
  for (j=0; j<g1_lod.num_quads; j++)
  {
    lod_node *p = g1_lod.quad[j];
    int 
      x1 = p->x1, y1 = p->y1, 
      x2 = p->x2, y2 = p->y2;

    sw32 mw_p1=width()+1;

    vt[0]=v + x1 + y1 * mw_p1;          
    vt[1]=v + x2 + y1 * mw_p1;          
    vt[2]=v + x2 + y2 * mw_p1;          
    vt[3]=v + x1 + y2 * mw_p1;          

    vt[0]->clear_calculations();
    vt[1]->clear_calculations();
    vt[2]->clear_calculations();
    vt[3]->clear_calculations();
  }

  g1_stat_counter.increment(g1_statistics_counter_class::FRAMES);

  pf_map_fast.stop();
}


class num_in_view_reset_class : public g1_global_id_reset_notifier
{
public:
  virtual void reset() 
  {
    g1_num_objs_in_view=0;   
  }
} num_in_view_reset_inst;
