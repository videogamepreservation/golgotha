/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_BASES_HH
#define G1_BASES_HH

#include "isllist.hh"
#include "memory/que.hh"
#include "g1_object.hh"
#include "objs/structure_death.hh"

class g1_path_object_class;

struct g1_build_item
{
  int type;
  g1_id_ref *path;  // null terminated
};

class g1_factory_class : public g1_object_class
{
protected:
  g1_structure_death_class death;
  g1_map_piece_class *create_object(int type);
public:
  g1_factory_class *next;
  i4_fixed_que<g1_build_item, 50> deploy_que;
  
  g1_factory_class(g1_object_type id, g1_loader_class *fp);  
  void think();
  i4_bool occupy_location();
  void unoccupy_location();
  virtual g1_path_object_class *get_start();
  virtual void set_start(g1_path_object_class *start);
  virtual i4_bool build(int type);
  virtual w32 get_path_color();
  virtual w32 get_selected_path_color();

  void damage(g1_object_class *obj, int hp, i4_3d_vector damage_dir);
};

extern i4_isl_list<g1_factory_class> g1_factory_list;

#endif
