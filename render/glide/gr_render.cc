/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "glide/gr_vram.hh"
#include "image/image.hh"
#include "r1_api.hh"
#include "time/profile.hh"
#include "video/glide/glide_display.hh"
#include "r1_clip.hh"
#include "r1_win.hh"
#include "video/glide/glide.h"
#include "device/processor.hh"

#ifdef _WINDOWS
#include "software/amd3d/amd3d.h"
#define USE_AMD3D
#endif

static i4_bool gr_support_amd3d = i4_F;

static float gr_const_r, gr_const_g, gr_const_b, gr_const_a, gr_x2, gr_y2;

//dont reorder this
static struct gr_static_struct
{
  float gr_xadd, gr_yadd;    // window offset additions
  float gr_smul, gr_tmul;    // texture coordinate multiplers
  float gr_r_tint_mul, gr_g_tint_mul;
  float gr_a_mul, gr_w_mul;            // scales the w value to best fit application's range  
  float VTX_SNAP_1, VTX_SNAP_2;
  float gr_b_tint_mul;
  float gr_xoff, gr_yoff;    // window offset additions
} gr_static_info = {0.f, 0.f, 0.f, 0.f, 1.f, 1.f, 255.f, 1.f, (float)(1L<<19), (float)(1L<<19), 1.f };

static i4_profile_class pf_glide_single_poly("glide render single poly"); 
static i4_profile_class pf_glide_shadows("glide shadows"); 
static i4_profile_class pf_glide_sprite("glide sprite"); 
static i4_profile_class pf_render_list("glide render list"); 
static i4_profile_class pf_use_texture("glide use texture");
static i4_profile_class pf_begin_render("glide begin render");
static i4_profile_class pf_end_render("glide end render");
static i4_profile_class pf_vram_select_texture("glide select texture");
static i4_profile_class pf_setup_vert("glide setup vert");


typedef i4_bool (*gr_copy_vert_function_type)(GrVertex *dst, r1_vert *src, int t_verts);
gr_copy_vert_function_type gr_copy_vert_function=0;

w32 gr_old_flags;

#ifdef _WINDOWS
inline w32 gset_and_get_flags() // returns old flags
{
  w32 control_word_new;	
  w32 control_word_old;

  _asm {
    finit
    fwait
    fnstcw    control_word_old ;
    fwait
    mov       eax, control_word_old ;      
    and       eax, 0xfffffcff ;
    mov       control_word_new, eax ;
    fldcw     control_word_new ;
    fwait
  }
  return control_word_old;
}

inline void grestore_flags(w32 old_value) // returns old flags
{
  _asm {
    fldcw old_value ;
  }
}

inline void gsnap_vert(GrVertex *dst)
{
  dst->x += gr_static_info.VTX_SNAP_1;
  dst->x -= gr_static_info.VTX_SNAP_1;

  dst->y += gr_static_info.VTX_SNAP_1;
  dst->y -= gr_static_info.VTX_SNAP_1;
}

#else

inline w32 gset_and_get_flags()
{
  return 0;
}

inline void grestore_flags(w32 old)
{
}


inline void gsnap_vert(GrVertex *dst)
{
  dst->x = float(int(dst->x * 16.0))/16.;
  dst->y = float(int(dst->y * 16.0))/16.;
}

#endif

 
#if 1

inline i4_bool gr_check_vert(GrVertex *dst) { return i4_T; }

#else
inline i4_bool gr_check_vert(GrVertex *dst)
{
  if (dst->x<gr_xoff || dst->y<gr_yoff || dst->x>gr_x2 || dst->y>gr_y2)
    return i4_F;
  else
    return i4_T;
}
#endif

i4_bool gr_copy_all_amd3d(GrVertex *dst, r1_vert *src, int t_verts);

