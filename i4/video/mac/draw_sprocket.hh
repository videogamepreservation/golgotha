/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef DRAW_SPROCKET_HH
#define DRAW_SPROCKET_HH

#include "image/depth.hh"
#include "image/image8.hh"
#include "image/image15.hh"
#include "image/image16.hh"
#include "image/image32.hh"
#include "video/display.hh"
#include <QuickDraw.h>
#include <Displays.h>
#include <DrawSprocket.h>

class draw_sprocket_class : public i4_display_class
{
protected:
  GDHandle gd;
  DisplayIDType display_id;
  w32 wx,wy;

  // PixMap handle to current back buffer memory
  PixMapHandle PMH;

  i4_draw_context_class *context;
  DSpContextReference DSp_ref;

  I4_SCREEN_TYPE *screen;

  class device_manager_class : public i4_device_class
  //{{{
  {
  protected:
    sw32 mx,my, omx,omy;
    w32 shift_state;            // keyboard shift state
    char mac_modifier[8];
  public:
    virtual char *name() { return "draw_sprocket device manager"; } 
    device_manager_class();

    virtual i4_bool process_events();
    w32 request_device(i4_event_handler_class *for_who, w32 event_types);
    void release_device(i4_event_handler_class *for_who, w32 event_types);
  };
  //}}}
  device_manager_class devs;
  
  class draw_sprocket_mode_class : public mode
  //{{{
  {
  public:
    DSpContextReference ref;

    draw_sprocket_mode_class(DSpContextReference _ref) : ref(_ref) {}
  };
  //}}}

  draw_sprocket_mode_class *new_mode(DSpContextReference context);
  void setup_back_buffer();
public:
  draw_sprocket_class();
  ~draw_sprocket_class();

  virtual i4_device_class *local_devices() { return &devs; }

  virtual i4_image_class *get_screen() { return screen; }
  virtual i4_draw_context_class *get_context() { return context; }

  virtual void flush();

  virtual char *name() const { return "Mac DrawSprocket"; }

  virtual w16 width() const { return wx; }
  virtual w16 height() const { return wy; }

  virtual mode *get_first_mode();
  virtual mode *get_next_mode(mode *last_mode);

  virtual i4_bool initialize_mode(mode *which_one);

  virtual i4_bool close();
  virtual i4_bool available();

  virtual i4_bool realize_palette(i4_pal_handle_class pal_id);

  virtual i4_bool set_mouse_shape(i4_cursor_class *cursor);
};

#endif

//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}
