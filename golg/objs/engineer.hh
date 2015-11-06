/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_ENGINEER_HH
#define G1_ENGINEER_HH

#include "objs/map_piece.hh"

class g1_engineer_class : public g1_map_piece_class
{
protected: 
  g1_mini_object     *back_wheels;
  g1_mini_object     *front_wheels;


public:
  g1_engineer_class(g1_object_type id, g1_loader_class *fp);
  virtual void save(g1_saver_class *fp);

  virtual void think();

  virtual li_object *message(li_symbol *message_name,
                             li_object *message_params, 
                             li_environment *env);
};

#endif
