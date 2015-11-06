/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_F_TABLES_HH
#define G1_F_TABLES_HH



#include "math/point.hh"
#include "math/num_type.hh"

extern float g1_table_0_31_to_n1_1[32];   // maps 5 bits of integer into float range -1 to 1  
extern float g1_table_0_31_to_0_1[32];    // maps 5 bits of integer into float range 0 to 1
extern float g1_table_0_255_to_0_1[256];

extern float g1_shadow_sub_0_63_to_0_1_red_or_green[64];  // used in map_vert.hh for cloud shadows
extern float g1_shadow_sub_0_63_to_0_1_blue[64];



inline w16 g1_normal_to_16(i4_3d_vector v)
{
  w32 x=i4_f_to_i((v.x+1.0)*15.5),
    y=i4_f_to_i((v.y+1.0)*15.5),
    z=i4_f_to_i((v.z+1.0)*15.5);

  return (x<<10)|(y<<5)|z;    
}


#endif
