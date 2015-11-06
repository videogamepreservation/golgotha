/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/



#include "visible.hh"
#include "map.hh"
#include "map_cell.hh"

/* 
Fast Line-Edge Intersections on a Uniform Grid 
by Andrew Shapira
from "Graphics Gems", Academic Press, 1990
*/



#define G1_STOP(hx, hy) \
{                 \
  S("stopped"); x2=lx;          \
  y2=ly;          \
  hit_x=hx;       \
  hit_y=hy;       \
  return i4_F;    \
}

#define S(x) { i4_warning("%s : %d %d",x,cx,cy); }

#define OCTANT(f1, f2, f3, f4, f5, i1, s1, r1, r2)              \
{                                                               \
for (f1, f2, f3, nr = 0; f4; f5)                                \
{                                                               \
  if (nr < liconst)                                             \
  {                                                             \
    if (i1)                                                     \
    { S("test 1");                                                          \
      if (map->cell(cx, cy)->is_blocking())                      \
        { G1_STOP(cx,cy);  }                                    \
    } else if  (map->cell(cx, cy)->is_blocking())                \
    { G1_STOP(cx,cy); }                                         \
    else if (map->cell(cx-1, cy-1)->is_blocking())               \
    { G1_STOP(cx-1,cy-1); }                                     \
    else if (map->cell(cx, cy-1)->is_blocking())                 \
    { G1_STOP(cx,cy-1); }                                       \
    else if (map->cell(cx-1, cy)->is_blocking())                 \
    { G1_STOP(cx-1, cy);  }                                     \
  }                                                             \
  else                                                          \
  {                                                             \
    s1;                                                         \
    if (nr -= liconst)                                          \
    {                                                           \
      S("test 2"); if (map->cell(cx, cy)->is_blocking())                      \
        { G1_STOP(cx,cy);  }                                    \
    } else if  (map->cell(cx, cy)->is_blocking())                \
    { G1_STOP(cx,cy); }                                         \
    else if (map->cell(cx-1, cy-1)->is_blocking())               \
    { G1_STOP(cx-1,cy-1); }                                     \
    else if (map->cell(cx, cy-1)->is_blocking())                 \
    { G1_STOP(cx,cy-1); }                                       \
    else if (map->cell(cx-1, cy)->is_blocking())                 \
    { G1_STOP(cx-1, cy);  }                                     \
  }                                                             \
  lx=cx;                                                        \
  ly=cy;                                                        \
}                                                               \
return i4_T;                                                    \
}                                                               

i4_bool g1_visibility_check_old(sw32 x1, sw32 y1, 
                            sw32 &x2, sw32 &y2, 
                            sw32 &hit_x, sw32 &hit_y,
                            g1_map_class *map)

