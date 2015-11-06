/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef IMAGE_WIN_HH
#define IMAGE_WIN_HH

#include "window/window.hh"
#include "window/win_evt.hh"

class i4_image_class;

class i4_image_window_class : public i4_parent_window_class
{
  i4_image_class *im, *darken_im;
  i4_bool del, darken, active;

  void reparent(i4_image_class *draw_area, i4_parent_window_class *parent);
public:
  // if delete on destructor is true then change_image will delete the old image
  void change_image(i4_image_class *im);  

  i4_image_window_class(i4_image_class *im,
                        i4_bool delete_on_destructor=i4_F,
                        i4_bool dark_when_not_active=i4_T);

  virtual void parent_draw(i4_draw_context_class &context);
  void receive_event(i4_event *ev);
  ~i4_image_window_class();
  char *name() { return "image_window"; }
} ;

#endif
