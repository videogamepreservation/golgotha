/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_FILL_HH
#define G1_FILL_HH

#include "arch.hh"

class g1_flood_fill_class
{
  class fill_rec
  {
    public :
    short x,y;
    fill_rec *last;
    fill_rec(short X, short Y, fill_rec *Last)
    { x=X; y=Y; last=Last; }
  } ;

public:
  virtual void get_clip(sw32 &x1, sw32 &y1, sw32 &x2, sw32 &y2) = 0;
  virtual i4_bool blocking(sw32 x, sw32 y) = 0;
  virtual void fill_block(sw32 x, sw32 y, sw32 startx, sw32 starty) =0;
  void fill(sw32 x, sw32 y);
};



#endif
