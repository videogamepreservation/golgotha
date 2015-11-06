/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "object_definer.hh"
#include "lisp/li_class.hh"
#include "objs/model_draw.hh"

class g1_cobra_tank_class : public g1_object_class
{
public:
  virtual i4_float occupancy_radius() const { return draw_params.extent(); }
  
  g1_cobra_tank_class(g1_object_type id, g1_loader_class *fp)
    : g1_object_class(id,fp)
  {
    radar_type=G1_RADAR_VEHICLE;
    flags=SHADOWED |
      SELECTABLE |
      TARGETABLE |
      GROUND | 
      HIT_GROUND |
      BLOCKING |
      DANGEROUS;
    
    draw_params.setup("cobra");
  }
  

  void think()
  {
    grab_old();
    
    unoccupy_location();

    x += 0.01;
    
    if (occupy_location())
      request_think();
  }
};

static g1_object_definer<g1_cobra_tank_class>
damager_type("cobra_tank", g1_object_definition_class::EDITOR_SELECTABLE |
   g1_object_definition_class::MOVABLE);

