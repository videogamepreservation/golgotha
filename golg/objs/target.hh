/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_TARGET_HH
#define G1_TARGET_HH

#include "g1_object.hh"
#include "objs/model_draw.hh"

extern g1_object_type g1_target_type;

class g1_target_class : public g1_object_class
{
public:
  g1_model_draw_parameters draw_params;

  g1_target_class(g1_object_type id, g1_loader_class *fp);

  virtual i4_float occupancy_radius() const { return 0.0; }
  virtual i4_bool occupy_location();
  virtual void draw(g1_draw_context_class *context);
  virtual void think() {}

  virtual void setup(i4_float x, i4_float y);
};

#endif
