/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_MAP_CELL_HH
#define G1_MAP_CELL_HH

#include "arch.hh"
#include "rotation.hh"
#include "player_type.hh"
#include "file/file.hh"
#include "range.hh"
#include "math/point.hh"
#include "obj3d.hh"
#include "g1_limits.hh"
#include "math/point.hh"
#include "f_tables.hh"
#include "map_man.hh"

class g1_object_class;
class g1_solid_class;
class g1_bullet_class;
class g1_map_view_class;
class i4_image_class;
class g1_solid_class;
class g1_saver_class;
class g1_loader_class;
class g1_path_manager_class;
class g1_map_class;
class g1_draw_context_class;
class g1_object_chain_class;
class i4_loader_class;
class i4_saver_class;

struct g1_visible_cell
{
  w8 x,y;
};


class g1_map_cell_class
{
public:
  enum { NO_CRITICAL=0 };


  g1_object_chain_class *object_list;  // lists of objects that are standing on this tile
                                       
  void add_object(g1_object_chain_class &c);
  i4_bool remove_object(g1_object_chain_class &c);

  //  g1_graph_node nearest_critical[G1_GRADE_LEVELS][2][G1_CRITICALS_PER_CELL];
  // tells which critical points are near this cell
  w16 scratch_x, scratch_y;

  w16   type;     // index into an array of ground object types (see tile.hh)
  
  w16   flags;
  w16   top_left_normal;        // 1-5-5-5  top bit indicates needs recalc
  w16   bottom_right_normal;    // 1-5-5-5  top bit indicates needs recalc

  void recalc_top_left(int cx, int cy);
  void recalc_bottom_right(int cx, int cy);

  void  get_top_left_normal(i4_3d_vector &v, int cx, int cy)
  {
    if (top_left_normal & 0x8000)
      recalc_top_left(cx, cy);

    v.x=g1_table_0_31_to_n1_1[(top_left_normal>>10)&31];
    v.y=g1_table_0_31_to_n1_1[(top_left_normal>>5)&31];
    v.z=g1_table_0_31_to_n1_1[(top_left_normal)&31];
  }

  void get_bottom_right_normal(i4_3d_vector &v, int cx, int cy)
  {
    if (bottom_right_normal & 0x8000)
      recalc_bottom_right(cx, cy);

    v.x=g1_table_0_31_to_n1_1[(bottom_right_normal>>10)&31];
    v.y=g1_table_0_31_to_n1_1[(bottom_right_normal>>5)&31];
    v.z=g1_table_0_31_to_n1_1[(bottom_right_normal)&31];
  }


  void  get_bottom_right_normal(float &x, float &y, float &z, int current_cell_offset);

  enum 
  {    
    FOGGED=             (1<<0),             // if fogged from local player's view 
    MIRRORED=           (1<<1),  // if building cell is mirrored across the x-axis

    ROTATE_BITS=        (1<<2)|(1<<3),

    IS_GROUND=          (1<<4),  // if this square doesn't block movement

    RADAR_NEEDS_RESTORE= (1<<5),            // if this spot on radar has been drawn
    RADAR_NEEDS_STACKED_RESTORE = (1<<6),   // if spot has been drawn more than once

    PLANAR=1<<13,      // if surface is planar and can be drawn with one quad rather than 2 tri's
    SCRATCH1=1<<14,
    SCRATCH2=1<<15
  };

  enum { SAVED_FLAGS = FOGGED | MIRRORED | ROTATE_BITS };

  void clear_restore_bits() { flags &= ~(RADAR_NEEDS_STACKED_RESTORE | RADAR_NEEDS_RESTORE); }
  i4_bool mirrored() { return (i4_bool)flags & MIRRORED; }
  

//   g1_player_type owner() 
//   { 
//     w32 ret=(flags&OWNER_BITS)>>8; 
//     //if (ret==31) return g1_no_player;
//     //else
//     return ret;
//   }

//   void set_owner(g1_player_type player)
//   {
//     //if (player==g1_no_player)
//     //  flags|=OWNER_BITS;
//     //else
//       flags=((flags & (~OWNER_BITS)) | (player<<8));
//   }

  void load_v2(i4_file_class *fp, w16 *tile_remap);   // version 2
  void load_v3(i4_file_class *fp, w16 *tile_remap);   // version 3
  void load_v4(i4_file_class *fp, w16 *tile_remap);   // version 4
  void load_v5(i4_file_class *fp, w16 *tile_remap);   // version 5
  void load_v6(i4_file_class *fp, w16 *tile_remap);   // version 6

  void set_rotation(g1_rotation_type type) { flags=(flags & (~ROTATE_BITS))|(type<<2); }
  g1_rotation_type get_rotation() { return (g1_rotation_type)((flags&ROTATE_BITS)>>2); }
  
  void init(w16 _type, g1_rotation_type rotation, i4_bool mirrored);

  i4_bool is_blocking() const { return !(flags&IS_GROUND); }

  void set_planar_flag(w8 h1, w8 h2, w8 h3, w8 h4)
  { 
    if (h1==h2 && h2==h3 && h3==h4)
      flags|=PLANAR;
    else
      flags&=~PLANAR;
  }

  i4_bool list_capable() { return i4_T; }

  g1_object_chain_class *get_solid_list();
  g1_object_chain_class *get_non_solid_list();
  g1_object_chain_class *get_obj_list()  { return object_list; }

  void unfog(int x, int y);
};


i4_bool g1_load_map_cells(g1_map_cell_class *list, int lsize, 
                          w16 *tile_remap, i4_loader_class *fp);

void g1_save_map_cells(g1_map_cell_class *list, int lsize, i4_saver_class *fp);

inline g1_map_cell_class *g1_get_cell(int x, int y)
{
  return g1_cells + y*g1_map_width + x;
}

#endif
