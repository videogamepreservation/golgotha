/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef CONTEXT_HH
#define CONTEXT_HH

#include "area/rectlist.hh"

class i4_draw_context_class
{
  public :
  i4_rect_list_class clip;
  i4_rect_list_class *single_dirty; // all areas that only need updating for the next frame
  i4_rect_list_class *both_dirty;   // all areas that need updating on all future frames
  sw16 xoff,yoff;

  i4_draw_context_class(sw16 x1, sw16 y1, sw16 x2, sw16 y2)
  {
    clip.add_area(x1,y1,x2,y2);
    single_dirty=0;
    both_dirty=0;
    xoff=0;
    yoff=0;
  }
  ~i4_draw_context_class()
  {
    if (single_dirty) 
      delete single_dirty;
    if (both_dirty)
      delete both_dirty;
  }

  void add_single_dirty(sw16 x1, sw16 y1, sw16 x2, sw16 y2)
  {
    if (single_dirty)
      single_dirty->add_area(x1,y1,x2,y2);
  }

  void add_both_dirty(sw16 x1, sw16 y1, sw16 x2, sw16 y2)
  {
    if (both_dirty)
      both_dirty->add_area(x1,y1,x2,y2);
  }

} ;


#endif
