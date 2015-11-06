/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include <windows.h>
#include <ddraw.h>
#include <math.h>
#include <memory.h>

#include "r1_clip.hh"

#include "software/r1_software.hh"
#include "software/r1_software_texture.hh"
#include "software/r1_software_globals.hh"
#include "software/mappers.hh"
#include "software/inline_fpu.hh"

#include "time/profile.hh"

static i4_profile_class pf_software_render_poly("software::render_poly");

#define R1_ABS(x) ((x)>0)?(x):(-(x))

r1_software_class::r1_software_class()
{  
  last_node=0;

  cur_s_width  = 0;
  cur_t_height = 0;

  render_device_flags = R1_SOFTWARE;
  
  allow_backfaces = i4_T;
  use_spans       = i4_T;
  do_perspective  = i4_T;
  do_flat         = i4_F;
  lock_cheat      = i4_F;
  
  cur_color_tint = -1;

  holy_mode         = i4_F;
  alphatexture_mode = i4_F;
}

r1_software_class::~r1_software_class()
{  
}

void r1_software_class::uninit()
{
  delete tmanager;
  tmanager = 0;  
}

void r1_software_class::set_write_mode(r1_write_mask_type mask)
{      
  write_mask=mask;
  //update_tri_function(); not needed on a write mask change
}

void r1_software_class::set_alpha_mode(r1_alpha_type type)
{
  if (alpha_mode != type)
  {
    alpha_mode = type;
    update_tri_function();
  }
}

void r1_software_class::set_shading_mode(r1_shading_type type)
{
  if (shade_mode != type)
  {
    shade_mode = type;
    update_tri_function();
  }
}

static i4_profile_class pf_software_select_texture("software select texture");

void r1_software_class::use_texture(r1_texture_handle material_ref, sw32 desired_width, w32 frame)
{
  if (!tmanager->valid_handle(material_ref))
  {
    disable_texture();
    return;
  }

  sw32 w,h;

  pf_software_select_texture.start();

  r1_miplevel_t *mip = tmanager->get_texture(material_ref, frame, desired_width, w,h);
  if (!mip)
  {
    disable_texture();
    return ;
  }

  r1_software_texture_class::used_node *n = (r1_software_texture_class::used_node *)mip->vram_handle;
  r1_software_texture_class::free_node *f = (r1_software_texture_class::free_node *)n->node;

  r1_software_texture_ptr    = (w16 *)f->start;
  r1_software_texture_width  = f->mip->width;
  r1_software_texture_height = f->mip->height;
  
  switch (f->mip->width)
  {
    case    1: r1_software_twidth_log2 =  0; break;
    case    2: r1_software_twidth_log2 =  1; break;
    case    4: r1_software_twidth_log2 =  2; break;
    case    8: r1_software_twidth_log2 =  3; break;
    case   16: r1_software_twidth_log2 =  4; break;
    case   32: r1_software_twidth_log2 =  5; break;
    case   64: r1_software_twidth_log2 =  6; break;
    case  128: r1_software_twidth_log2 =  7; break;
    case  256: r1_software_twidth_log2 =  8; break;
    case  512: r1_software_twidth_log2 =  9; break;
    case 1024: r1_software_twidth_log2 = 10; break;
  }

  // don't select the texture again if it's the same one we used last time
  if (mip)
  {        
    if (mip->entry->is_transparent())    
    {
      holy_mode = i4_T;
      alphatexture_mode = i4_F;
    }    
    else
    if (mip->entry->is_alphatexture())
    {
      alphatexture_mode = i4_T;
      holy_mode = i4_F;
    }
    else
    {
      holy_mode = i4_F;
      alphatexture_mode = i4_F;
    }

    if (mip != last_node)
    {    
      ((r1_software_texture_class *)tmanager)->select_texture(mip->vram_handle, cur_s_width, cur_t_height);
      last_node = mip;
    }
  }

  texture_mode=i4_T;  

  update_tri_function();
  pf_software_select_texture.stop();
}

// drawing will be done with the constant color if textures are disabled
void r1_software_class::disable_texture()
{  
  texture_mode=i4_F;
  last_node=0;
  holy_mode=i4_F;
  alphatexture_mode=i4_F;
  update_tri_function();
}

void r1_software_class::set_z_range(i4_float near_z, i4_float far_z)
{
  r1_near_clip_z = near_z;
  r1_far_clip_z  = far_z;
}

