/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef BORDER_FRAME_HH
#define BORDER_FRAME_HH


#include "window/window.hh"
#include "string/string.hh"
#include "memory/array.hh"
#include "window/colorwin.hh"

class i4_button_class;
class i4_image_class;
class i4_graphical_style_class;

// this window is responsible for drawing the border area around the game window's
// it also loads the board frame from disk when need.  My hope is that during the normal
// game it can free up the memory associated with the border.
class g1_border_frame_class : public i4_color_window_class
{
public:


private:

  enum
  { 
    OPTIONS,
    BUILD_FIRST, BUILD_LAST=BUILD_FIRST+0x1000,
    ACTIVE_FIRST, ACTIVE_LAST=ACTIVE_FIRST+0x1000
  };

  // left justify draw number with commas if needed

  
  enum { REFRESH_FRAME=1,
         REFRESH_ALL=REFRESH_FRAME*2-1
  };

  struct last_struct
  {
    int money, ammo[4], cost, frame_upgrade_level;
    int lives;
    last_struct() { memset(this, 0, sizeof(*this)); }
  } last;

  
  int refresh;
  void reparent(i4_image_class *draw_area, i4_parent_window_class *parent);

public:  
  int border_x();
  int border_y();

  void relocate(i4_parent_window_class *w, char *loc, int x, int y);

  void resize_controller(int shrink_add);

  int shrink;              // how many pixels the 3d view is shrinked in for speed-up
  i4_image_class *frame;

  i4_bool mouse_grabbed;

  i4_window_class *strategy_window;
  i4_window_class *controller_window;
  i4_window_class *radar_window;

  void set_strategy_on_top(i4_bool v);
  i4_bool strategy_on_top;

  void update();       // check for changes in the game
  
  g1_border_frame_class();
  
  virtual void parent_draw(i4_draw_context_class &context);
  virtual void draw(i4_draw_context_class &context);

  void receive_event(i4_event *ev);

  ~g1_border_frame_class();
  char *name() { return "border_frame"; }

};


class g1_strategy_screen_class : public i4_color_window_class
{
  enum { OPTIONS, BUILD=1, ACTIVE=100 };
  i4_array<i4_button_class *> build_buttons;

public:
  int border_x();
  int border_y();
  int shrink;              // how many pixels the 3d view is shrinked in for speed-up
  i4_image_class *frame;

  void create_build_buttons();

  virtual void parent_draw(i4_draw_context_class &context);
  g1_strategy_screen_class();
  void receive_event(i4_event *ev);
  char *name() { return "strategy_screen"; }
};


extern i4_event_handler_reference_class<g1_border_frame_class> g1_border;
extern i4_event_handler_reference_class<g1_strategy_screen_class> g1_strategy_screen;

w32 g1_get_upgrade_color(int upgrade_level);  // used for font colors & border edges


class g1_help_screen_class : public i4_parent_window_class
{
  i4_image_class *help;
  w32 mess_id_to_send;
public:
  char *name() { return "help screen"; }

  g1_help_screen_class(w16 w, w16 h, 
                       i4_graphical_style_class *style, 
                       const i4_const_str &image_name,
                       w32 mess_id_to_send);

  virtual void parent_draw(i4_draw_context_class &context);
  void receive_event(i4_event *ev);
  ~g1_help_screen_class();
};

#endif


