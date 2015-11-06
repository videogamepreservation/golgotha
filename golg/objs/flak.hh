/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef FLAK_HH
#define FLAK_HH

#include "g1_object.hh"
#include "math/vector.hh"

class g1_flak_class : public g1_object_class
{
protected:
  w16  time;
  i4_float size;
public:
  virtual i4_float occupancy_radius() const  { return 0.4; }

  g1_flak_class(g1_object_type id, g1_loader_class *fp);
  virtual void setup(const i4_3d_vector &pos, i4_float size);
  
  virtual void draw(g1_draw_context_class *context);
  virtual void think();
};

#endif
