/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef __MAP_HPP_
#define __MAP_HPP_


#include "arch.hh"
#include "error/error.hh"
#include "g1_limits.hh"
#include "player_type.hh"
#include "time/time.hh"
#include "g1_vert.hh"
#include "g1_object.hh"
#include "reference.hh"
#include "global_id.hh"

class g1_map_cell_class;
class g1_map_vertex_class;
class g1_draw_context_class;
class g1_obj_conscell_class;
class g1_solid_class;
class g1_bullet_class;
class g1_solid_class;
class g1_saver_class;
class g1_loader_class;
class g1_path_manager_class;
class g1_visible_projection;
class g1_quad_object_class;
class g1_movie_flow_class;
class g1_light_object_class;
class i4_str;
class i4_polygon_class;
struct g1_visible_cell;
class i4_pal_handle_class;
class g1_astar_map_solver_class;
class g1_takeover_pad_class;
//class g1_critical_graph_class;
typedef g1_typed_reference_class<g1_takeover_pad_class> g1_takeover_pad_ref;

// these bit flags are passed into save & load for a map
// this can be used to merge in sections of another map or reduce the amount
// of undo info need to be saved and loaded
enum {
  G1_MAP_CELLS=(1<<0),
  G1_MAP_VERTS=(1<<1),
  G1_MAP_OBJECTS=(1<<2),
  G1_MAP_MOVIE=(1<<3),
  G1_MAP_PLAYERS=(1<<4),
  G1_MAP_TICK=(1<<5),
  G1_MAP_GRAPH=(1<<6),
  G1_MAP_LIGHTS=(1<<7),
  G1_MAP_SKY=(1<<8),
  G1_MAP_VARS=(1<<9),
  G1_MAP_VIEW_POSITIONS=(1<<11),
  G1_MAP_RES_FILENAME=(1<<12),
  G1_LAST_THING,
  G1_MAP_ALL=(G1_LAST_THING-1)*2-1,

  G1_MAP_SELECTED_VERTS=(1<<13)  // not part of G1_MAP_ALL because G1_MAP_VERTS encompases
};


enum {
  G1_RECALC_RADAR_VIEW         = (1<<0),
  G1_RECALC_PAD_LIST           = (1<<1),
//   G1_RECALC_BLOCK_MAPS         = (1<<2), 
//   G1_RECALC_CRITICAL_DATA      = (1<<3),
  G1_RECALC_WATER_VERTS        = (1<<4),  // determines which verts are drawn with wave motion
  G1_RECALC_STATIC_LIGHT       = (1<<5)  // determines how much directional light hits each cell
};


class g1_map_class
{
private:
  friend class g1_map_view_class;
  friend class g1_editor_class;

  friend i4_bool g1_load_level(const i4_const_str &filename,
                               int reload_textures_and_models,
                               w32 exclude_flags);

//  friend class g1_critical_map_maker_class;  
  friend class g1_object_controller_class;

  void save_objects(g1_saver_class *out);

  w32 recalc;       // bits telling what need recalculating set by add_undo in editor
  w32 w,h;

  g1_map_cell_class *cells;       // 2d array (w*h) of tile type, rotation, blocking status
  g1_map_vertex_class *verts;    // 2d array ((w+1)*(h+1)) of height and lighting info of corners

//   g1_block_map_class block[G1_GRADE_LEVELS];    // blockage maps for different slopes
//   g1_collision_map_class collide; // collision map for objects

//   g1_critical_graph_class *critical_graph;

  enum { THINK_QUE_SIZE=G1_MAX_OBJECTS };
  g1_object_class *think_que[THINK_QUE_SIZE];
  w32 think_head, think_tail;

  
  void delete_map_view();

  g1_object_class **load_objects(g1_loader_class *fp, w32 &tobjs);

  // returns sections actually loaded
  w32 load(g1_loader_class *fp, w32 sections);

  i4_bool load_sky(g1_loader_class *fp);
  void save_sky(g1_saver_class *fp);

//   void save_critical_map(g1_saver_class *f);
//   void load_critical_map(g1_loader_class *f);

  g1_astar_map_solver_class *solver;
  g1_path_manager_class *path_manager;
  

  // this is a hook so the level editor can draw selected verts
  typedef void (*cell_draw_function_type)(sw32 x, sw32 y, void *context);
  cell_draw_function_type post_cell_draw;
  void *post_cell_draw_context;

  i4_str *filename;

  void init_lod();
  void calc_map_lod(g1_object_controller_class *);

  i4_bool movie_in_progress;

public:
  // only use this if you know what you are doing
  void change_map(int w, int h, g1_map_cell_class *cells, g1_map_vertex_class *vertex);
  
  i4_str *sky_name;

  void recalc_static_stuff();

  void remove_from_think_list(g1_object_class *o);

  void mark_for_recalc(w32 flags) { recalc |= flags; }

//   g1_block_map_class *get_block_map(w8 grade) const { return (g1_block_map_class*)&block[grade]; }

  i4_bool playing_movie() { return movie_in_progress; }  

  i4_const_str get_filename();
  void set_filename(const i4_const_str &fname);
    

