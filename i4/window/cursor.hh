/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef __CURSOR_HPP_
#define __CURSOR_HPP_

class i4_image_class;
#include "image/image.hh"

class i4_cursor_class
{
  public :
  i4_image_class *pict;       // picture of what the mouse actually looks like
  i4_color trans;             // transparent color in image
  i4_coord hot_x,hot_y;       // offset from top-left where hot-spot is

  i4_cursor_class(i4_image_class *pict, 
                  i4_color trans, 
                  i4_coord hot_x, i4_coord hot_y,
                  const i4_pal *convert_to=0);
  
  ~i4_cursor_class();
  i4_cursor_class *copy(const i4_pal *convert_to=0);  
  i4_cursor_class();
} ;

class i4_string_manager_class;

// load cursor takes a "resource string" for a cursor name
// currently a cursor resource should be in the form
// cursorname filename trans_color hot_x hot_y
i4_cursor_class *i4_load_cursor(char *cursor_name, 
                                i4_string_manager_class *sman);


#endif
