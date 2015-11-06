/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_LIGHT_HH
#define G1_LIGHT_HH

#include "math/vector.hh"

class i4_saver_class;
class i4_loader_class;

struct g1_light_info
{
  float directional_intensity;           // 0..1
  i4_3d_vector direction;

  float ambient_intensity;               // 0..1
  float shadow_intensity[256];

  void recalc_shadow_intensity();
  void set_ambient_intensity(float v);
  void set_directional_intensity(float v);

  void save(i4_saver_class *fp);
  i4_bool load(i4_loader_class *fp);
  void defaults();
  g1_light_info();
} ;

extern g1_light_info g1_lights;




#endif

