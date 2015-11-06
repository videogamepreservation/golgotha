/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_MAP_COLLISION_HH
#define G1_MAP_COLLISION_HH

// make this work in the editor, too
class g1_collision_map_class
{
protected:
  w8 *map;
  w16 wx,wy;
  w16 bwx;
public:
  w16 width() const { return wx; }
  w16 height() const { return wy; }

  g1_collision_map_class() : map(0) {}
  g1_collision_map_class(w16 _wx, w16 _wy) { init(_wy,_wy); }

  void init(w16 _wx, w16 _wy)
  {
    uninit();
    wx = _wx; wy = _wy;
    bwx = (wx+7)/8;
    map = (w8 *)i4_malloc(bwx*wy, "collision_map");
    I4_ASSERT(map, "No block map allocated");
  }

  void uninit()
  {
    if (map)
      i4_free(map);
    map=0;
  }

  ~g1_collision_map_class()
  {
    uninit();
  }

  void clear()
  {
    memset(map, 0, bwx*wy);
  }

  i4_bool is_blocked(w16 x, w16 y) const
  {
    w8 mask = 1<<(x&7);
    x /= 8;
    return (map[y*bwx+x] & mask)!=0;
  }

  void set_blocking(w16 x, w16 y, i4_bool flag)
  {
    w8 mask = 1<<(x&7), bit = flag<<(x&7);
    x /= 8;
    map[y*bwx+x] = (map[y*bwx+x]&~mask)|bit;
  }
};

#endif
