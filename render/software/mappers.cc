/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "software/r1_software.hh"
#include "software/r1_software_globals.hh"
#include "software/mappers.hh"
#include "device/processor.hh"

//global instance of the various function tables

texture_scanline_func_ptr texture_scanline_functions[SPAN_TRI_LAST];
poly_setup_func_ptr       poly_setup_functions[SPAN_TRI_LAST];
tri_draw_func_ptr         tri_draw_functions[SPAN_TRI_LAST];
span_draw_func_ptr        span_draw_functions[SPAN_TRI_LAST];

//this is super important. this points to the current scanline texturemapping function
//and is used by all of the rasterizers / span drawers (so before you call the rasterizer /
//setup function, make sure you set this pointer correctly)
texture_scanline_func_ptr cur_scanline_texture_func = 0;

//the current span list builder (either the one optimized for intel or amd3d)
build_triangle_span_lists_func_ptr cur_build_span_lists_function = 0;

void r1_software_class::initialize_function_pointers(i4_bool amd3d)
{
  //zero them out to start with
  memset(texture_scanline_functions, 0, sizeof(texture_scanline_func_ptr) * SPAN_TRI_LAST);
  memset(poly_setup_functions,       0, sizeof(poly_setup_func_ptr)       * SPAN_TRI_LAST);  
  memset(tri_draw_functions,         0, sizeof(tri_draw_func_ptr)         * SPAN_TRI_LAST);
  memset(span_draw_functions,        0, sizeof(span_draw_func_ptr)        * SPAN_TRI_LAST);
  
  cur_build_span_lists_function = intel_build_triangle_span_lists;

  //scanline texturing / drawing functions
  texture_scanline_functions[SPAN_TRI_AFFINE_UNLIT]              = texture_scanline_affine_unlit;
  texture_scanline_functions[SPAN_TRI_AFFINE_UNLIT_HOLY]         = texture_scanline_affine_unlit_holy;
  texture_scanline_functions[SPAN_TRI_AFFINE_UNLIT_HOLY_BLEND]   = texture_scanline_affine_unlit_holy_blend;
  texture_scanline_functions[SPAN_TRI_AFFINE_UNLIT_ALPHA]        = texture_scanline_affine_unlit_alpha;
    
#ifdef USE_ASM
  //this is a special case mapper optimized in asm only
  //(ignores the t gradient, basically, since sprites have no delta t per screen x)
  texture_scanline_functions[SPAN_TRI_AFFINE_UNLIT_ALPHA_SPRITE] = texture_scanline_affine_unlit_alpha_sprite;
#else  
  texture_scanline_functions[SPAN_TRI_AFFINE_UNLIT_ALPHA_SPRITE] = texture_scanline_affine_unlit_alpha;
#endif

  texture_scanline_functions[SPAN_TRI_AFFINE_LIT]              = texture_scanline_affine_lit;
  texture_scanline_functions[SPAN_TRI_PERSPECTIVE_LIT]         = texture_scanline_perspective_lit;
  texture_scanline_functions[SPAN_TRI_PERSPECTIVE_UNLIT_ALPHA] = texture_scanline_perspective_unlit_alpha;
  texture_scanline_functions[SPAN_TRI_PERSPECTIVE_UNLIT]       = texture_scanline_perspective_unlit;
  texture_scanline_functions[SPAN_TRI_PERSPECTIVE_UNLIT_HOLY]  = texture_scanline_perspective_unlit_holy;
  
  texture_scanline_functions[SPAN_TRI_SOLID_FILL]              = texture_scanline_solid_fill;
  texture_scanline_functions[SPAN_TRI_SOLID_BLEND]             = texture_scanline_solid_blend_half;

  //poly setup functions (calculates deltas, gradients, etc)
  poly_setup_functions[SPAN_TRI_AFFINE_UNLIT]              = poly_setup_affine_lit;
  poly_setup_functions[SPAN_TRI_AFFINE_UNLIT_HOLY]         = poly_setup_affine_lit;
  poly_setup_functions[SPAN_TRI_AFFINE_UNLIT_HOLY_BLEND]   = poly_setup_affine_lit;
  poly_setup_functions[SPAN_TRI_AFFINE_UNLIT_ALPHA]        = poly_setup_affine_lit;
  poly_setup_functions[SPAN_TRI_AFFINE_UNLIT_ALPHA_SPRITE] = poly_setup_affine_lit;
  poly_setup_functions[SPAN_TRI_AFFINE_LIT]                = poly_setup_affine_lit;

  poly_setup_functions[SPAN_TRI_PERSPECTIVE_UNLIT]       = poly_setup_perspective_lit;
  poly_setup_functions[SPAN_TRI_PERSPECTIVE_UNLIT_HOLY]  = poly_setup_perspective_lit;
  poly_setup_functions[SPAN_TRI_PERSPECTIVE_UNLIT_ALPHA] = poly_setup_perspective_lit;
  poly_setup_functions[SPAN_TRI_PERSPECTIVE_LIT]         = poly_setup_perspective_lit;

  poly_setup_functions[SPAN_TRI_SOLID_FILL]              = poly_setup_solid_color;
  poly_setup_functions[SPAN_TRI_SOLID_BLEND]             = poly_setup_solid_color;

  //span drawing functions (sets up some variables, steps through spans, calculates starting s,t,etc, for each span)
  span_draw_functions[SPAN_TRI_AFFINE_UNLIT]              = span_draw_affine_unlit;
  span_draw_functions[SPAN_TRI_AFFINE_UNLIT_HOLY]         = span_draw_affine_unlit;
  span_draw_functions[SPAN_TRI_AFFINE_UNLIT_HOLY_BLEND]   = span_draw_affine_unlit;
  span_draw_functions[SPAN_TRI_AFFINE_UNLIT_ALPHA]        = span_draw_affine_unlit;
  span_draw_functions[SPAN_TRI_AFFINE_UNLIT_ALPHA_SPRITE] = span_draw_affine_unlit;
  span_draw_functions[SPAN_TRI_AFFINE_LIT]                = span_draw_affine_lit;
  span_draw_functions[SPAN_TRI_PERSPECTIVE_UNLIT]         = span_draw_perspective_unlit;
  span_draw_functions[SPAN_TRI_PERSPECTIVE_UNLIT_HOLY]    = span_draw_perspective_unlit;
  span_draw_functions[SPAN_TRI_PERSPECTIVE_UNLIT_ALPHA]   = span_draw_perspective_unlit;
  span_draw_functions[SPAN_TRI_PERSPECTIVE_LIT]           = span_draw_perspective_lit;

  span_draw_functions[SPAN_TRI_SOLID_FILL]              = span_draw_solid_color;
  span_draw_functions[SPAN_TRI_SOLID_BLEND]             = span_draw_solid_color;
  
  //triangle drawing functions (rasterizes triangles, steps the necessary values (s,t,l,etc) )
  tri_draw_functions[SPAN_TRI_AFFINE_UNLIT]              = tri_draw_affine_unlit;
  tri_draw_functions[SPAN_TRI_AFFINE_UNLIT_HOLY]         = tri_draw_affine_unlit;
  tri_draw_functions[SPAN_TRI_AFFINE_UNLIT_HOLY_BLEND]   = tri_draw_affine_unlit;
  tri_draw_functions[SPAN_TRI_AFFINE_UNLIT_ALPHA]        = tri_draw_affine_unlit;
  tri_draw_functions[SPAN_TRI_AFFINE_UNLIT_ALPHA_SPRITE] = tri_draw_affine_unlit;
  tri_draw_functions[SPAN_TRI_AFFINE_LIT]                = tri_draw_affine_lit;  

  tri_draw_functions[SPAN_TRI_PERSPECTIVE_UNLIT]       = tri_draw_perspective_unlit;
  tri_draw_functions[SPAN_TRI_PERSPECTIVE_UNLIT_HOLY]  = tri_draw_perspective_unlit;
  tri_draw_functions[SPAN_TRI_PERSPECTIVE_UNLIT_ALPHA] = tri_draw_perspective_unlit;
  tri_draw_functions[SPAN_TRI_PERSPECTIVE_LIT]         = tri_draw_perspective_lit;

  tri_draw_functions[SPAN_TRI_SOLID_FILL]              = 0;//tri_draw_solid_color;
  tri_draw_functions[SPAN_TRI_SOLID_BLEND]             = 0;//tri_draw_solid_color;

  do_amd3d = i4_F;

#ifdef USE_AMD3D
  if (amd3d)
  {
    i4_cpu_info_struct s;
    i4_get_cpu_info(&s);

    if (s.cpu_flags & i4_cpu_info_struct::AMD3D)
    {
      do_amd3d = i4_T;

      //we're compiling amd3d stuff, its requesting the amd3d functions,
      //the processor supports them, do it
      cur_build_span_lists_function = amd3d_build_triangle_span_lists;
    
      span_draw_functions[SPAN_TRI_AFFINE_UNLIT]              = span_draw_affine_unlit_amd3d;
      span_draw_functions[SPAN_TRI_AFFINE_UNLIT_HOLY]         = span_draw_affine_unlit_amd3d;
      span_draw_functions[SPAN_TRI_AFFINE_UNLIT_HOLY_BLEND]   = span_draw_affine_unlit_amd3d;
      span_draw_functions[SPAN_TRI_AFFINE_UNLIT_ALPHA]        = span_draw_affine_unlit_amd3d;
      span_draw_functions[SPAN_TRI_AFFINE_UNLIT_ALPHA_SPRITE] = span_draw_affine_unlit_amd3d;
      span_draw_functions[SPAN_TRI_AFFINE_LIT]                = span_draw_affine_lit_amd3d;
      span_draw_functions[SPAN_TRI_PERSPECTIVE_UNLIT]         = span_draw_perspective_unlit_amd3d;
      span_draw_functions[SPAN_TRI_PERSPECTIVE_UNLIT_HOLY]    = span_draw_perspective_unlit_amd3d;
      span_draw_functions[SPAN_TRI_PERSPECTIVE_UNLIT_ALPHA]   = span_draw_perspective_unlit_amd3d;
      span_draw_functions[SPAN_TRI_PERSPECTIVE_LIT]           = span_draw_perspective_lit_amd3d;

      texture_scanline_functions[SPAN_TRI_AFFINE_LIT]              = texture_scanline_affine_lit_amd3d;
      texture_scanline_functions[SPAN_TRI_PERSPECTIVE_LIT]         = texture_scanline_perspective_lit_amd3d;
      texture_scanline_functions[SPAN_TRI_PERSPECTIVE_UNLIT_ALPHA] = texture_scanline_perspective_unlit_alpha_amd3d;
      texture_scanline_functions[SPAN_TRI_PERSPECTIVE_UNLIT]       = texture_scanline_perspective_unlit_amd3d;
      texture_scanline_functions[SPAN_TRI_PERSPECTIVE_UNLIT_HOLY]  = texture_scanline_perspective_unlit_holy_amd3d;
    }
  }
#endif
}

