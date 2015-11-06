/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "path_api.hh"
#include "solvemap_astar.hh" //(OLI) Debug hack
#include <math.h>

extern g1_astar_map_solver_class *g1_solvemap;

int g1_block_map_class::max_fit_NS(int x, int y, g1_compass_direction dir, int max) const
{
  int k;
  
  k=0; 
  while (k<max && 
         x-k>=0 &&
         !is_blocked(x-k,y,dir) &&
         x+k<width() &&
         !is_blocked(x+k,y,dir))
    k++;
  return k;
}

int g1_block_map_class::max_fit_WE(int x, int y, g1_compass_direction dir, int max) const
{
  int k;

  k=0; 
  while (k<max && 
         y-k>=0 &&
         !is_blocked(x,y-k,dir) &&
         y+k<height() &&
         !is_blocked(x,y+k,dir))
    k++;
  return k;
}

#if 1
int floor(i4_float val) { return i4_f_to_i(val); }
int ceil(i4_float val) { return 2048-i4_f_to_i(2048.0-val); }

i4_float g1_block_map_class::line_of_sight(i4_float x1, i4_float y1, i4_float x2, i4_float y2,
                                           i4_float rad_x, i4_float rad_y) const
{
  i4_float x, y, dx, fit, max_fit;
  i4_float tmp, dist_y, dist_x;
  int ix, iix, iy, lx, rx, nlx, nrx, min_lx, max_rx, lbx, rbx;
  int ey;
  int flip;
  w8 dir;
    
  //{{{ setup

  // limit rad values
  if (y1-rad_y<0) rad_y = y1;
  if (y1+rad_y>i4_float(height())) rad_y = i4_float(height()) - y1;
  if (y2-rad_y<0) rad_y = y2;
  if (y2+rad_y>i4_float(height())) rad_y = i4_float(height()) - y2;
  if (x1-rad_x<0) rad_x = x1;
  if (x1+rad_x>i4_float(width())) rad_x = i4_float(width()) - x1;
  if (x2-rad_x<0) rad_x = x2;
  if (x2+rad_x>i4_float(width())) rad_x = i4_float(width()) - x2;

  dist_y = y2-y1;
  dist_x = x2-x1;

    // direction of blocking that would stop movement
  dir = ((dist_y<0)? G1_NORTH : G1_SOUTH) | ((dist_x<0)? G1_EAST : G1_WEST);

  // absolute values of distances
  dist_y = (dist_y<0) ? -dist_y: dist_y;
  dist_x = (dist_x<0) ? -dist_x: dist_x;

  if (dist_x>dist_y)
  {
    // flip axes for easier processing
    flip=1;
    tmp = x1; x1 = y1; y1 = tmp;
    tmp = x2; x2 = y2; y2 = tmp;
    tmp = rad_x; rad_x = rad_y; rad_y = tmp;
  }
  else
    flip=0;

  if (y1>y2)
  {
    tmp = x1; x1 = x2; x2 = tmp;
    tmp = y1; y1 = y2; y2 = tmp;
  }

  dx = (x2-x1)/(y2-y1);                         // change in X over y
  iy = floor(y1-rad_y);                         // integer 'pixel' location
  y = iy;
  ey = ceil(y2+rad_y);                          // ending row
  x = x1 + (i4_float(iy+1)-y1)*dx;              // center of edge
  if (dx>0)
  {
    lbx = floor(x1-rad_x);                      // bounding edges
    rbx = ceil(x2+rad_x);
    lx = lbx;                                   // initial left edge
    rx = ceil(x1+rad_x);                        // initial right edge
    max_fit = rad_x + rad_y*dx;                 // initial corridor width
  }
  else
  {
    lbx = floor(x2-rad_x);
    rbx = ceil(x1+rad_x);
    lx = floor(x1-rad_x);
    rx = rbx;
    max_fit = rad_x - rad_y*dx;
  }
  //}}}

  // Edge Stepping
  while (iy<ey && max_fit>0)
  {
    // calculate locations of next edges
    nlx = floor(x - max_fit);
    nlx = (lbx>nlx)? lbx : nlx;
    nrx = ceil(x + max_fit);
    nrx = (rbx<nrx)? rbx : nrx;

    // scan row
    ix = floor(x),
      min_lx = (dx>0)? lx  : nlx,
      max_rx = (dx>0)? nrx : rx ;

    for (iix=min_lx; iix<max_rx; iix++)
    {
      //{{{ (OLI) Debug hack
#if 0
      if (g1_solvemap)
        if (flip)
          g1_solvemap->ok(iy,iix);
        else
          g1_solvemap->ok(iix,iy);
#endif
      //}}}
      if ((!flip && is_blocked(iix,iy,dir)) ||
          (flip && is_blocked(iy,iix,dir)))
      {
        if (iix==ix)
          fit = 0.0;
        else if (iix<ix)
          fit = x - i4_float(iix+1);
        else
          fit = i4_float(iix) - x;
        if (y<y1)
        {
          i4_float fity = (y1-y-1);
          fit = (fity>fit)? fity : fit;
        }
        else if (y>y2)
        {
          i4_float fity = (y-y2);
          fit = (fity>fit)? fity : fit;
        }
        max_fit = (fit<max_fit)? fit : max_fit;
      }
    }

    // update values to next row
    iy++;
    x += dx;
    y += 1.0;
    lx = nlx; rx = nrx;
  }

  i4_float d = sqrt(1.0+dx*dx);
  return max_fit/d;
}
#else
i4_float g1_block_map_class::line_of_sight(i4_float _x1, i4_float _y1, i4_float _x2, i4_float _y2,
                                           i4_float rad_x, i4_float rad_y) const
{
  int x1=int(_x1),y1=int(_y1),x2=int(_x2),y2=int(_y2);

  int max_fit = 20;
  int 
    dx=x2-x1,
    dy=y2-y1,
    sx,sy,
    x,y,
    xf,yf,
    nx,ny;
    
  g1_compass_direction xdir,ydir;

  // determine step directions
  if (dx>=0)  {   sx=1;  xdir = G1_WEST;  }
  else        {   sx=-1; xdir = G1_EAST; dx=-dx; }
  if (dy>=0)  {   sy=1;  ydir = G1_SOUTH; }
  else        {   sy=-1; ydir = G1_NORTH; dy=-dy; }

  // find maximum size convoy that can fit at start point
  int i=0;
  while ((max_fit = max_fit_NS(x1,y1+i,g1_compass_direction(xdir|ydir),max_fit))>=i &&
         (max_fit = max_fit_NS(x1,y1-i,g1_compass_direction(xdir|ydir),max_fit))>=i)
    i++;
  max_fit=i-1;

  // initial positions & fractions
  x = x1;    y =y1;
  xf = dy/2; yf = dx/2;

  if (dx>dy)
  {
    while (x!=x2 || y!=y2) 
    {
      ny = dy-xf + yf;
      if (ny>dx)
      {
        while (max_fit_NS(x,y+sy*max_fit,ydir,max_fit)<max_fit)
          max_fit--;
        if (max_fit==0)
          return 0;
        y += sy;
        xf = dx-yf + xf;
        yf = 0;
      }
      else
      {
        while (max_fit_WE(x+sx*max_fit,y,xdir,max_fit)<max_fit)
          max_fit--;
        if (max_fit==0)
          return 0;
        x += sx;
        yf = ny;
        xf = 0;
      }
    }
  }
  else
  {
    while (x!=x2 || y!=y2) 
    {
      nx = dx-yf + xf;
      if (nx>dy)
      {
        while(max_fit_WE(x+sx*max_fit,y,xdir,max_fit)<max_fit)
          max_fit--;
        if (max_fit==0)
          return 0;
        x += sx;
        yf = dy-xf + yf;
        xf = 0;
      }
      else
      {
        while(max_fit_NS(x,y+sy*max_fit,ydir,max_fit)<max_fit)
          max_fit--;
        if (max_fit==0)
          return 0;
        y += sy;
        xf = nx;
        yf = 0;
      }
    }
  }
  return max_fit;
}
#endif
