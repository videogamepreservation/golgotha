/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#error

#ifndef PATH_API_HH
#define PATH_API_HH

#include "error/error.hh"
#include "memory/malloc.hh"
#include "math/num_type.hh"
#include <string.h>

enum g1_compass_direction { G1_NORTH=1, G1_SOUTH=2, G1_WEST=4, G1_EAST=8 };

class g1_block_map_class
{
protected:
  w8 *block_map;
  w16 wx,wy;
  w16 bwx;

  int max_fit_NS(int x, int y, g1_compass_direction dir, int max) const;
  int max_fit_WE(int x, int y, g1_compass_direction dir, int max) const;

public:
  w16 width() const { return wx; }
  w16 height() const { return wy; }

  g1_block_map_class() : block_map(0) {}
  g1_block_map_class(w16 _wx, w16 _wy) { init(_wy,_wy); }

  void init(w16 _wx, w16 _wy)
  {
    wx = _wx; wy = _wy;
    bwx = (wx+1)/2;
    block_map = (w8 *)i4_malloc(bwx*wy, "block_map");
    I4_ASSERT(block_map, "No block map allocated");
  }

  void uninit()
  {
    if (block_map)
      i4_free(block_map);
    block_map=0;
  }

  ~g1_block_map_class()
  {
    uninit();
  }

  void clear()
  {
    memset(block_map, 0, bwx*wy);
  }

  void block(w16 x, w16 y, w8 dir)
  {
    if (x&1)
      dir <<= 4;
    x /= 2;
    block_map[y*bwx+x] |= dir;
  }

  void unblock(w16 x, w16 y, w8 dir)
  {
    if (x&1)
      dir <<= 4;
    x /= 2;
    block_map[y*bwx+x] &= ~(dir);
  }

  i4_bool is_blocked(w16 x, w16 y, w8 dir) const
  {
    if (x&1)
      dir <<= 4;
    x /= 2;
    return (block_map[y*bwx+x] & dir)!=0;
  }

  i4_float line_of_sight(i4_float x1, i4_float y1, i4_float x2, i4_float y2,
                         i4_float rad_x=5.0, i4_float rad_y=5.0) const;
};

#endif