#ifdef USE_ASM

//laziness. include all the scanline texturemappers / fillers
#include "software/affine_map_lit_asm.cc"
#include "software/affine_map_unlit_asm.cc"
#include "software/affine_map_unlit_holy_asm.cc"
#include "software/affine_map_unlit_holy_blend_asm.cc"
#include "software/affine_map_unlit_alpha_asm.cc"
#include "software/affine_map_unlit_alpha_sprite_asm.cc"
#include "software/affine_map_unlit_true_alpha_c.cc"
#include "software/perspective_map_lit_asm.cc"
#include "software/perspective_map_unlit_asm.cc"
#include "software/perspective_map_unlit_holy_asm.cc"
#include "software/perspective_map_unlit_alpha_asm.cc"
#include "software/perspective_map_unlit_true_alpha_c.cc"
#include "software/solid_blend_half_asm.cc"
#include "software/solid_fill_asm.cc"

#ifdef USE_AMD3D
//include all the amd3d texturemappers too
#include "software/amd3d/affine_map_lit_asm_amd3d.cc"
#include "software/amd3d/perspective_map_lit_asm_amd3d.cc"
#include "software/amd3d/perspective_map_unlit_alpha_asm_amd3d.cc"
#include "software/amd3d/perspective_map_unlit_asm_amd3d.cc"
#include "software/amd3d/perspective_map_unlit_holy_asm_amd3d.cc"
#endif

#else

#include "software/affine_map_lit_c.cc"
#include "software/affine_map_unlit_c.cc"
#include "software/affine_map_unlit_holy_c.cc"
#include "software/affine_map_unlit_holy_blend_c.cc"
#include "software/affine_map_unlit_alpha_c.cc"
#include "software/affine_map_unlit_true_alpha_c.cc"
#include "software/perspective_map_lit_c.cc"
#include "software/perspective_map_unlit_c.cc"
#include "software/perspective_map_unlit_holy_c.cc"
#include "software/perspective_map_unlit_alpha_c.cc"
#include "software/perspective_map_unlit_true_alpha_c.cc"
#include "software/solid_blend_half_c.cc"
#include "software/solid_fill_c.cc"

#endif