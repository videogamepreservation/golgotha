/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef MAP_VIEW_HH
#define MAP_VIEW_HH

#include "arch.hh"
#include "math/vector.hh"

class i4_parent_window_class;

enum
{
  G1_RADAR_CLICK_HOLDS_VIEW      =1,
  G1_RADAR_DRAW_ALL_PATHS        =2,
  G1_RADAR_DRAW_UNHIDDEN_PATHS   =4,
  G1_RADAR_USE_DIRTIES           =8,
  G1_RADAR_CLICK_SELECTS_PATH    =16,
  G1_RADAR_INTERLACED            =32,
  G1_RADAR_USE_ICONS             =64
};

i4_parent_window_class *g1_create_radar_view(int max_w, int max_h, int flags);

// this will re-render and area of the radar map
void g1_radar_refresh(int game_x1, int game_y1, int game_x2, int game_y2);

// tell the radar where the view is, so we can show screen position
void g1_radar_looking_at(float x1, float y1, float x2, float y2);

// when the map is changed by the editor/or a new level, call this
void g1_radar_recalculate_backgrounds();


// callled when radar should redraw (probably once 1-per second)
void g1_radar_update();
                         

void g1_unfog_radius(const i4_3d_vector &v, float r);

#endif


