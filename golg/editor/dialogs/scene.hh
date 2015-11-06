/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_SCENE_EDITOR_HH
#define G1_SCENE_EDITOR_HH


#include "window/window.hh"
#include "device/device.hh"

class g1_movie_flow_class;
class i4_graphical_style_class;


class g1_scene_editor_class : public i4_event_handler_class
{
  g1_movie_flow_class *movie;
  i4_graphical_style_class *style;
  i4_event_handler_reference_class<i4_parent_window_class> parent;
  sw32 wx, wy;

  void last_scene();

public:
  enum { ADD_SCENE,             // sent to self when add scene is clicked
         CHANGE_SCENE_NUMBER,   // sent to self when a scene number is changed
         WINDOW_CLOSED,
         DELETE_SCENE
  };
         
  g1_scene_editor_class() { parent=0; wx=wy=0; }

  char *name() { return "scene_editor"; }

  void receive_event(i4_event *ev);

  void show(i4_parent_window_class *parent_window,
            g1_movie_flow_class *movie,
            i4_graphical_style_class *style);

  void hide();

  i4_bool show_is_valid_option() { return (i4_bool)(parent.get()==0); }

  ~g1_scene_editor_class() { hide(); }
};

#endif
