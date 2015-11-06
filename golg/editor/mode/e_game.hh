/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef _E_GAME_HH__
#define _E_GAME_HH__


#include "editor/e_mode.hh"

class g1_critical_graph_class;
class g1_game_mode : public g1_mode_handler
{

public:
  virtual state current_state();
  g1_ai_mode(g1_controller_edit_class *c);
};




#endif
