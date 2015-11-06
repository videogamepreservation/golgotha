/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_GOAL_HH
#define G1_GOAL_HH

#include "g1_limits.hh"
#include "g1_object.hh"

class g1_goal_class : public g1_object_class
{
public:
  int power[G1_MAX_PLAYERS];
  g1_model_draw_parameters draw_params;

  g1_goal_class(g1_object_type id, g1_loader_class *fp);
  virtual void save(g1_saver_class *fp);

  virtual void draw(g1_draw_context_class *context);
  virtual void think();
  virtual i4_float occupancy_radius() const { return 0; }
};

#endif