i4_bool gr_copy_all(GrVertex *dst, r1_vert *src, int t_verts)
{
  while (t_verts--)
  {
    float oow = src->w * gr_static_info.gr_w_mul;

    dst->x = src->px + gr_static_info.gr_xoff;
    dst->y = src->py + gr_static_info.gr_yoff;
    gsnap_vert(dst);
             
    dst->oow = oow;

    dst->r = src->r * gr_static_info.gr_r_tint_mul;
    dst->g = src->g * gr_static_info.gr_g_tint_mul;
    dst->b = src->b * gr_static_info.gr_b_tint_mul;

    dst->tmuvtx[0].sow = src->s * gr_static_info.gr_smul * oow;
    dst->tmuvtx[0].tow = src->t * gr_static_info.gr_tmul * oow;
    dst->tmuvtx[0].oow = oow;

    dst->a = src->a*255.f;
    
    //if (!gr_check_vert(dst))
    //{      
    //  i4_warning("vert out of window");
    //  return i4_F;
    //}    

    dst++;
    src++;    
  }

  return i4_T;
}


// this is used if R1_ALPHA_DISABLED & R1_WHITE_SHADING
i4_bool gr_copy_white_shade(GrVertex *dst, r1_vert *src, int t_verts)
{
  while (t_verts--)
  {
    float oow = src->w * gr_static_info.gr_w_mul;

    dst->x=src->px + gr_static_info.gr_xoff;
    dst->y=src->py + gr_static_info.gr_yoff;
    gsnap_vert(dst);

    dst->oow=oow;

    float base_color=src->r;
    dst->r=base_color * gr_static_info.gr_r_tint_mul;
    dst->g=base_color * gr_static_info.gr_g_tint_mul;
    dst->b=base_color * gr_static_info.gr_b_tint_mul;

    dst->tmuvtx[0].sow=src->s * gr_static_info.gr_smul * oow;
    dst->tmuvtx[0].tow=src->t * gr_static_info.gr_tmul * oow;
    dst->tmuvtx[0].oow=oow;

    dst->a=src->a * 255;


    if (!gr_check_vert(dst))
    {
      i4_warning("vert out of window");
      return i4_F;
    }

    dst++;
    src++;    

  }
  return i4_T;
}

i4_bool gr_copy_fullbright_shade(GrVertex *dst, r1_vert *src, int t_verts)
{
  while (t_verts--)
  {
    float oow = src->w * gr_static_info.gr_w_mul;

    dst->x=src->px + gr_static_info.gr_xoff;
    dst->y=src->py + gr_static_info.gr_yoff;
    gsnap_vert(dst);

    dst->oow=oow;

    dst->r = gr_static_info.gr_r_tint_mul;
    dst->g = gr_static_info.gr_g_tint_mul;
    dst->b = gr_static_info.gr_b_tint_mul;

    dst->tmuvtx[0].sow=src->s * gr_static_info.gr_smul * oow;
    dst->tmuvtx[0].tow=src->t * gr_static_info.gr_tmul * oow;
    dst->tmuvtx[0].oow=oow;

    dst->a=src->a * 255;

    if (!gr_check_vert(dst))
    {
      i4_warning("vert out of window");
      return i4_F;
    }

    dst++;
    src++;    
  }
  return i4_T;
}



i4_bool gr_copy_no_shade(GrVertex *dst, r1_vert *src, int t_verts)
{
  while (t_verts--)
  {
    float oow = src->w * gr_static_info.gr_w_mul;

    dst->x=src->px + gr_static_info.gr_xoff;
    dst->y=src->py + gr_static_info.gr_yoff;
    gsnap_vert(dst);

    dst->oow=oow;

    dst->r = gr_const_r * gr_static_info.gr_r_tint_mul;
    dst->g = gr_const_g * gr_static_info.gr_g_tint_mul;
    dst->b = gr_const_b * gr_static_info.gr_b_tint_mul;

    dst->tmuvtx[0].sow=src->s * gr_static_info.gr_smul * oow;
    dst->tmuvtx[0].tow=src->t * gr_static_info.gr_tmul * oow;
    dst->tmuvtx[0].oow=oow;

    dst->a=src->a * 255;

    if (!gr_check_vert(dst))
    {
      i4_warning("vert out of window");
      return i4_F;
    }

    dst++;
    src++;    
  }
  return i4_T;
}



