/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "light.hh"
#include "saver_id.hh"
#include "saver.hh"
#include "map.hh"

g1_light_info g1_lights;


void g1_light_info::recalc_shadow_intensity()
{
  for (int i=0; i<256; i++)
    shadow_intensity[i]=1.0 - i4_float(i)/256.0;
}


void g1_light_info::set_ambient_intensity(float v)
{
  ambient_intensity=v;
  recalc_shadow_intensity();

}

void g1_light_info::set_directional_intensity(float v)
{
  directional_intensity=v;
  recalc_shadow_intensity();
}




void g1_light_info::save(i4_saver_class *fp)
{
  fp->mark_section("Map Lights V2");
  fp->write_format("fffff", 
                   &directional_intensity, 
                   &direction.x, &direction.y, &direction.z,
                   &ambient_intensity);
}

i4_bool g1_light_info::load(i4_loader_class *fp)
{
  if (fp && fp->goto_section("Map Lights V2"))
  {
    fp->read_format("fffff", 
                    &directional_intensity, 
                    &direction.x, &direction.y, &direction.z,
                    &ambient_intensity);

    recalc_shadow_intensity();
    return i4_T;
  }

  return i4_F;
}

void g1_light_info::defaults()
{
  directional_intensity=0.3;
  direction=i4_3d_vector(1, 0.0, -0.1);
  direction.normalize();
  ambient_intensity=0.05;
}

g1_light_info::g1_light_info()
{
  defaults();
}
