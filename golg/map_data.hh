/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_MAP_DATA_HH
#define G1_MAP_DATA_HH

#include "arch.hh"

class g1_loader_class;
class g1_saver_class;

class g1_map_data_class
{
public:
  char *name;
  
  g1_map_data_class *next;
  static g1_map_data_class *first;

  g1_map_data_class(char *name);
  ~g1_map_data_class();


  virtual void free()                                  {}
  virtual void load(g1_loader_class *fp, w32 sections) {}
  virtual void save(g1_saver_class *fp, w32 sections) {}
};

#endif
