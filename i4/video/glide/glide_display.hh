/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef GLIDE_DISPLAY_HH
#define GLIDE_DISPLAY_HH


#include "video/display.hh"
#include "video/glide/glide.h"

class i4_cursor_class;


class i4_glide_display_class : public i4_display_class
{
protected:
  i4_draw_context_class *context;

  class glide_mode_class : public i4_display_class::mode
  {
  public:
    sw32 glide_mode;
  };

  glide_mode_class tmp, cur_mode;
  i4_cursor_class *mcursor;


  sw32 last_mouse_x, last_mouse_y;

  i4_image_class *bbuf,    // backbuffer
    *mouse_save1,          // save of screen under mouse
    *mouse_save2, 
    *prev_mouse_save;      // pointer to mouse_save1 or 2, depending on which saved last
  
  void remove_cursor(sw32 x, sw32 y, i4_image_class *mouse_save);
  void save_and_draw_cursor(sw32 x, sw32 y, i4_image_class *&mouse_save);

  i4_image_class *fake_screen;
  i4_bool detected;
  i4_rect_list_class next_frame_copy;

  i4_image_class *get_writeable_screen();
  i4_image_class *get_readable_screen();
  i4_display_list_struct me;

public:

  // fill these in with the derived class (so we can share this with linux and win32)
  virtual i4_device_class *local_devices() = 0;
  virtual i4_bool create_window() = 0;
  virtual void destroy_window() = 0;
  virtual void get_mouse_pos(sw32 &mouse_x, sw32 &mouse_y) = 0;

  virtual i4_draw_context_class *get_context() { return context; }

  virtual w16 width() const { return 640; }
  virtual w16 height() const { return 480; }

  virtual mode *current_mode() { return &cur_mode; }
  virtual mode *get_first_mode(int driver_id);
  virtual mode *get_next_mode();

  virtual unsigned long window_handle() { return 0; } 


  virtual i4_image_class *lock_frame_buffer(i4_frame_buffer_type type,
                                            i4_frame_access_type access);
  virtual void unlock_frame_buffer(i4_frame_buffer_type type);
  virtual i4_refresh_type update_model() { return I4_PAGE_FLIP_REFRESH; }

  virtual i4_bool initialize_mode();
  virtual void flush();
  i4_glide_display_class();
  virtual i4_image_class *get_screen();

  // should be called before a program quits
  virtual i4_bool close();

  virtual i4_bool lock_mouse_in_place(i4_bool yes_no) = 0;
  virtual i4_bool set_mouse_shape(i4_cursor_class *cursor);
  void begin_render_mode();
  void end_render_mode();

  void init();
  void uninit();

  ~i4_glide_display_class();
};


extern i4_glide_display_class *i4_glide_display;

#endif