class r1_glide_render_window_class : public r1_render_window_class
{
public:
  r1_glide_render_window_class *next_rw;

  i4_bool need_flip_update;

  r1_glide_render_window_class(w16 w, w16 h,
                               r1_expand_type expand_type,
                               r1_render_api_class *api);

  ~r1_glide_render_window_class();

  void draw(i4_draw_context_class &context);

  void request_redraw(i4_bool for_a_child)
  {
    if (!for_a_child)
    {
      if (children.begin()!=children.end())
        children.begin()->request_redraw(i4_F);
    }

    need_flip_update=i4_T;

    r1_render_window_class::request_redraw(for_a_child);
  }

  char *name() { return  "glide render window"; }
};


r1_glide_render_window_class *r1_render_window_list=0;


r1_glide_render_window_class::r1_glide_render_window_class(w16 w, w16 h,
                                                           r1_expand_type expand_type,
                                                           r1_render_api_class *api)
  : r1_render_window_class(w,h, expand_type, api)
{  
  need_flip_update=i4_T;
  next_rw=r1_render_window_list;
  r1_render_window_list=this;
}

r1_glide_render_window_class::~r1_glide_render_window_class()
{
  if (this==r1_render_window_list)
    r1_render_window_list=next_rw;
  else
  {
    r1_glide_render_window_class *last=0;
    for (r1_glide_render_window_class *p=r1_render_window_list; p && p!=this;)
    {
      last=p;
      p=p->next_rw;
    }
    last->next_rw=next_rw;
  }
}


class r1_glide_render_class : public r1_render_api_class
{
  static i4_display_class       *display;
  
  r1_write_mask_type pre_holy_write_mask;
  r1_alpha_type      pre_holy_alpha_mode;

  i4_bool texture_mode;
  i4_bool holy_mode;
  i4_bool holy_mode_lock;
  
public:
  r1_glide_render_class *next_rw;

  enum { MAX_VERTS=32 };
  char *name() { return "glide"; }

  virtual void copy_part(i4_image_class *im,                                          
                         int x, int y,             // position on screen
                         int x1, int y1,           // area of image to copy 
                         int x2, int y2)
  {
    if (i4_glide_display)
    {
      i4_image_class *bbuf;
      bbuf=i4_glide_display->lock_frame_buffer(I4_BACK_FRAME_BUFFER, I4_FRAME_BUFFER_WRITE);
      if (bbuf)
      {
        im->put_part(bbuf, i4_f_to_i(gr_static_info.gr_xoff) + x, 
                      i4_f_to_i(gr_static_info.gr_yoff) + y,x1,y1,x2,y2,
                      *context);
        i4_glide_display->unlock_frame_buffer(I4_BACK_FRAME_BUFFER);
      }
      
    }


  }




  void enable_holy()
  {    
    if (!holy_mode && !holy_mode_lock)
    {          
      holy_mode_lock = i4_T;

      pre_holy_write_mask = get_write_mask();
      pre_holy_alpha_mode = get_alpha_mode();

      set_write_mode(R1_COMPARE_W | R1_WRITE_COLOR);

      holy_mode = i4_T;

      set_alpha_mode(R1_ALPHA_LINEAR);

      holy_mode_lock = i4_F;
    }
  }

  void disable_holy()
  {
    if (holy_mode && !holy_mode_lock)
    {      
      holy_mode_lock = i4_T;

      set_write_mode(pre_holy_write_mask);
      
      holy_mode=i4_F;
      
      set_alpha_mode(pre_holy_alpha_mode);
      
      holy_mode_lock = i4_F;
    }
  }


  virtual void set_z_range(float _near_z, float _far_z)
  {    
    r1_near_clip_z = _near_z;
    r1_far_clip_z  = _far_z;

    gr_static_info.gr_w_mul = _far_z / (float)GR_WDEPTHVALUE_FARTHEST;
  }


