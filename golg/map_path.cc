/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "map.hh"
#include "visible.hh"
#include "solvemap_astar.hh"

i4_bool g1_map_class::find_path(i4_float start_x, i4_float start_y,
                                i4_float destx, i4_float desty,
                                i4_float *points, w16 &t_nodes)
{
  if (destx<0) destx=0; else if (destx>=w) destx = w-1;
  if (desty<0) desty=0; else if (desty>=h) desty = h-1;

  // jc: fixme
  //  i4_float tt=get_block_map(grade)->line_of_sight(c->x,c->y,destx,desty, rad_x, rad_y);
  //  i4_float tc=c->calculate_size(destx - c->x,desty - c->y)-0.001;
  if (0) //tt>=tc)
  {
    // if the destination is visible, then path is just a line
    points[0]=destx;
    points[1]=desty;
    //    points[2]=c->x;
    //    points[3]=c->y;
    t_nodes=2;
  }
  else
  {
    if (!solver->path_solve(start_x, start_y,
                            destx, desty,
                            points, t_nodes))
    {
      t_nodes=0;
      return i4_F;
    }
  }
  return i4_T;
}
