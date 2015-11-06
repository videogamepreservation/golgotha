/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef __SVGA_VD_HPP_
#define __SVGA_VD_HPP_

#include "image/depth.hh"
#include "device/event.hh" 
#include "video/display.hh"
#include "image/image8.hh"
#include "image/image32.hh"
#include "image/img8clip.hh"

class svgalib_display_class : public i4_display_class
{
  void copy_part_to_vram(i4_image_class *im, int x, int y, int x1, int y1, int x2, int y2);

  w8 *v_addr;
  void fill_amode(int mode);
  class svga_mode : public mode
  {
    public :
    i4_display_class *assoc;     // pointer to use, so we can confirm we created this mode
    w32 mode_num;          // for the display driver's use only
  } amode;
  enum { NOT_INITIALIZED,
	 INITIALIZED_AND_NOT_AVAILABLE,
	 INITIALIZED_AND_AVAILABLE
	 } init;

  i4_pal_handle_class mouse_pal;
  int cur_svga_mode,mouse_x,mouse_y;

  
  i4_image_class *mouse_pict;    // generic image which will take the type of the screen when we copy the save portion
  i4_SCREEN_TYPE *mouse_save;
              

  int mouse_event_flags;
  i4_draw_context_class *context;

  public :

  class svgalib_device_manager : public i4_device_class     // maintains the mouse & keyboard
  {
    public :
    char *name() { return "svgalib device manager"; } 
  } devs;


  class svgalib_mouse_notify : public i4_event_handler_class   // receives events from the mouse, so display can update mouse position
  {
    public :
    svgalib_display_class *display;

    virtual void receive_event(i4_event *ev)
    {
      if (ev->type==i4_event::MOUSE_MOVE)
      {
	i4_mouse_move_event_class *mev=(i4_mouse_move_event_class *)ev;
	display->mouse_x=mev->x;
	display->mouse_y=mev->y;
      }
    }
  } mouse_notify;

  i4_device_class *local_devices() { return &devs; }
  
  i4_image8_clippable *screen;


  virtual i4_image_class *next_frame_screen(i4_rect_list_class *area_of_change);
  virtual i4_image_class *both_screens();

  virtual w16 width() { return screen->width(); }
  virtual w16 height() { return screen->height(); }


  virtual void flush();
  svgalib_display_class();

  virtual w8  bits_per_pixel() { return 8; }


  virtual mode *get_first_mode();
  virtual mode *get_next_mode(mode *last_mode);
  i4_rect_list_class *get_clip_list() { return &screen->clip_list; }

  virtual i4_draw_context_class *get_context() { return context; }

  virtual i4_bool realize_palette(i4_pal_handle_class pal_id);

  // initialize_mode need not call close() to switch to another mode
  i4_bool initialize_mode(mode *which_one);

  // should be called before a program quits
  i4_bool close();


  char *name() { return "SVGALIB console display"; }
  i4_bool available();
  ~svgalib_display_class();
} ;

extern svgalib_display_class svgalib_display_instance;


#endif
