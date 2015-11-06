/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_MAP_VARS_HH
#define G1_MAP_VARS_HH

#include "map_data.hh"
#include "lisp/li_optr.hh"

class li_class;

class g1_map_vars_class : public g1_map_data_class
{
public:
  li_object_pointer var_ptr;
  li_class *vars() { return (li_class *)var_ptr.get(); }

  g1_map_vars_class() : g1_map_data_class("vars") {}

  virtual void load(g1_loader_class *fp, w32 sections);
  virtual void save(g1_saver_class *fp, w32 sections);
};

extern g1_map_vars_class  g1_map_vars;


#endif
