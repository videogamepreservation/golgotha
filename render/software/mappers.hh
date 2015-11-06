/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef MAPPERS_HH
#define MAPPERS_HH

#include "software/r1_software.hh"

//this is what defines all of the various texturemapping, tri setup, and span drawing functions
//all of the various texturemappers / fillers / lit texturemappers / etc go through approximately
//3 stages.

//1 - "poly setup" - the polygon is broken into triangles and the gradients are calculated for
//                   whatever is requried, edges are added to the span list table, etc

//(if spanbuffering, all of the edges are sorted and spans generated for each tri inbetween)
//steps 1 and 2

//2 - "rasterization or spanning" - the tri's list of spans is stepped through or the edges
//                                  of the tri are stepped down

//3 - "scanline texturing" - the horizontal spans of the tri are drawn

union span_entry;
struct span_tri_info;
struct s_vert;
struct tri_edge;

//span sorting function types
typedef i4_bool build_triangle_span_lists_func();
typedef i4_bool (*build_triangle_span_lists_func_ptr)();

build_triangle_span_lists_func intel_build_triangle_span_lists;

extern build_triangle_span_lists_func_ptr cur_build_span_lists_function;

//texturemapping function types
typedef void (texture_scanline_func)(w16 *scanline, sw32 offset, void *span_info, sw32 width);
typedef void (*texture_scanline_func_ptr)(w16 *scanline, sw32 offset, void *span_info, sw32 width);

extern texture_scanline_func_ptr texture_scanline_functions[];
extern texture_scanline_func_ptr cur_scanline_texture_func;

texture_scanline_func texture_scanline_affine_lit;
texture_scanline_func texture_scanline_affine_unlit;
texture_scanline_func texture_scanline_affine_unlit_holy;

texture_scanline_func texture_scanline_perspective_lit;
texture_scanline_func texture_scanline_perspective_unlit;
texture_scanline_func texture_scanline_perspective_unlit_holy;
texture_scanline_func texture_scanline_perspective_unlit_alpha;
texture_scanline_func texture_scanline_perspective_unlit_true_alpha;


texture_scanline_func texture_scanline_solid_blend_half;
texture_scanline_func texture_scanline_solid_fill;
texture_scanline_func texture_scanline_affine_unlit_holy_blend;
texture_scanline_func texture_scanline_affine_unlit_alpha;
texture_scanline_func texture_scanline_affine_unlit_alpha_sprite;
texture_scanline_func texture_scanline_affine_unlit_true_alpha;

//all texturemapping types are elisted below
//these values should not interfere
#define LEADING_1                               ((w8)128)

#define SPAN_TRI_UNDEFINED                      ((w8) 0)
#define SPAN_TRI_AFFINE_LIT                     ((w8) 1)
#define SPAN_TRI_PERSPECTIVE_LIT                ((w8) 2)
#define SPAN_TRI_AFFINE_UNLIT                   ((w8) 3)
#define SPAN_TRI_PERSPECTIVE_UNLIT              ((w8) 4)
#define SPAN_TRI_SOLID_FILL                     ((w8) 5)

//all types above this are considered "solid"
#define SPAN_TRI_SEE_THRU                       ((w8) 6) //used as comparison in span buffering
//all types below this are considered "see-thru-able"

#define SPAN_TRI_SOLID_BLEND                    ((w8) 7)
#define SPAN_TRI_AFFINE_UNLIT_HOLY              ((w8) 8)
#define SPAN_TRI_AFFINE_UNLIT_HOLY_BLEND        ((w8) 9)
#define SPAN_TRI_AFFINE_UNLIT_ALPHA             ((w8)10)
#define SPAN_TRI_AFFINE_UNLIT_ALPHA_SPRITE      ((w8)11)
#define SPAN_TRI_PERSPECTIVE_UNLIT_HOLY         ((w8)12)
#define SPAN_TRI_PERSPECTIVE_UNLIT_ALPHA        ((w8)13)
#define SPAN_TRI_LAST                           ((w8)14)

//poly setup functions
typedef void (poly_setup_func)(s_vert *v, sw32 t_verts);
typedef void (*poly_setup_func_ptr)(s_vert *v, sw32 t_verts);

poly_setup_func poly_setup_affine_lit;
poly_setup_func poly_setup_perspective_lit;
poly_setup_func poly_setup_solid_color;

//special cased alpha-blend sprite
poly_setup_func sprite_setup_affine_unlit_alpha;

extern poly_setup_func_ptr poly_setup_functions[];

//spanning functions (steps through the list of spans for the tri)
typedef void (span_draw_func)(span_tri_info *tri);
typedef void (*span_draw_func_ptr)(span_tri_info *tri);

span_draw_func span_draw_affine_unlit;
span_draw_func span_draw_affine_lit;

span_draw_func span_draw_perspective_unlit;
span_draw_func span_draw_perspective_lit;

span_draw_func span_draw_solid_color;

extern span_draw_func_ptr span_draw_functions[];

//straight rasterization functions
typedef void (tri_draw_func)(tri_edge &top_to_mid, tri_edge &top_to_bot, tri_edge &mid_to_bot, sw32 start_y, i4_bool edge_comp);
typedef void (*tri_draw_func_ptr)(tri_edge &top_to_mid, tri_edge &top_to_bot, tri_edge &mid_to_bot, sw32 start_y, i4_bool edge_comp);

tri_draw_func tri_draw_affine_unlit;
tri_draw_func tri_draw_affine_lit;
tri_draw_func tri_draw_perspective_lit;
tri_draw_func tri_draw_perspective_unlit;
tri_draw_func tri_draw_solid_color;

extern tri_draw_func_ptr tri_draw_functions[];

#ifdef USE_AMD3D

//declare these amd3d functions

build_triangle_span_lists_func amd3d_build_triangle_span_lists;

texture_scanline_func texture_scanline_affine_lit_amd3d;
texture_scanline_func texture_scanline_perspective_lit_amd3d;
texture_scanline_func texture_scanline_perspective_unlit_amd3d;
texture_scanline_func texture_scanline_perspective_unlit_holy_amd3d;
texture_scanline_func texture_scanline_perspective_unlit_alpha_amd3d;

span_draw_func span_draw_affine_unlit_amd3d;
span_draw_func span_draw_affine_lit_amd3d;
span_draw_func span_draw_perspective_unlit_amd3d;
span_draw_func span_draw_perspective_lit_amd3d;

#endif

#endif

