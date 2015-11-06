/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_CONTROLLER_WINDOW_MANAGER_HH
#define G1_CONTROLLER_WINDOW_MANAGER_HH

#include "device/device.hh"
#include "palette/pal.hh"
#include "time/time.hh"
#include "math/num_type.hh"
#include "math/pi.hh"
#include "camera.hh"
#include "window/window.hh"

class i4_window_manager_class;
class g1_map_class;
class i4_parent_window_class;
class i4_graphical_style_class;
class r1_render_api_class;
class g1_object_controller_class;
class i4_image_class;
class g1_map_class;
class g1_border_frame_class;
class g1_map_view_class;
class i4_display_class;
class g1_object_class;
class g1_loader_class;
class g1_saver_class;

// this class manages the view(s) of the game
// derived from it is the g1_editor_class that creates 
// views with editing behavior (derived from g1_object_controller_class)

class g1_cwin_man_class : public i4_event_handler_class
{
private:
  g1_view_state_class view_state;
  
protected:                           // for use by derived classes
  i4_graphical_style_class   *style;
  i4_parent_window_class     *parent;
  i4_image_class             *root_image;


  const i4_pal *pal;

public:                             // called by main program
  char *name() { return "g1_cwin_man_class"; }

  virtual void create_views();
  virtual void destroy_views();
  
  virtual void map_changed();     // called when a new map is loaded/created

  virtual void init(i4_parent_window_class *_parent,
                    i4_graphical_style_class *_style,
                    i4_image_class *_root_image,
                    i4_display_class *display,
                    i4_window_manager_class *wm);
  
  virtual void uninit() { ; }

  virtual void set_edit_mode(i4_bool yes_no) { ; }

  virtual void save_views(g1_saver_class *fp);
  virtual void load_views(g1_loader_class *fp);

  virtual void receive_event(i4_event *ev);

  g1_cwin_man_class();
};


// initial 0, but should be changed by global constructor if you want
// this to have different behavior,  if still 0 by the time the game uses it
// it will create a g1_cwin_man_class

extern g1_cwin_man_class *g1_cwin_man;

void g1_change_key_context(w8 view_mode);



#endif