  r1_color_tint_handle register_color_tint(i4_float r, i4_float g, i4_float b)
  {
    if (num_color_tints==MAX_COLOR_TINTS) return 0;

    color_tint_list[num_color_tints].r = r * 255.f;
    color_tint_list[num_color_tints].g = g * 255.f;
    color_tint_list[num_color_tints].b = b * 255.f;

    num_color_tints++;

    return num_color_tints-1;
  }


  void set_color_tint(r1_color_tint_handle c)
  {
    if (c==0)
    {
      gr_static_info.gr_r_tint_mul = 255.f;
      gr_static_info.gr_g_tint_mul = 255.f;
      gr_static_info.gr_b_tint_mul = 255.f;

      color_tint_on = i4_F;
    }
    else
    {
      color_tint_on = i4_T;
      
      gr_static_info.gr_r_tint_mul = color_tint_list[c].r;
      gr_static_info.gr_g_tint_mul = color_tint_list[c].g;
      gr_static_info.gr_b_tint_mul = color_tint_list[c].b;
    }
  }


  virtual void set_shading_mode(r1_shading_type type) 
  { 
    if (type!=shade_mode)
    {
      if (type==R1_SHADE_DISABLED)
        gr_copy_vert_function=gr_copy_fullbright_shade;
      else if (type==R1_CONSTANT_SHADING)
      {
        gr_const_r=((const_color&0xff0000)>>16)/255.0;
        gr_const_g=((const_color&0xff00)>>8)/255.0;
        gr_const_b=((const_color&0xff)>>0)/255.0;
        gr_copy_vert_function=gr_copy_no_shade;
      }
      else if (type==R1_WHITE_SHADING)
        gr_copy_vert_function=gr_copy_white_shade;
      else
      {
        if (!gr_support_amd3d)
          gr_copy_vert_function = gr_copy_all;
        else
          gr_copy_vert_function = gr_copy_all_amd3d;
      }

      r1_render_api_class::set_shading_mode(type);
    }
  }

  virtual void set_constant_color(w32 color)
  { 
    if (color!=get_constant_color())
    {
      r1_render_api_class::set_constant_color(color);

      switch (shade_mode)
      {
        case R1_SHADE_DISABLED :
          gr_copy_vert_function=gr_copy_fullbright_shade;
          break;

        case R1_CONSTANT_SHADING :
        {
          gr_const_r=((const_color&0xff0000)>>16)/255.0;
          gr_const_g=((const_color&0xff00)>>8)/255.0;
          gr_const_b=((const_color&0xff)>>0)/255.0;
          gr_copy_vert_function=gr_copy_no_shade;
        } break;
        
        case R1_WHITE_SHADING :
          gr_copy_vert_function=gr_copy_white_shade;
          break;
          
        case R1_COLORED_SHADING :
          if (!gr_support_amd3d)
            gr_copy_vert_function = gr_copy_all;
          else
            gr_copy_vert_function = gr_copy_all_amd3d;
          break;
      }

      grConstantColorValue(color);
      //grAlphaTestReferenceValue(color>>24);
    }
  }

  r1_glide_render_class()
  {
    holy_mode      = i4_F;
    holy_mode_lock = i4_F;
    last_node=0;
    write_mask=R1_COMPARE_W | R1_WRITE_W | R1_WRITE_COLOR;
    shade_mode=R1_SHADE_DISABLED;
    alpha_mode=R1_ALPHA_DISABLED;
    texture_mode=i4_T;

    //currently, only flag is R1_SOFTWARE (and this is hardware)
    render_device_flags=0;
  }

  r1_glide_vram_class *vram() { return (r1_glide_vram_class *)tmanager; }
  

