/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef REMOVE_MAN_HH
#define REMOVE_MAN_HH

#include "init/init.hh"

class g1_object_class;

class g1_remove_manager_class : public i4_init_class
{
  g1_object_class **remove_list;
  int removes;

  friend class g1_map_class;
  virtual void init();
  virtual void uninit();
public:

  void process_requests();
  void request_remove(g1_object_class *obj);

};

extern g1_remove_manager_class g1_remove_man;

#endif

//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}
