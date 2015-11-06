/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_LIGHT_OBJECT_HH
#define G1_LIGHT_OBJECT_HH

#include "g1_object.hh"
#include "g1_render.hh"
#include "objs/model_draw.hh"

extern g1_object_type g1_lightbulb_type;


class g1_light_object_class : public g1_object_class
{
public:
  static g1_light_object_class *cast(g1_object_class *obj)
  {
    if (!obj || obj->id != g1_lightbulb_type)
    { 
#ifdef I4_CAST_WARN
      i4_warning("bad cast to lightbulb_obj!\n"); 
#endif
      return 0; 
    }
    
    return (g1_light_object_class *)obj;
  }

  i4_float r,g,b;                      // 0..1 if colored lighting supported
  i4_float white;                      // 0..1 if only white light supported

  // light atenuation formula = min(1/(c1 + c2*d + c3*d*d),1)
  i4_float c1,c2,c3;                   // 0..1 light antenuation values

  g1_light_object_class *next;   // next point source light in list

  sw32 change_radius;
  w32 *add_intensities;


  enum { DATA_VERSION=0 };

  g1_light_object_class(g1_object_type id, g1_loader_class *fp);

  void save(g1_saver_class *fp);

  virtual i4_bool occupy_location();
  virtual void unoccupy_location();
  
  void setup(float _x, float _y, float _h, 
             float _r, float _g, float _b, float _white,
             float min_light_contribute=0.05, 
             float linear_contribute=0.25, 
             float geometric_contribute=0.5);

  void move(float new_x, float new_y, float new_h);
            
  virtual void draw(g1_draw_context_class *context);
  virtual void think();
  ~g1_light_object_class();
};

extern g1_object_type g1_lightbulb_type;

#endif