  // returns false if display is not compatible with render_api, i.e. if you pass
  // the directx display to the glide render api it return false
  // init will create the texture manager, which can be used after this call
  virtual i4_bool init(i4_display_class *_display)
  {
    if (_display==i4_glide_display)
    {
      display=_display;

      tmanager=new r1_glide_vram_class(display->get_palette());

#ifdef USE_AMD3D
      i4_cpu_info_struct s;
      i4_get_cpu_info(&s);

      if (s.cpu_flags & i4_cpu_info_struct::AMD3D)
        gr_support_amd3d = i4_T;
      
#endif
      if (!gr_support_amd3d)
        gr_copy_vert_function = gr_copy_all;
      else
        gr_copy_vert_function = gr_copy_all_amd3d;


      return i4_T;
    }
    else return i4_F;
  }

  // texture handle is obtained from the texture manager, this is enables texture mapping
  virtual void use_texture(r1_texture_handle material_ref, sw32 desired_width, w32 frame)
  {
    pf_use_texture.start();

    if (!tmanager->valid_handle(material_ref))
    {
      disable_texture();
      pf_use_texture.stop();
      return;
    }
    else
    if (!texture_mode)
    {
      guColorCombineFunction(GR_COLORCOMBINE_TEXTURE_TIMES_ITRGB);
      texture_mode = i4_T;
    }    

    sw32 width,height;    

    r1_miplevel_t *mip = tmanager->get_texture(material_ref, frame, desired_width, width, height);

    // don't select the texture again if it's the same one we used last time
    if (mip)
    {        
      i4_bool is_alpha = mip->entry->is_alphatexture();
      i4_bool is_holy  = mip->entry->is_transparent();

      if (is_alpha || is_holy)      
        enable_holy();      
      else
        disable_holy();

      if (mip != last_node)
      {
        last_node = mip;

        pf_vram_select_texture.start();
        vram()->select_texture(mip->vram_handle, gr_static_info.gr_smul, gr_static_info.gr_tmul, is_holy, is_alpha);
        pf_vram_select_texture.stop();
      }
    }
    
    pf_use_texture.stop();
  }


  // drawing will the constant color to render with if textures are disabled
  virtual void disable_texture()
  {
    if (texture_mode)
    {
      guColorCombineFunction(GR_COLORCOMBINE_ITRGB);  // turn off textures
      texture_mode = i4_F;
      last_node    = 0;

      disable_holy();
    }        
  }


