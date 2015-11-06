/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_CRATE_HH
#define G1_CRATE_HH


#include "g1_object.hh"
#include "resources.hh"

class g1_crate_class : public g1_object_class
{

  int ticks_to_think;

public:
  enum ctype { HEALTH,
               MISSILE,
               BULLET,
               CHAIN,
               MONEY,
               MAX_TYPES
  };

  enum atype { SMALL,
               LARGE };


  ctype get_type();
  void set_type(ctype x);

  atype get_amount();
  void set_amount(atype x);

  int &ticks_left();
  float &yvel(); 
  float float_height() { return 0.2; }

  int added_money();
  int added_bullets();
  int added_health();
  int added_missiles();
  int added_chain();
    

  g1_crate_class(g1_object_type id, g1_loader_class *fp);
  i4_bool occupy_location();
  void draw(g1_draw_context_class *context);

  void setup(const i4_3d_vector &pos, ctype t, atype amount, int ticks=-1);
  void think();
  void object_changed_by_editor(g1_object_class *who, li_class *old_values);
  void note_stank_near(g1_player_piece_class *s);
  void go_away();

};


#endif
