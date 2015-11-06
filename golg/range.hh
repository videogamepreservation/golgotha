/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_RANGE_HH
#define G1_RANGE_HH

enum g1_fire_range_type
    { G1_FIRE_RANGE_CALCULATE=0,
      G1_FIRE_RANGE_1=1,
      G1_FIRE_RANGE_2=2,
      G1_FIRE_RANGE_3=3,
      G1_FIRE_RANGE_4=4,
      G1_FIRE_RANGE_5=5,
      G1_FIRE_RANGE_6=6,
      G1_FIRE_RANGE_7=7,
      G1_FIRE_RANGE_8=8 };

enum { G1_FIRST_FIRE_RANGE=G1_FIRE_RANGE_1,
       G1_LAST_FIRE_RANGE=G1_FIRE_RANGE_8 };

#endif
