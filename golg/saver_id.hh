/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_SAVER_ID_HH
#define G1_SAVER_ID_HH


enum {   
  G1_SECTION_MAP_DIMENSIONS_V1,

  G1_SECTION_TILE_MATCHUP_V1,

  OLD_G1_SECTION_CELL_V1,
  G1_SECTION_OBJECTS_V1,

  G1_SECTION_OBJECT_BASE_V1,
  G1_SECTION_PATHS_V1,

  G1_SECTION_GRAPH_NODES_V1,
  OLD_G1_SECTION_GRAPH_EDGES_V1,
  G1_SECTION_GRAPH_WEIRDS_V1,

  OLD_G1_SECTION_CELL_V2,
  G1_SECTION_GRAPH_EDGES_V2,

  OLD_G1_SECTION_CELL_V3,

  G1_SECTION_TICK,

  G1_SECTION_MOVIE,

  OLD_G1_SECTION_CELL_V4,    

  OLD_G1_SECTION_MAP_VERT_V1,

  OLD_G1_SECTION_MAP_LIGHTS_V1,

  OLD_G1_SECTION_MAP_VERT_V2,

  G1_SECTION_MODEL_QUADS,
  G1_SECTION_MODEL_TEXTURE_NAMES,
  G1_SECTION_MODEL_VERT_ANIMATION,
  
  G1_SECTION_PLAYER_INFO,

  OLD_G1_SECTION_MAP_VERT_V3,     // added flags

  G1_SECTION_SKY_V1,             // sky model and info used for level

  OLD_G1_SECTION_CELL_V5,

  OLD_G1_SECTION_CELL_V6,

  G1_SECTION_OBJECT_TYPES_V1, // saves the name of each object type so dynamic id's are matched
  G1_SECTION_MAP_SECTIONS_V1,

  OLD_G1_SECTION_MAP_VERT_V4,      // added selected prefix

  G1_SECTION_CRITICAL_POINTS_V1,
  G1_SECTION_CRITICAL_GRAPH_V1,
  G1_SECTION_CRITICAL_MAP_V1,
  
  

  G1_SECTION_OBJECT_LIST = 0x8000

} ;


#endif
