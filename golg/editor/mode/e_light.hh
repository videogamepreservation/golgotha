/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef LIGHT_MODE_HH
#define LIGHT_MODE_HH


#include "editor/e_state.hh"
#include "editor/mode/e_mode.hh"

class g1_light_mode : public g1_mode_handler
{
public:
  virtual state current_state();
  g1_light_mode(g1_controller_edit_class *c) : g1_mode_handler(c) {}
};


class g1_light_params : public g1_mode_creator
{
public:
  char *name() { return "LIGHT"; }
  enum 
  {
    ROTATE,
    ZOOM,
    MOVE,
    SELECT,
    GDARKEN,
    GBRIGHTEN,
    DDARKEN,
    DBRIGHTEN,
    AMBIENT,
    GDARKEN_REP,
    GBRIGHTEN_REP,
    DDARKEN_REP,
    DBRIGHTEN_REP,
  } ;

  void create_buttons(i4_parent_window_class *container);

  g1_mode_handler *create_mode_handler(g1_controller_edit_class *c)
  { return new g1_light_mode(c); }


  void receive_event(i4_event *ev);
} ;

extern g1_light_params g1_e_light;



#endif
