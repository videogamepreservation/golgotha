/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

class i4_window_class;
class i4_parent_window_class;
#include "string/string.hh"

void i4_create_dialog(const i4_const_str &fmt,
                      i4_parent_window_class *parent,
                      i4_graphical_style_class *style,
                      ...);