  virtual void set_alpha_mode(r1_alpha_type type)
  {
    if (holy_mode)
      disable_holy();

    if (type==get_alpha_mode()) return;

    switch (type)
    {
      case R1_ALPHA_DISABLED :
        grAlphaBlendFunction(GR_BLEND_ONE,GR_BLEND_ZERO,
                             GR_BLEND_ZERO,GR_BLEND_ZERO);

        grAlphaCombine(GR_COMBINE_FUNCTION_ZERO, GR_COMBINE_FACTOR_NONE,
                       GR_COMBINE_LOCAL_NONE,GR_COMBINE_OTHER_NONE,0);

        break;

      case R1_ALPHA_CONSTANT :    // enable alpha
        grAlphaBlendFunction(GR_BLEND_SRC_ALPHA, GR_BLEND_ONE_MINUS_SRC_ALPHA,
                             GR_BLEND_ZERO,GR_BLEND_ZERO);

        if (!holy_mode)
        {
          grAlphaCombine(GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_OTHER_CONSTANT,
                         GR_COMBINE_LOCAL_NONE,GR_COMBINE_OTHER_CONSTANT,0);
        }
        else
        {
          grAlphaCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
                         GR_COMBINE_FACTOR_TEXTURE_ALPHA,
                         GR_COMBINE_LOCAL_NONE,
                         GR_COMBINE_OTHER_CONSTANT,0);
        }
        break;
          
      case R1_ALPHA_LINEAR :
        grAlphaBlendFunction(GR_BLEND_SRC_ALPHA, GR_BLEND_ONE_MINUS_SRC_ALPHA,
                             GR_BLEND_ZERO,GR_BLEND_ZERO);

        if (!holy_mode)
        {
          grAlphaCombine(GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE, 
                         GR_COMBINE_LOCAL_ITERATED, GR_COMBINE_OTHER_NONE, 0);
        }
        else
        {
          grAlphaCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
                         GR_COMBINE_FACTOR_TEXTURE_ALPHA,
                         GR_COMBINE_LOCAL_NONE,
                         GR_COMBINE_OTHER_ITERATED,0);
        }
        break;
    }    

    r1_render_api_class::set_alpha_mode(type);
  }

  virtual void set_filter_mode(r1_filter_type type)
  {
    if (type!=filter_mode)
    {
      //if (type==R1_NO_FILTERING)
      //  grTexFilterMode(GR_TMU0,GR_TEXTUREFILTER_POINT_SAMPLED,GR_TEXTUREFILTER_POINT_SAMPLED);
      //else
      //if (type==R1_BILINEAR_FILTERING)       
        grTexFilterMode(GR_TMU0,GR_TEXTUREFILTER_BILINEAR,GR_TEXTUREFILTER_BILINEAR);
    }

    r1_render_api_class::set_filter_mode(type);
  }

  virtual void set_write_mode(r1_write_mask_type mask)
  {
    if (holy_mode)
      disable_holy();
    
    w32 diff=mask^get_write_mask();      // bits that differ

    if (diff & R1_WRITE_COLOR)  // need to change the color write mask
      grColorMask(mask & R1_WRITE_COLOR ? 1 : 0, 0);  // turn on color writes

    if (diff & (R1_WRITE_W | R1_COMPARE_W))  // change z write or compare?
    {
      switch (mask & (R1_WRITE_W | R1_COMPARE_W))
      {
        case R1_WRITE_W :
          grDepthBufferMode(GR_DEPTHBUFFER_WBUFFER);
          grDepthMask(1);
          grDepthBufferFunction(GR_CMP_ALWAYS);  // write to all z always (no compare)
          break;

        case R1_WRITE_W | R1_COMPARE_W :
          grDepthBufferMode(GR_DEPTHBUFFER_WBUFFER);
          grDepthMask(1);
          grDepthBufferFunction(GR_CMP_LESS);
          break;

        case R1_COMPARE_W :
          grDepthBufferMode(GR_DEPTHBUFFER_WBUFFER);
          grDepthMask(0);
          grDepthBufferFunction(GR_CMP_LESS);
          break;

        case 0 :
          grDepthBufferMode(GR_DEPTHBUFFER_DISABLE);
          grDepthMask(0);
          break;          
      }
    }

    r1_render_api_class::set_write_mode(mask);
  }


  /*
  virtual void render_poly(int t_verts, r1_vert *verts, int *vertex_index)
  {
    pf_setup_vert.start();
    GrVertex g_vert_buf[MAX_VERTS];
    
    for (int i=0; i<t_verts; i++)
    {
      if (!gr_copy_vert_function(g_vert_buf + i, verts+vertex_index[i], 1))
      {
        pf_setup_vert.stop();
        return;
      }
    }
    pf_setup_vert.stop();

    pf_glide_single_poly.start();
    grDrawPolygonVertexList(t_verts, g_vert_buf);
    pf_glide_single_poly.stop();
  }
  */

  virtual void render_poly(int t_verts, r1_vert *verts)
  {
    pf_setup_vert.start();
    
    GrVertex g_vert_buf[MAX_VERTS];
    if (!gr_copy_vert_function(g_vert_buf, verts, t_verts))
    {
      pf_setup_vert.stop();
      return;
    }
    pf_setup_vert.stop();

    pf_glide_single_poly.start();    
    grDrawPolygonVertexList(t_verts, g_vert_buf);
    pf_glide_single_poly.stop();
  }

  virtual void render_pixel(r1_vert *pixel)
  {
    GrVertex p;
    if (gr_copy_vert_function(&p, pixel, 1))
      grDrawPoint(&p);
  }

  virtual void render_lines(int t_lines, r1_vert *verts)
  {
    GrVertex g_vert_buf[MAX_VERTS];
    if (gr_copy_vert_function(g_vert_buf, verts, t_lines+1))
      for (int i=0; i<t_lines; i++)
        grDrawLine(g_vert_buf+i , g_vert_buf+i+1);
  }


  
  // creates an image of the same bit depth and palette of screen (for use with put_image)
  virtual i4_image_class *create_compatible_image(w16 w, w16 h)
  {
    return i4_create_image(w,h,display->get_palette());    
  }



  r1_render_window_class *create_render_window(int visable_w, int visable_h,
                                               r1_expand_type type)
  {
    return new r1_glide_render_window_class(visable_w, visable_h, R1_COPY_1x1, this);
  }

  void clear_area(int x1, int y1, int x2, int y2, w32 color, float z)
  {
    w32 old_const_color=get_constant_color();
    set_constant_color(color);

    r1_shading_type old_shade_mode=get_shade_mode();
    set_shading_mode(R1_CONSTANT_SHADING);

    disable_texture();
    disable_holy();

    r1_vert v[4];

    float w = 1.f/z;

    v[0].px=x1;     v[0].py=y1;
    v[1].px=x1;     v[1].py=y2+1;
    v[2].px=x2+1;   v[2].py=y2+1;
    v[3].px=x2+1;   v[3].py=y1;  
    
    GrVertex g_vert_buf[4];
    if (!gr_copy_vert_function(g_vert_buf, v, 4)) return;
    
    if (z==r1_far_clip_z)
    {
      g_vert_buf[0].oow = GR_WDEPTHVALUE_NEAREST;
      g_vert_buf[1].oow = GR_WDEPTHVALUE_NEAREST;
      g_vert_buf[2].oow = GR_WDEPTHVALUE_NEAREST;
      g_vert_buf[3].oow = GR_WDEPTHVALUE_NEAREST;
    }
    else
    if (z==r1_near_clip_z)
    {
      g_vert_buf[0].oow = GR_WDEPTHVALUE_FARTHEST;
      g_vert_buf[1].oow = GR_WDEPTHVALUE_FARTHEST;
      g_vert_buf[2].oow = GR_WDEPTHVALUE_FARTHEST;
      g_vert_buf[3].oow = GR_WDEPTHVALUE_FARTHEST;
    }
    else
    {
      float oow = (1.f/(float)z) * gr_static_info.gr_w_mul;
      g_vert_buf[0].oow = oow;
      g_vert_buf[1].oow = oow;
      g_vert_buf[2].oow = oow;
      g_vert_buf[3].oow = oow;
    }

    grDrawPolygonVertexList(4, g_vert_buf);
    
    set_shading_mode(old_shade_mode);
    set_constant_color(old_const_color);
  }
};

