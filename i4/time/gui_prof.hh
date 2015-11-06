/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "arch.hh"

class i4_window_class;
class i4_parent_window_class;
class i4_graphical_style_class;
class i4_event_reaction_class;

void i4_profile_watch(i4_graphical_style_class *style,
                      i4_parent_window_class *parent,                      
                      sw32 &win_x, sw32 &win_y,
                      w32 w, w32 h,
                      int open_type,  // 0==close, 1==open, 2==toggle window
                      i4_event_reaction_class *on_close=0);