//initialized during init_ctable

void r1_software_class::prepare_verts(s_vert *s_verts, r1_vert *r1_verts, sw32 num_verts)
{
  sw32 i;
    
  s_vert  *s_v  = s_verts;
  r1_vert *r1_v = r1_verts;
    
  start_ceil(); //qftoi will use the ceil() function to convert floats to ints

  if (r1_software_render_expand_type!=R1_COPY_1x1_SCANLINE_SKIP)
  {
    for (i=0;i<num_verts;i++)
    {        
      s_v->px = r1_v->px;
      s_v->py = r1_v->py;

      if (s_v->px < 0.0001) s_v->px = 0;
      else
      if (s_v->px > width_compare) s_v->px = width_compare;
    
      if (s_v->py < 0.0001) s_v->py = 0;
      else
      if (s_v->py > height_compare) s_v->py = height_compare;
         
      s_v->ix = qftoi(s_v->px);
      s_v->iy = qftoi(s_v->py);
      
      s_v->ooz = r1_v->w;
    
      s_v++;
      r1_v++;
    }
  }
  else
  {
    for (i=0;i<num_verts;i++)
    {        
      s_v->px = r1_v->px;
      s_v->py = r1_v->py*0.5;

      if (s_v->px < 0.0001) s_v->px = 0;
      else
      if (s_v->px > width_compare) s_v->px = width_compare;
    
      if (s_v->py < 0.0001) s_v->py = 0;
      else
      if (s_v->py > (height_compare>>1)) s_v->py = (height_compare>>1);
         
      s_v->ix = qftoi(s_v->px);
      s_v->iy = qftoi(s_v->py);
      
      s_v->ooz = r1_v->w;
      
      s_v++;
      r1_v++;
    }
  }

  stop_ceil();

  w32     r,g,b;
  
  start_trunc(); //qftoi will now truncate floats to ints

  switch (shade_mode)
  {
  case R1_CONSTANT_SHADING:
    //whenever set_constant_color is called, cur_color
    //is adjusted for the proper bit depth / display mode
    s_verts->color = cur_color;
    break;

  case R1_COLORED_SHADING:
    s_v  = s_verts;
    r1_v = r1_verts;    
    for (i=0;i<num_verts;i++)
    {                    
      //these muls and shifts adjust for the proper bit depth / display mode
      r = qftoi(r1_v->r*red_clight_mul);
      g = qftoi(r1_v->g*green_clight_mul);
      b = qftoi(r1_v->b*blue_clight_mul);

      s_v->color = (r<<fmt.red_shift) | (g<<fmt.green_shift) | (b<<fmt.blue_shift);
      s_v->color = s_v->color | (s_v->color << 16);
      s_v++;
      r1_v++;
    }
  //even in colored shade mode, we need to calculate l
  case R1_WHITE_SHADING:
    s_v  = s_verts;
    r1_v = r1_verts;
    
    for (i=0;i<num_verts;i++)
    {  
      s_v->l = qftoi(r1_v->r*(i4_float)NUM_LIGHT_SHADES);      

#ifdef DEBUG
      if (s_v->l > NUM_LIGHT_SHADES || s_v->l < 0)
      {
        if (s_v->l < 0)  s_v->l = 0;
        else
        if (s_v->l > NUM_LIGHT_SHADES) s_v->l = NUM_LIGHT_SHADES;
        i4_warning("Bad lighting value");
      }
#endif       
      
      s_v++;
      r1_v++;
    }
    break;
  }  
  
  stop_trunc();

  if (texture_mode)
  {    
    s_vert  *s_v  = s_verts;
    r1_vert *r1_v = r1_verts;

    for (i=0;i<num_verts;i++)
    {      
#ifdef DEBUG
      
      if (r1_v->s > 1.0001f || r1_v->s < -0.0001f)
      {
//        i4_warning("bad s texture coordinate");        
      }
      if (r1_v->t > 1.0001f || r1_v->t < -0.0001f)
      {
//        i4_warning("bad t texture coordinate");        
      }
      
#endif
      //not perspective correct just yet, the poly might be affine mapped
      s_v->st_projected = i4_F;

      //software safety: computes texture coordinates as though valid texture ranges are from 0.5 to max-0.5
      //decreases chance of stepping off texture. *could* be replaced to go from 0 to max but its probably risky
      
      s_v->s = (r1_v->s * (cur_s_width  - 1.f) ) + 0.5f;
      s_v->t = (r1_v->t * (cur_t_height - 1.f) ) + 0.5f;

      //cap the values
      //this almost certainly cannot be removed since the maxtool often produces invalid texture coordinates,
      //although you might be able to cap it to the range [0,max] instead of [0.5, max-0.5]
      if (s_v->s < 0.5f)
      {
        s_v->s = 0.5f;
      }
      else
      if (s_v->s > cur_s_width - 0.5f)
      {
        s_v->s = cur_s_width - 0.5f;
      }

      if (s_v->t < 0.5f)
      {
        s_v->t = 0.5f;
      }
      else
      if (s_v->t > cur_t_height - 0.5f)
      {
        s_v->t = cur_t_height - 0.5f;
      }
      
      s_v++;
      r1_v++;
    }
  }  
}

