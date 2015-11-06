/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_MONEYCRATE_HH
#define G1_MONEYCRATE_HH

#include "g1_object.hh"

class g1_moneycrate_class : public g1_object_class
{
public:
  g1_moneycrate_class(g1_object_type id, g1_loader_class *fp);

  void setup(i4_3d_vector pos, w32 val);
  void follow(i4_3d_vector pos, i4_3d_vector rot);
  void release();
  w32 value() const;
  static i4_3d_vector crate_attach();

  virtual void think();
};

#endif
