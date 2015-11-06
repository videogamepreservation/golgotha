/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_PATH_HH
#define G1_PATH_HH

#include "arch.hh"
#include "math/num_type.hh"
#include "memory/growarry.hh"

class g1_saver_class;
class g1_loader_class;

// this is defined internally in path.cc, you don't need to know the details, use
// the path manager to find out info about your path
class g1_path_class;

typedef g1_path_class *g1_path_handle;

class g1_path_manager_class
{
public:
  g1_path_handle alloc_path(w32 t_positions,
                            i4_float *positions);

  void get_position(g1_path_handle path, i4_float &x, i4_float &y);
  i4_bool get_nth_position(g1_path_handle path, int n, i4_float &x, i4_float &y);

  // advance_path automatically frees the path after the last
  // reference gets to the end of it
  g1_path_handle advance_path(g1_path_handle path);

  i4_bool is_last_path_point(g1_path_handle path);
  
  void free_path(g1_path_handle &path);
      
  void save(g1_saver_class *fp, g1_path_handle path);
  g1_path_handle load(g1_loader_class *fp);
};

extern g1_path_manager_class g1_path_manager;

#endif
