/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "team_api.hh"


class ai_builder : public g1_team_api_class
{
public:
  ai_builder(g1_loader_class *f=0) 
  {
  }
  
  virtual void think()
  {
    build_unit(g1_get_object_type("peon_tank"));
  }    
};

g1_team_api_definer<ai_builder> ai_builder_def("ai_builder");

