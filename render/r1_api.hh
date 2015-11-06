/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef R1_RENDER_API_HH
#define R1_RENDER_API_HH

#include "tex_id.hh"
#include "video/display.hh"
#include "r1_vert.hh"
#include "tnode.hh"

typedef struct
{
  i4_float r,g,b;
} r1_color_tint;

typedef w32 r1_color_tint_handle;

enum r1_alpha_type 
{ 
  R1_ALPHA_DISABLED,
  R1_ALPHA_CONSTANT,          // use the constant color's alpha
  R1_ALPHA_LINEAR             // use the alpha specified in the verts
};

enum r1_shading_type
{
  R1_SHADE_DISABLED,          // routines will not light
  R1_CONSTANT_SHADING,        // routines will use the constant color
  R1_WHITE_SHADING,           // routines will use only the red component (as white)
  R1_COLORED_SHADING          // routines will use r,g, and b
};

enum
{
  R1_WRITE_COLOR     = 1,    // write pixels to frame buffer
  R1_WRITE_W         = 2,    // write w values to depth buffer
  R1_COMPARE_W       = 4     // if compare is off w then the depth buffer is not consulted
};
typedef w32 r1_write_mask_type;

enum r1_filter_type
{
  R1_NO_FILTERING,
  R1_BILINEAR_FILTERING
};

enum r1_expand_type 
{ 
  R1_COPY_1x1,               // 1 - 1 pixel copy  (render size = w,h)
  R1_COPY_2x2,               // 2 - 1 pixel blow up (render size = w/2, h/2)
  R1_COPY_1x1_SCANLINE_SKIP  // skips a scan line on each row (render size = w, h/2)
}; 

enum r1_render_flags
{
  R1_SOFTWARE=1
};

enum r1_feature_flags
{
  R1_SPANS=1,
  R1_PERSPECTIVE_CORRECT=2,
  R1_LOCK_CHEAT=4,
  R1_Z_BIAS=8, //everything drawn after this is turned on is more likely to be drawn 
               //in front of things drawn before it was turned on
  R1_ALL_FEATURES = R1_SPANS | R1_PERSPECTIVE_CORRECT | R1_LOCK_CHEAT | R1_Z_BIAS
};

class r1_last_node;
class r1_texture_manager_class;
class r1_miplevel_t;

class r1_texture_manager;
class g1_texture_node_struct;
class r1_render_window_class;      // defined in r1_win.hh

class r1_render_api_class
{
protected:
  friend r1_render_api_class *r1_get_api(i4_display_class *for_display);
  friend r1_render_api_class *r1_create_api(i4_display_class *for_display, char *name);

  friend void r1_destroy_api(r1_render_api_class *r);  

  r1_texture_manager_class *tmanager;

  // make these global so they can be accessed without pointer indirection (faster)
  static r1_miplevel_t          *last_node;
  static r1_shading_type         shade_mode;
  static r1_alpha_type           alpha_mode;
  static r1_write_mask_type      write_mask;
  static w32                     const_color;
  static r1_filter_type          filter_mode;
  static r1_render_api_class    *first;  
  
  static i4_float               r_tint_mul;
  static i4_float               g_tint_mul;
  static i4_float               b_tint_mul;
  static i4_bool                color_tint_on;

  enum { MAX_COLOR_TINTS = 32 };

  static r1_color_tint          color_tint_list[MAX_COLOR_TINTS];
  static sw8                    num_color_tints;

  r1_render_api_class           *next;
  
  //currently the only flag is R1_SOFTWARE
  w32                           render_device_flags;

  // returns false if display is not compatible with render_api, i.e. if you pass
  // the directx display to the glide render api it return false
  // init will create the texture manager, which can be used after this call
  // text_mem_size if size of buffer to hold compressed textures (in system memory)
  virtual i4_bool init(i4_display_class *display);

  // this will delete the texture manager (and free textures associated with) created by init
  virtual void uninit();

  virtual void copy_part(i4_image_class *im,                                          
                         int x, int y,             // position on screen
                         int x1, int y1,           // area of image to copy 
                         int x2, int y2) = 0;

public:
  w32                     get_render_device_flags()           { return render_device_flags; }
  r1_shading_type         get_shade_mode()                    { return shade_mode; }
  r1_alpha_type           get_alpha_mode()                    { return alpha_mode; }
  r1_write_mask_type      get_write_mask()                    { return write_mask; }
  w32                     get_constant_color()                { return const_color; }
  r1_filter_type          get_filter_mode()                   { return filter_mode; }

  r1_render_api_class() 
  {
    tmanager=0;
    render_device_flags=0;
    next=first;
    first=this;
  }

  static i4_draw_context_class  *context;

  virtual i4_bool pixel_double() { return i4_F; }

  r1_texture_manager_class *get_tmanager() { return tmanager; }  // created by init()