{
  sw32 cx, cy, lx=x1, ly=y1;

  sw32         nr;             /* remainder */
  sw32         deltax, deltay; /* Q.x - P.x, Q.y - P.y */
  sw32         liconst;          /* loop-invariant constant */

  deltax = x2 - x1;
  deltay = y2 - y1;

  if (x1==x2)
  {
    int add=y1>y2 ? -1 : 1;
    while (y1!=y2)
    {
      if (map->cell(x1,y1)->is_blocking())
      {
        hit_x=x1;
        hit_y=y1;
        y2=y1-add;
        return i4_F;
      }
      y1+=add;
    }
  }
  else if (y1==y2)
  {
    int add=x1>x2 ? -1 : 1;
    while (x1!=x2)
    {
      if (map->cell(x1,y1)->is_blocking())
      {
        hit_x=x1;
        hit_y=y1;
        x2=x1-add;
        return i4_F;
      }
      x1+=add;
    }
  }


  i4_warning("--------------");
      

  /* for reference purposes, let theta be the angle from P to Q */

  if ((deltax >= 0) && (deltay >= 0) && (deltay < deltax)) 
    /* 0 <= theta < 45 */
    OCTANT(cx = x1 + 1, cy = y1, liconst = deltax - deltay, 
           cx < x2, cx++, nr += deltay, cy++, up, left)
  else if ((deltax > 0) && (deltay >= 0) && (deltay >= deltax)) 
    /* 45 <= theta < 90 */
    OCTANT(cy = y1 + 1, cx = x1, liconst = deltay - deltax,
           cy < y2, cy++, nr += deltax, cx++, right, down)
  else if ((deltax <= 0) && (deltay >= 0) && (deltay > -deltax))
    /* 90 <= theta < 135 */
    OCTANT(cy = y1 + 1, cx = x1, liconst = deltay + deltax,
           cy < y2, cy++, nr -= deltax, cx--, left, down)
  else if ((deltax <= 0) && (deltay > 0) && (deltay <= -deltax)) 
    /* 135 <= theta < 180 */
    OCTANT(cx = x1 - 1, cy = y1, liconst = -deltax - deltay,
           cx > x2, cx--, nr += deltay, cy++, up, right)
  else if ((deltax <= 0) && (deltay <= 0) && (deltay > deltax))
    /* 180 <= theta < 225 */
    OCTANT(cx = x1 - 1, cy = y1, liconst = -deltax + deltay, 
           cx > x2, cx--, nr -= deltay, cy--, down, right)
  else if ((deltax < 0) && (deltay <= 0) && (deltay <= deltax))
    /* 225 <= theta < 270 */
    OCTANT(cy = y1 - 1, cx = x1, liconst = -deltay + deltax,
           cy > y2, cy--, nr -= deltax, cx--, left, up)
  else if ((deltax >= 0) && (deltay <= 0) && (-deltay > deltax))
    /* 270 <= theta < 315 */
    OCTANT(cy = y1 - 1, cx = x1, liconst = -deltay - deltax, 
           cy > y2, cy--, nr += deltax, cx++, right, up)
  else if ((deltax >= 0) && (deltay < 0) && (-deltay <= deltax))
    /* 315 <= theta < 360 */
    OCTANT(cx = x1 + 1, cy = y1, liconst = deltax + deltay,
           cx < x2, cx++, nr -= deltay, cy--, down, left)
  return i4_T;
                                            
}


i4_bool g1_visibility_check(sw32 x1, sw32 y1, 
                            sw32 &x2, sw32 &y2, 
                            sw32 &hit_x, sw32 &hit_y,
                            g1_map_class *map)
{
  // this does a fixed point step through the blocks the line passes through

  enum { POINT_5=(0xffff/2) };

  sw32 fx=(x1<<16)+POINT_5, fy=(y1<<16)+POINT_5, lx,ly,nx,ny;
  lx=x1; ly=y1;
 
  sw32 sx=(x2-x1), sy=(y2-y1), t;
  if (x1==x2 && y1==y2)
    return i4_T;

  if (abs(sx)>abs(sy))
    t=abs(sx);
  else
    t=abs(sy);

  sx=(sx<<16)/t;
  sy=(sy<<16)/t;


  while (t)
  {
    



    fx+=sx;    fy+=sy;
    nx=fx>>16; ny=fy>>16;

    if (nx!=lx && map->cell(nx, ly)->is_blocking())
    {
      x2=lx;     y2=ly;      hit_x=nx;  hit_y=ly;      return i4_F;
    }
    else if (ny!=ly && map->cell(lx, ny)->is_blocking())
    {
      x2=lx;     y2=ly;      hit_x=lx;  hit_y=ny;      return i4_F;
    }
    else if ((nx!=lx && ny!=ly) && map->cell(nx, ny)->is_blocking())
    {
      x2=lx;     y2=ly;      hit_x=nx;  hit_y=ny;      return i4_F;
    }

    lx=fx>>16; ly=fy>>16;          

    t--;
  }
  return i4_T;
}
