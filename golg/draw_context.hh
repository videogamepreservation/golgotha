/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_DRAW_CONTEXT_HH
#define G1_DRAW_CONTEXT_HH

#include "arch.hh"
#include "math/vector.hh"

class r1_render_api_class;
class i4_transform_class;
class i4_image_class;
class i4_draw_context_class;
class i4_graphical_style_class;
class i4_window_class;
class i4_window_manager_class;
class g1_light_set;
class r1_render_api_class;


class g1_draw_context_class
{
  enum { TRANS_STACK_SIZE=8 };
  i4_transform_class *stack[TRANS_STACK_SIZE];
  int top;

public:  
  void push(i4_transform_class &t) { stack[top++]=transform; transform=&t; }
  void pop() { top--; transform=stack[top]; }

  i4_bool draw_editor_stuff;

  i4_transform_class *transform;


  i4_image_class *screen;
  i4_draw_context_class *context;
  i4_graphical_style_class *style;
  g1_light_set *light;
  w32 default_render_bits;       // see polylist.hh, controls wireframe, texture, light rendering

  // this should be how much to scale the camera to fit window (maximum dimension of window)
  // for a pixel aspect ration of 1-1 window_scale_x==window_scale_y
  i4_float camera_scale_x;  
  i4_float camera_scale_y;

  i4_float texture_scale;          // if you scale a model beyond normal size, change this

  g1_draw_context_class();
  
  void window_setup(w32 win_x, w32 win_y, w32 win_w, w32 win_h,
                    float &center_x, float &center_y,
                    float &scale_x, float &scale_y,
                    float &ooscale_x, float &ooscale_y);
} ;

#endif
