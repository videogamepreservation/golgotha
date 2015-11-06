/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_LIMITS_HH
#define G1_LIMITS_HH
#include "math/num_type.hh"

enum { G1_MAX_OBJECTS_BITS=12 };
enum { G1_MAX_OBJECTS=1<<G1_MAX_OBJECTS_BITS };     // in a level at any time
enum { G1_MAX_MODELS=300 };
enum { G1_MAX_OBJECT_TYPES=400 };
enum { G1_MAX_TILES=500 };   // maximum building & ground "types" defined for game
enum { G1_MAX_SPRITES=300 } ;
enum { G1_MAX_PLAYERS=5 };      // maximum number of players, including the no_player team
// teams a player can be on, for single player game, the player is always on the ALLY team
enum g1_team_type { G1_ALLY, G1_ENEMY, G1_MAX_TEAMS }; 
enum { G1_MAX_SOUNDS=20 };
enum { G1_MAX_WAYPOINTS=200 };  // maximum number of way points in a path
enum { G1_MAX_CELL_FUNCTIONS=30 }; // maximum number of functions that can be assigned to cells
enum { G1_MAX_UNIT_PADS_PER_PLAYER=8 };
enum { G1_MAX_POLY_LIST_POLYS=256}; //max number of polygons in the poly list
enum { G1_MAX_TEXTURES=1500 };      // total game textures total
enum { G1_MAX_ANIMATIONS=50 };     // total animating textures total
enum { G1_CHROMA_COLOR=(254<<16)|(2<<8)|166 } ;
enum { G1_GRADE_LEVELS=4 };       // number of grade levels for vehicles
enum { G1_CRITICALS_PER_CELL=4 }; // number of nearest critical point references in a cell
enum { G1_MAX_MAPS=1 };            // maximum maps per level, will require some change for more
enum { G1_MAX_TAKEOVERPADS=100 };  // maximum number of takeover pads in a map
enum { G1_BUILD_PAD_W=6,           // size of construction pad
       G1_BUILD_PAD_H=6 };

enum { G1_MIN_MAP_DIMENSION=10,
       G1_MAX_MAP_DIMENSION=150 };

#endif
