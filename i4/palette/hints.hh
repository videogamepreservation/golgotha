/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef __HINTS_HPP_
#define __HINTS_HPP_

#include "palette/pal.hh"
#include "font/font.hh"

class i4_color_hint_class
{
  public :
  i4_color black;

  struct bevel { i4_color bright,medium,dark,text;  } ;
  
  struct bevel_actor  { bevel active,passive;  } ;
  
  bevel_actor window, button;


  i4_font_class *window_font,
          *button_font;

} ;

#endif
