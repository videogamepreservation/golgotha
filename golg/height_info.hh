/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef _HEIGHT_INFO_HH_
#define _HEIGHT_INFO_HH_

class g1_height_info
{
public:    
  enum
  {
    BELOW_GROUND = 1,
    ROTATE_BITS  = (4|8)
  };

  g1_height_info();  

  i4_bool in_height_range(i4_float z)
  {
    return (z<ceiling_height && z>=floor_height);
  }


  w8 flags;
  w16 model_type;

  i4_float floor_height;
  i4_float ceiling_height;
};
    
#endif
