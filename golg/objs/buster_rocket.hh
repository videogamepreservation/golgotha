/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_BUSTER_ROCKET_HH
#define G1_BUSTER_ROCKET_HH

#include "objs/guided_missile.hh"

class g1_buster_rocket_class : public g1_guided_missile_class
{
public:
  g1_buster_rocket_class(g1_object_type id, g1_loader_class *fp);

  virtual void add_smoke();
  virtual void update_smoke();
  virtual void delete_smoke();
};

#endif
