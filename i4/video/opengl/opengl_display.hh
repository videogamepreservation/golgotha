/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "video/x11/x11_input.hh"
#include "video/display.hh"

class opengl_display_class : public i4_display_class
{
  int GLX_error_base;
  int GLX_event_base;
  i4_bool know_if_available, known_availability;


  i4_draw_context_class *context;
  mode amode, cur_mode;
  i4_cursor_class *mcursor;

  i4_image_class *mouse_save1,          // save of screen under mouse
    *mouse_save2, 
    *prev_mouse_save;      // pointer to mouse_save1 or 2, depending on which saved last
  
  void remove_cursor(sw32 x, sw32 y, i4_image_class *mouse_save);
  void save_and_draw_cursor(sw32 x, sw32 y, i4_image_class *&mouse_save);

  i4_image_class *fake_screen;
  i4_rect_list_class next_frame_copy;
  sw32 last_mouse_x, last_mouse_y;
  
public:
  x11_input_class input;

  void get_pl_pos(int pixel_x, int pixel_y, float &glx, float &gly)
  {
    glx = pixel_x *2.0 / (float)(width()) -1;
    gly = ((height()-pixel_y) *2.0/  (float)height() -1 );
  }

  virtual char *name() { return "OpenGl X display"; }
  virtual i4_draw_context_class *get_context() { return context; }
  virtual i4_image_class *get_screen() { return fake_screen; }
  i4_bool set_mouse_shape(i4_cursor_class *cursor);
  virtual void flush();
  virtual w16 width() const { return fake_screen->width(); }
  virtual w16 height() const { return fake_screen->height(); }

  virtual mode *current_mode() { return &cur_mode; }  
  virtual mode *get_first_mode(); 
  virtual mode *get_next_mode(mode *last_mode) { return 0; }
  virtual i4_bool initialize_mode(mode *which_one);

  virtual i4_bool close();
  virtual i4_bool realize_palette(i4_pal_handle_class pal_id) { return i4_F; }

  virtual i4_bool lock_mouse_in_place(i4_bool yes_no) { return input.lock_mouse_in_place(yes_no); }




  opengl_display_class();

  i4_bool available();
  XVisualInfo *get_visual(Display *display);


};

extern opengl_display_class opengl_display_instance;
