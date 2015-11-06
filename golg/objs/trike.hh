/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_TRIKE_HH
#define G1_TRIKE_HH

#include "objs/map_piece.hh"

class g1_trike_class : public g1_map_piece_class
{
protected:
  enum {DATA_VERSION=1};

  g1_mini_object     *wheels;
public:
  g1_trike_class(g1_object_type id, g1_loader_class *fp);
  virtual void save(g1_saver_class *fp);
  virtual void think();
};

#endif