s_vert temp_s_verts[128];

void r1_software_class::render_poly(int t_verts, r1_vert *verts)
{
  if (t_verts>128) return;
    
  if (!poly_setup_functions[small_poly_type]) return;
  if (!poly_setup_functions[big_poly_type])   return;

  pf_software_render_poly.start();
  
  prepare_verts(temp_s_verts,verts,t_verts);

  s_vert *v0 = &temp_s_verts[0];
  s_vert *v1 = &temp_s_verts[1];
  s_vert *v2 = &temp_s_verts[2];
    
  sw32 i;
  
  //make sure polys are valid
  //compute the area of the whole polygon
  tri_area_struct *t = triangle_info;

  total_poly_area = 0;

  for (i=1; i<t_verts-1; i++,t++)
  {
    t->calc_area(v0->px, v1->px, v2->px, v0->py, v1->py, v2->py);
    
    total_poly_area += t->area;
    
    v1++;
    v2++;
  }

  //if its < 500 pixels, draw a "small" polygon (affine mapped)
  
  start_trunc();
  //everything inside here pretty much uses the truncation qftoi()

  if (!do_perspective || ((R1_ABS(total_poly_area)) < 500) )
  {
    //setup as a small poly
    (poly_setup_functions[small_poly_type])(temp_s_verts,t_verts);
  }        
  else
  {  
    //setup as a big poly
    (poly_setup_functions[big_poly_type])(temp_s_verts,t_verts);
  }
  
  stop_trunc();

  pf_software_render_poly.stop();
}

void r1_software_class::render_sprite(r1_vert *verts)
{
  if (!poly_setup_functions[small_poly_type]) return;
  
  pf_software_render_poly.start();

  prepare_verts(temp_s_verts,verts,4);

  tri_area_struct *t = triangle_info;

  total_poly_area = 0;

  s_vert *v0 = &temp_s_verts[0];
  s_vert *v1 = &temp_s_verts[1];
  s_vert *v2 = &temp_s_verts[2];
    
  sw32 i;

  for (i=1; i<3; i++,t++)
  {
    t->calc_area(v0->px, v1->px, v2->px, v0->py, v1->py, v2->py);
    
    total_poly_area += t->area;
    
    v1++;
    v2++;
  }

  start_trunc();

  //special case alpha-blended sprites
  if (small_poly_type==SPAN_TRI_AFFINE_UNLIT_ALPHA)
  {
    small_poly_type=SPAN_TRI_AFFINE_UNLIT_ALPHA_SPRITE;

    //sets it up as a sprite (doesnt do some of the unnecessary steps required on polygons)
    sprite_setup_affine_unlit_alpha(temp_s_verts,4);
  }
  else
  {
    //set it up as a polygon
    poly_setup_functions[small_poly_type](temp_s_verts,4);  
  }
  
  stop_trunc();

  pf_software_render_poly.stop();
}

