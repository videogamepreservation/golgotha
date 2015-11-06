/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "editor/e_game.hh"

g1_mode_handler::state g1_game_mode::current_state()
{
  w8 remap[]={ ROTATE,
               ZOOM,
               DRAG_SELECT,
               DRAG_SELECT,
               OTHER,
               OTHER
  };

  I4_ASSERT(g1_edit_state.ai.minor_mode<=sizeof(remap), "state too big");

  return (g1_mode_handler::state)remap[g1_edit_state.ai.minor_mode];
}


g1_game_mode::g1_game_mode(g1_controller_edit_class *c)
  : g1_mode_handler(c)
{
}
