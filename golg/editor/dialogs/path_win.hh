/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef PATH_WIN_HH
#define PATH_WIN_HH

#include "math/num_type.hh"
#include "gui/butbox.hh"
#include "window/window.hh"
#include "image/image32.hh"

class g1_path_tool_window_class : public i4_button_box_class
{
protected:
  i4_image_class *grade_icon[4], *size_icon[2];
  i4_const_str *grade_help[4], *size_help[2];
public:
  g1_path_tool_window_class(i4_graphical_style_class *style, i4_event_handler_class *send_to,
                            int buttons, i4_image_class **img, i4_const_str **help_names);
  
  char *name() { return "path_tool_win"; }
};

class g1_map_class;
class g1_critical_graph_class;
class g1_critical_map_maker_class;
class g1_astar_map_solver_class;
class g1_breadth_first_graph_solver_class;

class g1_path_window_class : public i4_parent_window_class
{
protected:
  i4_image_class *start_icon, *dest_icon, *crit_icon;
  int last_x, last_y;
  int map_changed;

  i4_float point[1024];
  w16 points;
  
public:
  enum { CELL_SIZE=3 };

  class coord 
  { 
  public:
    w16 x,y; 

    coord() : x(0), y(0) {}
  } start, dest;      // path to solve

  int mode;                  // current lay down mode from tool window
  
  w16 grade;
  w8 size;
  w8 tofrom;
  g1_map_class *map;
  g1_critical_graph_class *critical_graph;
  
  g1_critical_map_maker_class *maker;
  g1_astar_map_solver_class *solvemap;
  g1_breadth_first_graph_solver_class *solvegraph;
  
  i4_image32 *bitmap;

  g1_path_window_class(g1_map_class *map, i4_image_class **icons);
  ~g1_path_window_class();

  w32 critical_color(w16 x, w16 y);
  
  void solve();
  void changed() { map_changed = 1; request_redraw(); }

  void draw_to_bitmap();
  void parent_draw(i4_draw_context_class &context);
  virtual void receive_event(i4_event *ev);

  
  char *name() { return "path_win"; }
};

#endif