void r1_software_class::render_lines(int t_lines, r1_vert *verts)
{
  if (num_buffered_lines>=max_soft_lines) return;

  sw32 i;
  w32 r,g,b;
  i4_float fr,fg,fb;
  w16 start_color,end_color;
  
  start_trunc();
  
  for (i=0;i<t_lines;i++)
  {
    if (shade_mode==R1_CONSTANT_SHADING)
    {
      start_color = end_color = cur_color;
    }
    else
    {
      //draw the line w/a color that is sort of a combination of the 2 endpoints
      fr = verts[i].r;// + verts[i+1].r; if (fr > 1.f) fr = 1.f;
      fg = verts[i].g;// + verts[i+1].g; if (fg > 1.f) fg = 1.f;
      fb = verts[i].b;// + verts[i+1].b; if (fb > 1.f) fb = 1.f;
    
      r = qftoi(fr*red_clight_mul);
      g = qftoi(fg*green_clight_mul);
      b = qftoi(fb*blue_clight_mul);
      
      start_color = (r<<fmt.red_shift) | (g<<fmt.green_shift) | (b<<fmt.blue_shift);

      fr = verts[i+1].r;// + verts[i+1].r; if (fr > 1.f) fr = 1.f;
      fg = verts[i+1].g;// + verts[i+1].g; if (fg > 1.f) fg = 1.f;
      fb = verts[i+1].b;// + verts[i+1].b; if (fb > 1.f) fb = 1.f;
    
      r = qftoi(fr*red_clight_mul);
      g = qftoi(fg*green_clight_mul);
      b = qftoi(fb*blue_clight_mul);
    
      end_color = (r<<fmt.red_shift) | (g<<fmt.green_shift) | (b<<fmt.blue_shift);
    }
    
    software_lines[num_buffered_lines].x0 = qftoi(verts[i].px);    
    software_lines[num_buffered_lines].x1 = qftoi(verts[i+1].px);
    
    if (r1_software_render_expand_type==R1_COPY_1x1_SCANLINE_SKIP)
    {
      software_lines[num_buffered_lines].y0 = qftoi((verts[i].py*0.5)   + 0.5);
      software_lines[num_buffered_lines].y1 = qftoi((verts[i+1].py*0.5) + 0.5);
    }
    else
    {
      software_lines[num_buffered_lines].y0 = qftoi(verts[i].py   + 0.5);
      software_lines[num_buffered_lines].y1 = qftoi(verts[i+1].py + 0.5);
    }
    
    software_lines[num_buffered_lines].start_color = start_color;
    software_lines[num_buffered_lines].end_color   = end_color;
    num_buffered_lines++;
  }

  stop_trunc();
}

void r1_software_class::render_pixel(r1_vert *pixel)
{
  //heh
}

void r1_software_class::set_constant_color(w32 color)
{
  const_color = color;
  
  w32 r = (const_color & 0x00FF0000) >> 16;
  w32 g = (const_color & 0x0000FF00) >> 8;
  w32 b = (const_color & 0x000000FF);

  r = r >> (8-fmt.red_bits);
  g = g >> (8-fmt.green_bits);
  b = b >> (8-fmt.blue_bits);

  cur_color = (r<<fmt.red_shift) | (g<<fmt.green_shift) | (b<<fmt.blue_shift);
  cur_color = cur_color | (cur_color << 16);
}


void tri_area_struct::calc_area(float x0, float x1, float x2,
                                float y0, float y1, float y2)
{
#ifdef USE_ASM
  _asm
  {
    mov esi,dword ptr [this]

    fld dword ptr [x0]
    fld dword ptr [x1]
    fsub st(0),st(1) //st(0) = x1 - x0

    fld dword ptr [y0]
    fld dword ptr [y1]
    fsub st(0),st(1) //st(0) = y1 - y0

    //(y1-y0) y0 (x1-x0) x0
    fld dword ptr [x2]
    
    //x2 (y1-y0) y0 (x1-x0) x0
    fsub st(0),st(4)  //st(0) = x2 - x0

    //(x2-x0) (y1-y0) y0 (x1-x0) x0
    fld dword ptr [y2]
    
    //y2 (x2-x0) (y1-y0) y0 (x1-x0) x0
    fsub st(0),st(3) //st(0) = y2 - y0

    //(y2-y0) (x2-x0) (y1-y0) y0 (x1-x0) x0
    fxch st(4)
    
    //(x1-x0) (x2-x0) (y1-y0) y0 (y2-y0) x0
    fst dword ptr [esi]tri_area_struct.dx1x0
    fxch st(4)
    
    //(y2-y0) (x2-x0) (y1-y0) y0 (x1-x0) x0
    fst dword ptr [esi]tri_area_struct.dy2y0

    fmulp st(4),st(0)
    //(x2-x0) (y1-y0) y0 (y2-y0)*(x1-x0) x0

    fst dword ptr [esi]tri_area_struct.dx2x0
    fxch st(1)
    fst dword ptr [esi]tri_area_struct.dy1y0
    fmulp st(1),st(0)
    
    //(x2-x0)*(y1-y0) y0 (y2-y0)*(x1-x0) x0
    fxch st(1)

    //y0 (x2-x0)*(y1-y0) (y2-y0)*(x1-x0) x0
    fstp st(0)
    
    //(x2-x0)*(y1-y0) (y2-y0)*(x1-x0) x0
    fxch st(2)
    fstp st(0)

    //(y2-y0)*(x1-x0) (x2-x0)*(y1-y0)
    fsubp st(1),st(0)
    fstp dword ptr [esi+TRI_AREA_STRUCT_AREA]    
  }
#else  
  dx1x0 = x1 - x0;
  dx2x0 = x2 - x0;
  dy1y0 = y1 - y0;
  dy2y0 = y2 - y0;
  area = (float)(dx2x0*dy1y0) - (float)(dx1x0*dy2y0);
#endif
}