r1_glide_render_class r1_glide_render;

i4_display_class *r1_glide_render_class::display=0;



void r1_glide_render_window_class::draw(i4_draw_context_class &_context)
{
  r1_render_api_class::context = &_context;

  gr_static_info.gr_xoff = _context.xoff;
  gr_static_info.gr_yoff = _context.yoff;

  gr_static_info.gr_xadd = _context.xoff + gr_static_info.VTX_SNAP_1;
  gr_static_info.gr_yadd = _context.yoff + gr_static_info.VTX_SNAP_1;

  gr_x2 = gr_static_info.gr_xoff + width();
  gr_y2 = gr_static_info.gr_yoff + height();

  pf_begin_render.start();
  clip_with_z(_context);
  pf_begin_render.stop();

  gr_old_flags=gset_and_get_flags();
  r1_render_window_class::draw(_context);     
  grestore_flags(gr_old_flags);
    
  if (need_flip_update)
  {      
    request_redraw(i4_T);
      
    for (win_iter i=children.begin(); i!=children.end(); ++i)
      i->request_redraw(i4_F);

    need_flip_update=i4_F;
  }

  // if there was a texture miss
  if (r1_glide_render.get_tmanager()->texture_resolution_change())
    for (r1_glide_render_window_class *w=r1_render_window_list; w; w=w->next_rw)
      w->request_redraw(i4_F);

}

#ifdef USE_AMD3D
  
w32 r1vtxsize = sizeof(r1_vert);
w32 grvtxsize = sizeof(GrVertex);

