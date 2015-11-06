/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "editor/commands/fill.hh"

void g1_flood_fill_class::fill(sw32 x, sw32 y)
{
  if (blocking(x,y))
    return ;
      
  fill_rec *recs=0,*r;

  sw32 startx=x,starty=y;
  sw32 clip_x1, clip_y1, clip_x2, clip_y2;
  get_clip(clip_x1, clip_y1, clip_x2, clip_y2);

  do
  {
    if (recs)
    { 
      r=recs;
      recs=recs->last;
      x=r->x; y=r->y;
      delete r;
    }


    if (!blocking(x,y))
    {
      while (x>clip_x1 && !blocking(x,y)) 
        x--;

      if (blocking(x,y) && x<clip_x2)
        x++;


      if (y>clip_y1 && !blocking(x,y-1))
        recs=new fill_rec(x,y-1,recs);

      if (y<clip_y2 && !blocking(x, y+1))
        recs=new fill_rec(x,y+1,recs);

      do
      {
        fill_block(x, y, startx, starty);

        if (y>clip_y1 && x>clip_x1 && blocking(x-1, y-1) && !blocking(x, y-1))
          recs=new fill_rec(x,y-1,recs);


        if (y<clip_y2 && x>clip_x1 && blocking(x-1, y+1) && !blocking(x, y+1))
          recs=new fill_rec(x,y+1,recs);

        x++;
      } while (!blocking(x,y) && x<clip_x2);


      x--;
      if (y>clip_y1 && !blocking(x, y-1))
        recs=new fill_rec(x,y-1,recs);

      if (y<clip_y2 && !blocking(x, y+1))
        recs=new fill_rec(x,y+1,recs);
    }
  } while (recs);
}