  void set_post_cell_draw_function(cell_draw_function_type fun, void *context) 
  { 
    post_cell_draw=fun; 
    post_cell_draw_context=context;
  }


  i4_bool start_movie();
  void stop_movie();
  i4_bool advance_movie_with_time();

  g1_movie_flow_class *current_movie;

  g1_movie_flow_class *get_current_movie() { return current_movie; }

  w32 get_tick();
  i4_time_class tick_time;  

  w16 width()  const { return w; }
  w16 height() const { return h; }

  class range_iterator
  {
  protected:
    sw32 left, right, top, bottom, ix, iy;
    g1_map_cell_class *cell;
    g1_object_chain_class *chain;
    w32 object_mask_flags, type_mask_flags;
  public:
    void begin(float x, float y, float range);
    void mask(w32 _object_mask_flags, w32 _type_mask_flags=0xffffffff)
    {
      object_mask_flags = _object_mask_flags;
      type_mask_flags = _type_mask_flags;
    }
    void safe_restart();
    i4_bool end();
    void next();

    g1_object_class *get() const;

    g1_object_class *operator*() { return get(); }
    range_iterator& operator++() { next(); return *this;}
    range_iterator& operator++(int) { next(); return *this;}
  };

  sw32 get_objects_in_range(float x, float y, float range, 
                            g1_object_class *dest_array[], w32 array_size,
                            w32 object_mask_flags=0xffffffff, w32 type_mask_flags=0xffffffff);
                            

  g1_map_cell_class *cell(w16 x, w16 y) const;
  g1_map_cell_class *cell(w32 offset) const;
  g1_map_vertex_class *vertex(w16 x, w16 y) const;

  void request_think(g1_object_class *obj)
  {
    think_que[think_head]=obj;

    think_head++;
    if (think_head>=THINK_QUE_SIZE)
      think_head=0;
    
    if (think_head==think_tail)
      i4_error("g1_map_class::request_think - thinkers exceeded maximum");
  }

  void request_remove(g1_object_class *obj);

  void add_object(g1_object_chain_class &c, w32 x, w32 y);
  void remove_object(g1_object_chain_class &c);


  void remove_object_type(g1_object_type type);

  g1_map_class(const i4_const_str &fname);

//   void make_block_maps();

  void draw(g1_draw_context_class *context,
            i4_float player_x, 
            i4_float player_y,
            i4_float player_z,
            i4_angle player_angle);

  void draw_cells(g1_draw_context_class  *context,
                  g1_visible_cell *cell_list,
                  int t_visible_cells);


  void fast_draw_cells(g1_draw_context_class  *context);
//                        g1_visible_cell *cell_list,
//                        int t_visible_cells);


  void think_objects();


  // check so see if an object can move to the position x,y blocking is not checked
  // against solids on the same team.
  // returns 1 if hit object, -1 if hit building, and 0 if nothing
  int check_non_player_collision(g1_player_type player_num,
                                 const i4_3d_vector &point,
                                 i4_3d_vector &ray,
                                 g1_object_class*& hit) const;

  int check_terrain_location(i4_float x, i4_float y, i4_float z,
                             i4_float occupancy_radius,
                             w8 grade, w8 dir) const;

  void save(g1_saver_class *out, w32 sections);

  g1_object_class *find_object_by_id(w32 object_id, g1_player_type prefered_team);

  // this is not stank specific!
  i4_bool find_path(i4_float start_x, i4_float start_y,
                    i4_float dest_x, i4_float dest_y,
                    i4_float *points, w16 &t_nodes); 

  // all objects in this area will receive damage falling off with distance
  void damage_range(g1_object_class *obj,
                    i4_float x, i4_float y, i4_float z, 
                    i4_float range, w16 damage, i4_float falloff=0);

//   g1_critical_graph_class *get_critical_graph() { return critical_graph; }

  i4_float terrain_height(i4_float x, i4_float y) const;
  i4_float map_height(i4_float x, i4_float y, i4_float z) const;

  void calc_terrain_normal(i4_float x, i4_float y, i4_3d_vector &normal);
  void calc_pitch_and_roll(i4_float x, i4_float y, i4_float z, i4_float &pitch, i4_float &roll);

  void calc_height_pitch_roll(i4_float x, i4_float y, i4_float z,
                              i4_float &height, i4_float &pitch, i4_float &roll);
                              

  sw32 make_object_list(g1_object_class **buffer, sw32 buf_size);  // returns total added
  sw32 make_selected_objects_list(w32 *buffer, sw32 buf_size);  // return total added (saved id's)

  void change_vert_height(sw32 x, sw32 y, w8 new_height);

  // how much light illuminates an object at this position
  void get_illumination_light(i4_float x, i4_float y, i4_float &r, i4_float &g, i4_float &b);

  void reload();

  // returns the total number of cells that can be seen
  int calc_visible(i4_transform_class &t,
                   i4_polygon_class *area_poly,
                   g1_visible_cell *buffer, w32 buf_elements,
                   i4_float xscale, i4_float yscale);           // how much to scale x & y members of vertexes


  i4_float min_terrain_height(w16 x, w16 y);

  ~g1_map_class();
} ;


#endif