void tri_area_struct::calc_area(sw32 x0, sw32 x1, sw32 x2,
                                sw32 y0, sw32 y1, sw32 y2)
{
#ifdef USE_ASM
  _asm
  {
    mov esi,dword ptr [this]

    fild dword ptr [x0]
    fild dword ptr [x1]
    fsub st(0),st(1) //st(0) = x1 - x0

    fild dword ptr [y0]
    fild dword ptr [y1]
    fsub st(0),st(1) //st(0) = y1 - y0

    //(y1-y0) y0 (x1-x0) x0
    fild dword ptr [x2]
    
    //x2 (y1-y0) y0 (x1-x0) x0
    fsub st(0),st(4)  //st(0) = x2 - x0

    //(x2-x0) (y1-y0) y0 (x1-x0) x0
    fild dword ptr [y2]
    
    //y2 (x2-x0) (y1-y0) y0 (x1-x0) x0
    fsub st(0),st(3) //st(0) = y2 - y0

    //(y2-y0) (x2-x0) (y1-y0) y0 (x1-x0) x0
    fxch st(4)
    
    //(x1-x0) (x2-x0) (y1-y0) y0 (y2-y0) x0
    fst dword ptr [esi]tri_area_struct.dx1x0
    fxch st(4)
    
    //(y2-y0) (x2-x0) (y1-y0) y0 (x1-x0) x0
    fst dword ptr [esi]tri_area_struct.dy2y0

    fmulp st(4),st(0)
    //(x2-x0) (y1-y0) y0 (y2-y0)*(x1-x0) x0

    fst dword ptr [esi]tri_area_struct.dx2x0
    fxch st(1)
    fst dword ptr [esi]tri_area_struct.dy1y0
    fmulp st(1),st(0)
    
    //(x2-x0)*(y1-y0) y0 (y2-y0)*(x1-x0) x0
    fxch st(1)

    //y0 (x2-x0)*(y1-y0) (y2-y0)*(x1-x0) x0
    fstp st(0)
    
    //(x2-x0)*(y1-y0) (y2-y0)*(x1-x0) x0
    fxch st(2)
    fstp st(0)

    //(y2-y0)*(x1-x0) (x2-x0)*(y1-y0)
    fsubp st(1),st(0)
    fstp dword ptr [esi+TRI_AREA_STRUCT_AREA]    
  }
#else  
  dx1x0 = x1 - x0;
  dx2x0 = x2 - x0;
  dy1y0 = y1 - y0;
  dy2y0 = y2 - y0;
  area = (float)(dx2x0*dy1y0) - (float)(dx1x0*dy2y0);
#endif
}


