/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_EDITOR_TIME_HH
#define G1_EDITOR_TIME_HH



#include "device/device.hh"

class i4_window_class;
class i4_parent_window_class;
class i4_graphical_style_class;
class i4_image_class;
class i4_button_class;
class i4_button_box_class;

class g1_time_scroller;
class g1_frame_show;

class g1_edit_state_class;
class i4_spline_class;
class g1_movie_flow_class;
class i4_text_input_class;

class g1_time_line : public i4_event_handler_class
{
  w32 h;
  enum { LAST_TIME, NEXT_TIME, PLAY, REWIND, FFORWARD, LAST_SCENE, NEXT_SCENE,
         SCENE_CHANGED, FRAME_CHANGED, MOVIE_STOPPED};
  
  i4_event_handler_reference_class<g1_time_scroller> scroller;
  i4_event_handler_reference_class<g1_frame_show> frame_show;
  i4_event_handler_reference_class<i4_parent_window_class> time_win;
  i4_event_handler_reference_class<i4_window_class> sec_win;

  i4_event_handler_reference_class<i4_button_box_class> bbox;

  // save this so we can pop it when play is done
  i4_event_handler_reference_class<i4_button_class> play;

  g1_movie_flow_class *current_movie();
  i4_event_handler_reference_class<i4_text_input_class> scene_number_input;
  i4_event_handler_reference_class<i4_button_class> last_scene, next_scene;
  void set_current_scene(sw32 scene);
  void update_scene();

  void unpress_play();

public:
  ~g1_time_line();

  g1_edit_state_class *state;


  char *name() { return "time_line"; }



  i4_button_class *create_img_win(char *icon_res_name,
                                  w32 mess_id,
                                  i4_graphical_style_class *style);

  g1_time_line(i4_parent_window_class *parent, 
               i4_graphical_style_class *style,
               g1_edit_state_class *state);
               

  sw32 height() { return h; }
  void update();

  void set_current(w32 frame, i4_bool stop_play);

  w32 current_frame();
  w32 total_frames();

  void receive_event(i4_event *ev);

  void create_time_win(i4_graphical_style_class *style);

  i4_spline_class *selected_spline();
  
};


#endif
