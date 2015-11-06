/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_SCROLL_PICKER_HH
#define G1_SCROLL_PICKER_HH


#include "window/window.hh"
#include "window/colorwin.hh"
#include "memory/array.hh"
#include "r1_win.hh"

class i4_graphical_style_class;
class i4_scroll_bar;

struct g1_scroll_picker_info
{
  int scroll_offset;
  int object_size;
  int max_object_size, min_object_size;
  int max_objects_down;
  int win_x, win_y;


  g1_scroll_picker_info()
  {
    scroll_offset=0;
    object_size=64;
    max_object_size=128;
    min_object_size=16;
    max_objects_down=3;
    win_x=win_y=-1;
  }
};

class g1_scroll_picker_class : public i4_color_window_class
{
protected:
  i4_array<i4_window_class *> windows;
  i4_array<r1_render_window_class *> render_windows;

  int start_y, show_area_w;
  i4_scroll_bar *scroll_bar;
  g1_scroll_picker_info *info;



  // this should return 0 if scroll_object_num is too big
  virtual i4_window_class *create_window(w16 w, w16 h, int scroll_object_num) = 0;
  virtual void change_window_object_num(i4_window_class *win, int new_scroll_object_num) = 0;
  virtual int total_objects() = 0;
  virtual void rotate() { ; }
  virtual void mirror() { ; }

public:
  enum { ROTATE, MIRROR, GROW, SHRINK, SCROLL };

  void refresh();
  void create_windows();
  // option_flags can include (1<<ROTATE) | (1<<MIRROR) | (1<<GROW) | (1<<SHRINK) | (1<<SCROLL))
  g1_scroll_picker_class(i4_graphical_style_class *style, 
                         w32 option_flags,
                         g1_scroll_picker_info *info,
                         int total_objects);

  void receive_event(i4_event *ev);
  void parent_draw(i4_draw_context_class &context);
  void change_current_select();
};


#endif
