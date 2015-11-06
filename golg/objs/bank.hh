/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_BANK_HH
#define G1_BANK_HH

#include "g1_object.hh"
#include "inc/isllist.hh"
#include "objs/bases.hh"
#include "math/vector.hh"

class g1_moneycrate_class;

class g1_bank_class : public g1_factory_class
{
protected:
  i4_3d_vector crate_pos;
public:
  g1_bank_class *next;
  g1_moneycrate_class* crate();
  void set_crate(g1_moneycrate_class *cr);

  g1_bank_class(g1_object_type id, g1_loader_class *fp);

  void crate_location(i4_3d_vector &pos);
  void create_crate();
  void detach_crate();
  
  virtual i4_bool occupy_location();
  virtual void unoccupy_location();
  virtual void change_player_num(int new_team);
  virtual void think();
  virtual i4_bool build(int type);
  virtual g1_path_object_class *get_start() { return 0; }
  virtual void request_remove();
};

extern i4_isl_list<g1_bank_class> g1_bank_list[G1_MAX_PLAYERS];
extern int g1_bank_list_count[G1_MAX_PLAYERS];

#endif
