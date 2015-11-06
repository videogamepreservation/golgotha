/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

//{{{ Application Class
//
//$Id: app.hh,v 1.22 1998/06/02 23:50:20 jc Exp $
#ifndef I4_APP_HH
#define I4_APP_HH


class i4_window_manager_class;
class i4_parent_window_class;
class i4_graphical_style_class;

#include "video/display.hh"
#include "device/device.hh"
#include "palette/pal.hh"
#include "file/file.hh"

class i4_application_class : public i4_event_handler_class
{
protected:
  i4_bool finished;
  i4_display_class *display;
  i4_window_manager_class *wm;  

  i4_application_class() {}

  i4_display_class::mode *find_mode(w16 &width, w16 &height, int driver_id);

  virtual void handle_no_displays();  
  virtual void receive_event(i4_event *ev);

  virtual void get_input();
  virtual void calc_model();
  virtual void refresh();

  virtual void init();

  virtual void uninit();

  void memory_init();

  void resource_init(char *resource_file, 
                     void *resource_buffer);

  void display_init();
  void display_uninit();

  // normal the app use the registry to decide which display
  // to use. if no registry item available it uses the first display
  virtual i4_bool get_display_name(char *name, int max_len);

public:
  virtual void quit() { finished=i4_T; }

  // when you are not doing time critical return i4_T so i4 will give time to other processes
  virtual i4_bool idle() { return i4_T; }  


  virtual ~i4_application_class();
  virtual void run();
  i4_parent_window_class *get_root_window();
  i4_graphical_style_class *get_style();
  i4_window_manager_class *get_window_manager() { return wm; }
  i4_display_class *get_display() { return display; }
};

extern i4_application_class *i4_current_app;

#endif