  // texture handle is obtained from the texture manager, this is enables texture mapping
  virtual void use_texture(r1_texture_handle material_ref, 
                           sw32 desired_width,
                           w32 frame)                                                 = 0;

  // drawing will the constant color to render with if textures are disabled
  virtual void disable_texture()                                                      = 0;

  //tints all r g b values (multiplies)
  virtual r1_color_tint_handle register_color_tint(i4_float r, i4_float g, i4_float b);
  
  //calling this with a handle of 0 disables tinting
  virtual void set_color_tint(r1_color_tint_handle color_tint_handle);

  // constant color is used by line drawing, and poly draws-if disable_texture 
  virtual void set_constant_color(w32 color)                          { const_color=color; }
  virtual void set_shading_mode(r1_shading_type type)                 { shade_mode=type;   }
  virtual void set_alpha_mode(r1_alpha_type type)                     { alpha_mode=type;   }
  virtual void set_write_mode(r1_write_mask_type mask)                { write_mask=mask;   }
  virtual void set_filter_mode(r1_filter_type type)                   { filter_mode=type;  }

  // api will scale all z's (or w's) to reflect this range with best percision
  // near_z must be greater than 0
  virtual void set_z_range(float near_z, float far_z)                                 = 0;
  // make sure this updates the following variables:
  // r1_near_clip_z
  // r1_far_clip_z

  virtual void render_poly(int t_verts, r1_vert *verts)                               = 0;
  virtual void render_poly(int t_verts, r1_vert *verts, int *vertex_index);
  virtual void render_poly(int t_verts, r1_vert *verts, w16 *vertex_index);

  //this had better be a rectangle
  virtual void render_sprite(r1_vert *verts);

  virtual void render_pixel(r1_vert *pixel)                                           = 0;
  virtual void render_lines(int t_lines, r1_vert *verts )                             = 0;

  // color is standard argb, (z should be within range specifed by set_z_range)
  virtual void clear_area(int x1, int y1, int x2, int y2, w32 color, float z);
  
  // creates an image of the same bit depth and palette of screen (for use with put_image)
  virtual i4_image_class *create_compatible_image(w16 w, w16 h)                       = 0;

  // these should be called for an image before drawing to them
  virtual void lock_image(i4_image_class *im) { ; } 
  virtual void unlock_image(i4_image_class *im) { ; }

  // this function does clipping (based on context) and calls copy_part
  virtual void put_image(i4_image_class *im,                                          
                         int x, int y,             // position on screen
                         int x1, int y1,           // area of image to copy 
                         int x2, int y2);

  virtual r1_vert *clip_poly(sw32 *num_clip_verts, //how many verts in this polygon
                             r1_vert *t_vertices,  //pointer to the vertices
                             w16 *indices,         //pointer to the indices
                             r1_vert *clip_buf_1,  //pointer to clip buffer 1
                             r1_vert *clip_buf_2,  //pointer to clip buffer 2
                             w8 flags);            //flags to be considered when clipping
    
  virtual r1_vert *clip_poly(sw32 *num_clip_verts, //how many verts in this polygon
                             r1_vert *t_vertices,  //pointer to the vertices
                             w32 *indices,         //pointer to the indices
                             r1_vert *clip_buf_1,  //pointer to clip buffer 1
                             r1_vert *clip_buf_2,  //pointer to clip buffer 2
                             w8 flags);            //flags to be considered when clipping

  // creates a window that rendering can occur in
  // visable_w and & h is the area the window takes up on the actual screen
  // expand type will determine the size you can actually render to  

  virtual r1_render_window_class *create_render_window(int visable_w, int visable_h,
                                                       r1_expand_type type=R1_COPY_1x1) = 0;


  virtual i4_bool expand_type_supported(r1_expand_type type) 
  { return (i4_bool)(type==R1_COPY_1x1); }

  virtual void modify_features(w32 feature_bits, int on) { ; }
  virtual i4_bool is_feature_on(w32 feature_bits) { return i4_F; }

  virtual void default_state()
  {
    disable_texture();
    set_constant_color(0xffffff);
    set_z_range(0.001,10000);    
    set_shading_mode(R1_COLORED_SHADING);
    set_write_mode(R1_WRITE_COLOR | R1_WRITE_W | R1_COMPARE_W);
    set_alpha_mode(R1_ALPHA_DISABLED);    
    set_filter_mode(R1_NO_FILTERING);
    set_color_tint(0);

    modify_features(R1_ALL_FEATURES, 0);         // turn off these other features
    modify_features(R1_SPANS, 1);
    modify_features(R1_PERSPECTIVE_CORRECT, 1);
  }

  virtual char *name() = 0;
};

extern r1_render_api_class *r1_render_api_class_instance;

r1_render_api_class *r1_create_api(i4_display_class *for_display, char *api_name=0);

void r1_destroy_api(r1_render_api_class *render_api);

inline i4_float r1_ooz(i4_float z) { return 0.9999/z; }

#endif
