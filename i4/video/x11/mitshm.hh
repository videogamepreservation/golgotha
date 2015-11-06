/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef __MITSHM_HPP_
#define __MITSHM_HPP_

// this module can be compiled seperately from x11_display_class
// if a system does not support IPX do not compile this module
// otherwise it will link itself with the x11_display_class on init()

#include "arch.hh"
#include "image/image.hh"
#include "init/init.hh"
#include <X11/Xlib.h> 
#include <X11/Xutil.h>
#include <X11/keysym.h>

class x11_shm_extension_class;

class x11_shm_image_class
{
  public :
  w8 *data;
  virtual i4_bool copy_part_to_vram(x11_shm_extension_class *use, 
                                    i4_coord x, i4_coord y, 
                                    i4_coord x1, i4_coord y1, 
                                    i4_coord x2, i4_coord y2) = 0;
} ; 

class x11_shm_extension_class : public i4_init_class
{
  public :
  i4_bool need_sync_event;
  int shm_base,shm_error_base,shm_finish_event;



  virtual void note_event(XEvent &ev) = 0;
  x11_shm_extension_class();
  virtual i4_bool available(Display *display, char *display_name) = 0;
  virtual x11_shm_image_class *create_shm_image(Display *display,
					  Window window,
					  GC gc,
					  Visual *X_visual,
					  int visual_depth,
					  w16 &width, w16 &height) = 0;

  virtual void destroy_shm_image(Display *display, x11_shm_image_class *im) = 0;
  virtual void shutdown(Display *display) = 0;
} ;

#endif
