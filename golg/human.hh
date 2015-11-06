/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef HUMAN_HH
#define HUMAN_HH

#include "team_api.hh"
#include "memory/que.hh"

class g1_human_class : public g1_team_api_class
{
public:
  enum { MAX_COMMANDS = 30 };

  g1_human_class(g1_loader_class *f);
  ~g1_human_class();

  i4_float mouse_look_increment_x, mouse_look_increment_y;

  g1_typed_reference_class<g1_object_class> selected_object;
  g1_typed_reference_class<g1_object_class> preassigned[10];

  void send_selected_units(i4_float x, i4_float y);
  void clicked_on_object(g1_object_class *o);

  w8 determine_cursor(g1_object_class *object_mouse_is_on);
  void player_clicked(g1_object_class *o, float gx=0, float gy=0);
  int build_unit(g1_object_type type);

  virtual void think();
  virtual char *name() { return "Human"; }

  virtual void load(g1_loader_class *fp);
};

extern g1_human_class *g1_human;

#endif
