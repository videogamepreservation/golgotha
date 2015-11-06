/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_RENDER_HH
#define G1_RENDER_HH

#include "arch.hh"
#include "math/num_type.hh"
#include "player_type.hh"
#include "r1_vert.hh"
#include "tex_id.hh"

class g1_screen_box
{
public:
  i4_float x1,y1,x2,y2;
  i4_float z1,z2,w;
  w32 object_id;            // same as global id of object
};


struct g1_selectable_list
{
  enum { MAX=64 };
  w32 t_recent;
  g1_screen_box recent[MAX];
  g1_screen_box *add()   { return t_recent<MAX-1 ? recent + (t_recent++) : 0; }
  g1_selectable_list() { t_recent=0; }
};

class g1_post_draw_quad_class
{
public:
  w16 vert_ref[4];  // if vert_ref[3]=0xffff then it is a tri
  g1_post_draw_quad_class() { ; }
  g1_post_draw_quad_class(w16 a, w16 b, w16 c, w16 d)
  { 
    vert_ref[0]=a;
    vert_ref[1]=b;
    vert_ref[2]=c;
    vert_ref[3]=d;
  }
};

class g1_quad_object_class;
class i4_transform_class;
class g1_sprite_class;
class i4_3d_point_class;
class r1_render_api_class;
class cell_shadow_poly_list;
class g1_map_vertex_class;
class g1_map_cell_class;
class i4_transform_class;
class g1_quad_class;
class g1_draw_context_class;
class g1_object_class;
class g1_map_class;
class i4_draw_context_class;
class i4_font_class;

typedef g1_quad_class *(*g1_tint_modify_function_type)(g1_quad_class *in, g1_player_type player);
typedef void (*g1_get_ambient_function_type)(i4_transform_class *object_to_world, 
                                        i4_float &ar, i4_float &ag, i4_float &ab);

// this is the default function for handling tinted polygons
g1_quad_class *g1_tint_modify_function(g1_quad_class *in, g1_player_type player);

// default method for getting ambient lighting, reads from the map
void g1_get_ambient_function(i4_transform_class *object_to_world, 
                             i4_float &ar, i4_float &ag, i4_float &ab);

enum { G1_REVERSE_POINTS=2};

class g1_render_class
{
public:
  g1_selectable_list *current_selectable_list;
  g1_selectable_list *last_selectable_list;

  enum { WIREFRAME, TEXTURED, SOLID } draw_mode;

  static i4_bool main_draw;

  static i4_float scale_x, scale_y, ooscale_x, ooscale_y,
    center_x, center_y;

  static r1_render_api_class *r_api;

  // this number will be >0 && <=1, it use to interpolate object position on fast frame
  // rates i.e. draw_x = (lx-x)*frame_ration + lx
  float frame_ratio;


  i4_font_class *rendered_font;

  g1_tint_modify_function_type tint_modify;
  g1_get_ambient_function_type get_ambient;

  static sw8 render_damage_level;

  float calculate_frame_ratio();
  void set_render_damage_level(sw8 level) { render_damage_level = level; }

  void render_object(g1_quad_object_class *obj,
                     i4_transform_class   *object_to_view,
                     i4_transform_class   *object_to_world = 0,
                     i4_float             texture_scale = 1.0,                      
                     int                  player_num = -1,         // -1 is no player
                     sw32                 current_frame = 0,
                     g1_screen_box        *outline = 0,
                     w32                  option_flags = 0);

  void render_object_polys(g1_quad_object_class *obj,
                           i4_transform_class *object_to_view,
                           sw32 current_frame = 0);


  // draws a sprite on the screen nearest to view, sprite_width is in pixels
  void render_near_sprite(float px, float py,
                          r1_texture_handle tex,
                          float sprite_width=0.1, float sprite_height=0.1,
                          float s1=0, float t1=0, float s2=1, float t2=1);

  // draws a sprite on the at the specifed z, (it is projected) sprite_width is world-space size
  void render_sprite(const i4_3d_vector &view_space_position,
                     r1_texture_handle tex,
                     float sprite_width=0.1, float sprite_height=0.1,
                     float s1=0, float t1=0, float s2=1, float t2=1);

  void render_3d_line(const i4_3d_point_class &v1,
                      const i4_3d_point_class &v2,
                      i4_color color1,
                      i4_color color2,
                      i4_transform_class *t,
                      i4_bool draw_in_front_of_everything=i4_F);

  void draw_outline(g1_screen_box *outline, g1_object_class *for_who);

  void post_draw_quads();

  sw32 add_post_draw_vert(r1_vert &a);
  sw32 add_post_draw_quad(g1_post_draw_quad_class &q);

  void add_smoke_vert(i4_float x, i4_float y, i4_float z,
                      i4_float r, i4_float g, i4_float b, 
                      i4_float alpha)
  {
    r1_vert a;
  
    a.v.x=x;
    a.v.y=y;
    a.v.z=z;

    a.r=r;
    a.g=g;
    a.b=b;
    a.a=alpha;
  
    add_post_draw_vert(a);
  }

  void add_translucent_trail(i4_transform_class *t,
                             i4_3d_point_class *spots, int t_spots, 
                             float start_width, float end_width,
                             float start_alpha, float end_alpha,
                             w32 sc, w32 ec);

  /*  void draw_building(g1_draw_context_class *context,
                     g1_map_cell_class *cell,
                     w16 x, w16 y,
                     g1_map_vertex_class *v1,
                     g1_map_vertex_class *v2,
                     g1_map_vertex_class *v3,
                     g1_map_vertex_class *v4,
                     i4_bool p_or_span); */

  void draw_rectangle(int x1, int y1, int x2, int y2, i4_color color,
                      i4_draw_context_class &context);
                      

  // this routine transform all 4 verts (in place) and clip the resulting quad and render it
  void clip_render_quad(g1_quad_class *q, r1_vert *verts, 
                        i4_transform_class *t,
                        int current_frame);

  void install_font();
  void uninstall_font();

  g1_render_class()
  {
    rendered_font=0;
    draw_mode=TEXTURED;
    tint_modify=g1_tint_modify_function;
    get_ambient=g1_get_ambient_function;
    current_selectable_list=0;
  }

  i4_bool project_point(const i4_3d_point_class &p, r1_vert &v, i4_transform_class *transform);
};

void g1_setup_tri_texture_coords(r1_vert *tri1, r1_vert *tri2,
                                 int cell_rotation, int cell_is_mirrored);

extern g1_render_class g1_render;

#endif

