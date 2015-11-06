/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_TILE_PICKER_HH
#define G1_TILE_PICKER_HH


#include "window/window.hh"
#include "window/colorwin.hh"
#include "memory/array.hh"
#include "editor/dialogs/scroll_picker.hh"
#include "editor/dialogs/pick_win.hh"


class g1_tile_picker_class : public g1_scroll_picker_class
{
  i4_image_class *active_back;
  i4_image_class *passive_back;


  // this should return 0 if scroll_object_num is too big
  virtual i4_window_class *create_window(w16 w, w16 h, int scroll_object_num);
  virtual void change_window_object_num(i4_window_class *win, int new_scroll_object_num);
  virtual void rotate();
  virtual void mirror();
  virtual int total_objects();
public:

  g1_tile_picker_class(i4_graphical_style_class *style, 
                       g1_scroll_picker_info *info,
                       i4_image_class *active_back,
                       i4_image_class *passive_back);



};


#endif

