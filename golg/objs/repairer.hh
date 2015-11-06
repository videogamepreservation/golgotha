/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_REPAIRER_HH
#define G1_REPAIRER_HH

#include "sound/sound.hh"
#include "objs/map_piece.hh"

class g1_repairer_class : public g1_map_piece_class
{
protected:
  g1_mini_object        *turret,*boom,*tip;
  i4_float repair_angle, repair_length;
  i4_float length;
public:
  g1_repairer_class(g1_object_type id, g1_loader_class *fp);
  virtual void save(g1_saver_class *fp);
      
  virtual i4_bool can_attack(g1_object_class *who) const;
  virtual void think();
  virtual void draw(g1_draw_context_class *context);
};

#endif