i4_bool gr_copy_all_amd3d(GrVertex *dst, r1_vert *src, int t_verts)
{
  _asm
  {
    mov esi,dword ptr [t_verts]
    mov edi,dword ptr [src]

    and esi,esi
    jz  done

    lea ecx, dword ptr [gr_static_info]    
    mov edx,dword ptr [dst]

    movq mm7, qword ptr [ecx]gr_static_struct.gr_xadd        //gr_y_add  | gr_x_add
    mov eax,edi

    movq mm6, qword ptr [ecx]gr_static_struct.gr_smul        //gr_tmul   | gr_smul 
    mov ebx,edx

    movq mm5, qword ptr [ecx]gr_static_struct.gr_r_tint_mul  //gr_g_tint | gr_r_tint
    add eax,dword ptr [r1vtxsize]

    movq mm4, qword ptr [ecx]gr_static_struct.gr_a_mul       //gr_w_mul  | gr_a_mul
    add ebx,dword ptr [grvtxsize]
  
  //while (t_verts--)
  //{
  looper:
    cmp esi,1
    je  dont_prefetch
  
    prefetch (_eax)
    prefetch (_ebx)
  
  dont_prefetch:
    
    movq mm3, qword ptr [edi]r1_vert.a  //src->w  | src->a
    movq mm2, qword ptr [edi]r1_vert.r  //src->g  | src->r
      
    movq mm1, qword ptr [edi]r1_vert.s  //src->t  | src->s 
    movq mm0, qword ptr [edi]r1_vert.px //src->py | src->px
    
    pfmul (m3, m4) //mm3 - src->w * gr_w_mul | src->a * gr_a_mul
    pfmul (m2, m5) //mm2 - src->g * gr_g_mul | src->r * gr_r_mul
    
    pfmul (m1, m6) //mm1 - src->t * gr_tmul  | src->s * gr_smul
    pfadd (m0, m7) //mm0 - src->py + gr_yoff + VTX_SNAP | src->px + gr_xoff + VTX_SNAP
    
    movq qword ptr [edx]GrVertex.a, mm3 //dst->oow = src->w*gr_w_mul | dst->a = src->a * 255.f
    punpckhdq mm3,mm3 //mm3 - oow | oow
    
    movq qword ptr [edx]GrVertex.r, mm2 //dst->r = src->r*gr_r_mul | dst->g = src->g * gr_g_mul
    add eax,dword ptr [r1vtxsize]
    
    movd dword ptr [edx]GrVertex.tmuvtx[0].oow,mm3            
    pfmul (m1, m3) //mm1 - tow | sow
    
    movq mm3,qword ptr [ecx]gr_static_struct.VTX_SNAP_1
    add ebx,dword ptr [grvtxsize]
    
    movd mm2, dword ptr [edi]r1_vert.b  //mm2 - src->b
    movq qword ptr [edx]GrVertex.tmuvtx[0].sow,mm1 //dst->tmuvtx[0].sow = src->s*src->oow;
                                                     //dst->tmuvtx[0].tow = src->t*src->tow
    
    pfsub (m0,m3) //mm0 - src->py + gr_yoff | src->px + gr_xoff
    movd mm3, dword ptr [ecx]gr_static_struct.gr_b_tint_mul //mm3 - gr_b_mul      
    
    movq qword ptr [edx]GrVertex.x,mm0 //dst->x = src->px+VTX_SNAP-VTX_SNAP+gr_x_off
                                       //dst->y = src->py+VTX_SNAP-VTX_SNAP+gr_y_off
    
    pfmul (m2,m3) //mm2 - src->b * gr_b_mul
    add edi,dword ptr [r1vtxsize]
    
    movd dword ptr [edx]GrVertex.b, mm2 //dst->b = src->b * gr_b_mul
    add edx,dword ptr [grvtxsize]
    
    dec esi
    jnz looper
    
    femms
  done:
  }

  return i4_T;
}

#else

i4_bool gr_copy_all_amd3d(GrVertex *dst, r1_vert *src, int t_verts)
{
}
#endif
