/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef EDITOR_E_STATE_HH
#define EDITOR_E_STATE_HH


#include "reference.hh"
#include "device/device.hh"
#include "player_type.hh"
#include "rotation.hh"
#include "device/event.hh"
#include "g1_limits.hh"
#include "editor/dialogs/scroll_picker.hh"


class i4_image_class;
class i4_spline_class;
class i4_window_class;
class i4_graphical_style_class;
class i4_parent_window_class;
class i4_graphical_style_class;
class i4_button_box_class;
class i4_spline_class;
class i4_button_class;
class g1_reference_class;
class g1_object_class;
class g1_controller_edit_class;
class g1_mode_creator;

enum 
{ 
  G1_SET_MAJOR_MODE,
  G1_SET_MINOR_MODE,
  G1_EDIT_CHANGE_LIGHT_DIRECTION
};


class g1_set_major_mode_event : public i4_user_message_event_class
{  
public:
  char name[30];

  g1_set_major_mode_event(char *_name) 
    : i4_user_message_event_class(G1_SET_MAJOR_MODE)  { strcpy(name, _name); }

  virtual i4_event  *copy() { return new g1_set_major_mode_event(name); }  
};


class g1_set_minor_mode_event : public g1_set_major_mode_event
{  
public:
  w8 minor_mode;
  g1_set_minor_mode_event(char *major_mode_name, w8 minor_mode)
    : g1_set_major_mode_event(major_mode_name), minor_mode(minor_mode)
  {
    sub_type=G1_SET_MINOR_MODE;
  }

  virtual i4_event  *copy() { return new g1_set_minor_mode_event(name, minor_mode); }  
};



class g1_edit_state_class : public i4_event_handler_class
{

  i4_graphical_style_class *get_style();
  i4_window_class *create_buttons(w32 height);
  i4_window_class *tools;

  w32 win_h;
  i4_parent_window_class *parent;


  i4_image_class **icons;
  int total_icons;
  void load_icons();



public: 
  enum { NO_SNAP, SNAP_CENTER, SNAP_ORIGIN } snap;
  void snap_point(i4_3d_point_class &p, int do_z=0);

  
  g1_controller_edit_class *current_focus();

  struct context_help_struct
  {
    i4_event_handler_reference_class<i4_window_class> window;
    int mx,my;  // mouse position
    void show(const i4_const_str &help, int mx, int my);
    void hide();

  } context_help;

  i4_image_class *get_icon(int i) const 
  {
    I4_ASSERT(i>=0 || i<total_icons, "bad icon number");  
    return icons[i]; 
  }

  i4_button_class *create_button(char *res_name, w32 evalue, i4_bool popup, 
                                 i4_event_handler_class *send_to=0,
                                 i4_event *send_event=0);

  void add_but(i4_button_box_class *box,                     
               char *res_name, 
               w32 evalue, 
               i4_bool down,
               i4_event *send_event=0);




  char major_mode[20];

  g1_mode_creator *get_major_mode();
  i4_bool set_major_mode(char *mode_name);
  i4_bool set_minor_mode(char *major_mode_name, w32 minor_mode);

  void set_current_team(int team_num);
  g1_player_type current_team;
  
  g1_edit_state_class();

  void show_tools(i4_parent_window_class *parent,  w32 win_h);
                 
  void hide_tools();
  void receive_event(i4_event *ev);

  void uninit();   // free up memory at end of program

  void hide_focus();
  void show_focus();

  w32 tools_width();


  char *name() { return "edit_state"; }
};

extern g1_edit_state_class g1_edit_state;

#endif

