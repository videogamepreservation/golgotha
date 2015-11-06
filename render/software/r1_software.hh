/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef _R1_SOFTWARE_HH_
#define _R1_SOFTWARE_HH_

//define this to enable the use of assembly code
#define USE_ASM

//define this to use amd3d assembly or intel assembly (not defining it will use intel)
#define USE_AMD3D

#include "r1_api.hh"
#include "r1_win.hh"
#include "software/r1_software_types.hh"
#include <ddraw.h>

class r1_software_class : public r1_render_api_class
{
public: 
  char *name() { return "Software Span Buffer"; }

  friend class r1_software_render_window_class;

  r1_render_window_class *create_render_window(int w, int h, r1_expand_type expand_type);
  //creates a render_window class (again, i dunno wtf those do)

  i4_bool expand_type_supported(r1_expand_type type);
  //used when creating r1_render_windows, I think. determines if
  //the r1 implementation supports things like double pixel, scanline skip, etc
  //(the software rasterizer supports all of em)

  r1_software_class();
  ~r1_software_class();


  i4_bool init(i4_display_class *display);
  
  void uninit();

  void modify_features(w32 feature_bits, int on);
  //the 'features' are the main way an external application can control
  //exactly what the software rasterizer does. R1_SPANS can be used to turn
  //span buffering on and off (if its off, the faces must be pre-sorted),
  //R1_PERSPECTIVE can be used to turn perspective mapping off, R1_Z_BIAS
  //can be used to help things sort somewhat in front of other things in the
  //scene (although it should truely just be removed). R1_LOCK_CHEAT draws
  //to an unlocked surface (ie, it calls DirectDrawSurface->Lock() to get
  //the pointer, then calls Unlock() but still uses the pointer).
  
  i4_bool is_feature_on(w32 feature_bits);

  //i4_image handling routines
  i4_image_class *create_compatible_image(w16 w, w16 h);
  //creates an image of the same bit depth and palette of screen (for use with put_image)
     

  //the r1 interface
  void set_alpha_mode(r1_alpha_type type);
  void set_write_mode(r1_write_mask_type mask);
  void set_shading_mode(r1_shading_type type);
  void set_constant_color(w32 color);
  void set_z_range(i4_float near_z, i4_float far_z);

  void render_poly(int t_verts, r1_vert *verts);
  void render_sprite(r1_vert *verts);
  void render_pixel(r1_vert *pixel);
  void render_lines(int t_lines, r1_vert *verts);
  void clear_area(int x1, int y1, int x2, int y2, w32 color, float w);
  
  void use_texture(r1_texture_handle material_ref, 
                   sw32 desired_width,
                   w32 frame);
    
  void disable_texture();
  //no texturemapping, drawing is done with the constant color (see set_constant_color)

  r1_color_tint_handle register_color_tint(i4_float r, i4_float g, i4_float b);

  //calling this with a handle of 0 disables tinting
  void set_color_tint(r1_color_tint_handle color_tint_handle);

  //these are currently public so that non member functions (of which there are
  //several) can access them
  i4_bool use_spans;
  i4_bool lock_cheat;
  i4_bool allow_backfaces;
  sw8     cur_color_tint;

  void copy_part(i4_image_class *im,                                          
                 int x, int y,             // position on screen
                 int x1, int y1,           // area of image to copy 
                 int x2, int y2);

private:

  sw32 total_texture_mem() { return 1024*1024*2; }
  sw32 texture_mem_start() { return 0; }

  void initialize_function_pointers(i4_bool use_amd3d_functions);
  void init_color_tints();

  void update_tri_function();

  //private primitive functions
  void draw_line(sw32 x0, sw32 y0, sw32 x1, sw32 y1, w16 color);
  void draw_line(sw32 x0, sw32 y0, sw32 x1, sw32 y1, w16 start_color, w16 end_color);


  void prepare_verts(s_vert *s_v, r1_vert *r1_v, sw32 num_verts);  

  i4_pixel_format fmt;

  i4_bool holy_mode;
  i4_bool alphatexture_mode;
  i4_bool do_perspective;
  i4_bool do_flat;
  i4_bool texture_mode;
  float   z_bias;

  sw32    width_compare;
  sw32    height_compare;

  w32     cur_color;  

  float   cur_s_width,cur_t_height;

  float   const_color_red,
          const_color_green,
          const_color_blue,
          const_color_alpha;
  
  float red_clight_mul;
  float green_clight_mul;
  float blue_clight_mul;
};

extern r1_software_class r1_software_class_instance;
#endif