void r1_software_class::update_tri_function()
{
  //based on the current state, determines what type (SPAN_TRI_<something>) is
  //going to be drawn. this type will then be used as an index into the arrays
  //of function pointers (initialized in mappers.cc) to call functions that
  //handle that type of polygon / triangle

  if (texture_mode)
  {    
    if (alpha_mode==R1_ALPHA_DISABLED)
    {
      if (shade_mode==R1_SHADE_DISABLED)
      {        
        if (holy_mode)
        {
          small_poly_type = SPAN_TRI_AFFINE_UNLIT_HOLY;
          big_poly_type   = SPAN_TRI_PERSPECTIVE_UNLIT_HOLY;
        }
        else
        if (alphatexture_mode)
        {
          small_poly_type = SPAN_TRI_AFFINE_UNLIT_ALPHA;
          big_poly_type   = SPAN_TRI_PERSPECTIVE_UNLIT_ALPHA;
        }
        else
        {
          small_poly_type = SPAN_TRI_AFFINE_UNLIT;
          big_poly_type   = SPAN_TRI_PERSPECTIVE_UNLIT;        
        }        
      }
      else
      if (shade_mode==R1_CONSTANT_SHADING)
      {        
        small_poly_type = SPAN_TRI_SOLID_FILL;
        big_poly_type   = SPAN_TRI_SOLID_FILL;        
      }
      else
      {        
        if (holy_mode)        
        {
          small_poly_type = SPAN_TRI_AFFINE_UNLIT_HOLY;
          big_poly_type   = SPAN_TRI_PERSPECTIVE_UNLIT_HOLY;
        }
        else
        if (alphatexture_mode)
        {
          small_poly_type = SPAN_TRI_AFFINE_UNLIT_ALPHA;
          big_poly_type   = SPAN_TRI_PERSPECTIVE_UNLIT_ALPHA;
        }
        else
        {
          small_poly_type = SPAN_TRI_AFFINE_LIT;
          big_poly_type   = SPAN_TRI_PERSPECTIVE_LIT;
        }        
      }
    }
    else
    {
      //need a tmapper that does alpha
      if (holy_mode)
      {
        small_poly_type   = SPAN_TRI_AFFINE_UNLIT_HOLY_BLEND;
        big_poly_type     = SPAN_TRI_PERSPECTIVE_UNLIT_HOLY;
      }
      else
      if (alphatexture_mode)
      {
        small_poly_type = SPAN_TRI_AFFINE_UNLIT_ALPHA;
        big_poly_type   = SPAN_TRI_PERSPECTIVE_UNLIT_ALPHA;
      }
      else
      {
        small_poly_type   = SPAN_TRI_AFFINE_UNLIT;
        big_poly_type     = SPAN_TRI_PERSPECTIVE_UNLIT;        
      }
    }       
  }
  else
  {
    if (alpha_mode==R1_ALPHA_DISABLED)
    {
      small_poly_type = SPAN_TRI_SOLID_FILL;
      big_poly_type   = SPAN_TRI_SOLID_FILL;      
    }
    else
    {
      switch(shade_mode)
      {
      case R1_SHADE_DISABLED:
      case R1_WHITE_SHADING:
        small_poly_type = SPAN_TRI_UNDEFINED;
        big_poly_type   = SPAN_TRI_UNDEFINED;        
        break;

      case R1_CONSTANT_SHADING:
      case R1_COLORED_SHADING:
        small_poly_type = SPAN_TRI_SOLID_BLEND;
        big_poly_type   = SPAN_TRI_SOLID_BLEND;        
        break;
      }
    }
  }
}

void r1_software_class::modify_features(w32 feature_bits, int on)
{
  if (feature_bits & R1_SPANS)
  {
    if (on)
      use_spans = i4_T;
    else
      use_spans = i4_F;
  }

  if (feature_bits & R1_PERSPECTIVE_CORRECT)
  {
    if (on)
      do_perspective = i4_T;
    else
      do_perspective = i4_F;
  }

  if (feature_bits & R1_LOCK_CHEAT)
  {
    if (on)
      lock_cheat = i4_T;
    else
      lock_cheat = i4_F;
  }

  if (feature_bits & R1_Z_BIAS)
  {
    //maybe re-implement this?

    //if (on)
    //  z_bias = 0.f;//0.01f;
    //else
    //  z_bias = 0.f;
  }
}

i4_bool r1_software_class::is_feature_on(w32 feature_bits)
{
  if (feature_bits & R1_SPANS) return use_spans;
  if (feature_bits & R1_PERSPECTIVE_CORRECT) return do_perspective;
  if (feature_bits & R1_LOCK_CHEAT) return lock_cheat;
  if (feature_bits & R1_Z_BIAS)
  {
    //if (z_bias!=0.f)
    //  return i4_T;
    //else
    //  return i4_F;
  }
  return i4_F;
}
