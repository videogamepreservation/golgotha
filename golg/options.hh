/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_OPTIONS_HH
#define G1_OPTIONS_HH


#include "window/colorwin.hh"
#include "device/device.hh"
#include "gui/image_win.hh"

class i4_graphical_style_class;
class i4_event_handler_class;
class i4_button_class;
class i4_const_str;

class g1_option_window : public i4_parent_window_class
{
  i4_graphical_style_class *style;
  i4_image_class *options_background;
  i4_event_handler_reference_class<i4_image_window_class> shadow_image_win, sound_image_win;

  i4_button_class *create_button(const i4_const_str &help, 
                                 int im,
                                 i4_event_handler_reference_class<i4_image_window_class> *win_ref,
                                 int mess_id);
  void add_buttons();

  enum { STOPPED,
         SLIDE_RIGHT,
         SLIDE_LEFT
  } mode;

  int get_correction();
  int slide_speed, slide_left, slide_correction;

public:
  enum 
  {
    SLIDE_AWAY,
    SLIDE,
    VIS_LOW,
    VIS_MEDIUM,
    VIS_HIGH,

    NORMAL_PIXEL,
    INTERLACE,
    DOUBLE_PIXEL,
    SOUND, 

    SHADOWS,
    GAME_SPEED
  };

  ~g1_option_window();
  void parent_draw(i4_draw_context_class &context);
  void forget_redraw();
  g1_option_window(i4_graphical_style_class *style);

  void receive_event(i4_event *ev);
  char *name() { return "g1_option_window"; }
};


extern i4_event_handler_reference_class<g1_option_window> g1_options_window;

#endif

