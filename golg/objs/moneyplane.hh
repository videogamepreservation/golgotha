/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_MONEYPLANE_HH
#define G1_MONEYPLANE_HH

#include "objs/map_piece.hh"
#include "math/num_type.hh"

class g1_moneycrate_class;
class g1_moneyplane_class : public g1_map_piece_class
{
public:
  g1_moneyplane_class(g1_object_type id, g1_loader_class *fp);

  g1_moneycrate_class* crate();
  void set_crate(g1_moneycrate_class *cr);

  i4_bool move(const i4_3d_vector &d);
  virtual void damage(g1_object_class *obj, int hp, i4_3d_vector _damage_dir);

  virtual void think();
};

#endif
