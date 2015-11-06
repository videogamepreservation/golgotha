/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_CHUNK_EXPLODE_HH
#define G1_CHUNK_EXPLODE_HH

#include "g1_object.hh"


class g1_chunk_explosion_class : public g1_object_class
{
public:
  g1_chunk_explosion_class(g1_object_type id, g1_loader_class *fp);

  i4_3d_vector dir;
  i4_3d_vector avel;
  
  int ticks_left;  
  void setup(const i4_3d_vector &pos,
             const i4_3d_vector &rotations,
             g1_quad_object_class *model,
             const i4_3d_vector &dir,
             int ticks_till_explode);  
  void draw(g1_draw_context_class *context);  
  void think();  
}; 


#endif
