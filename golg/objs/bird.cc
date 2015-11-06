/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "objs/def_object.hh"
#include "lisp/li_init.hh"
#include "lisp/li_class.hh"
#include "li_objref.hh"
#include "map_man.hh"
#include "map.hh"
#include "math/pi.hh"
#include "objs/model_draw.hh"
#include "math/angle.hh"

static li_symbol_class_member flapping("flapping");

static li_int_class_member ticks_to_think("ticks_to_think");

static li_float_class_member xy_v("xy_v"), zv("zv"), 
  turn_speed("turn_speed"), 
  flap_theta("flap_theta");

// current destination of the bird is set by def_object automatically if the
// object has these variables when deploy_to is called
static li_float_class_member dest_x("dest_x"), dest_y("dest_y");     


static li_symbol *sym_yes=0, *sym_no=0;    // cache the pointer to the "yes" symbol


li_object *g1_draw_and_think(li_object *o, li_environment *env)
{
  g1_dynamic_object_class *me=g1_dynamic_object_class::get(li_car(o));


  g1_model_draw_parameters mp;
  mp.model=me->get_type()->model;
  g1_model_draw(me, mp, g1_draw_context);

  if (me->get_flag(g1_object_class::MAP_OCCUPIED))
  {
    ticks_to_think()=20;
    me->request_think();
  }

  return 0;
}


li_object *g1_bird_message(li_object *o, li_environment *env)
{
  g1_dynamic_object_class *me=g1_dynamic_object_class::get(li_car(o));
  return 0;
}

li_object *g1_bird_think(li_object *o, li_environment *env)
{
  g1_dynamic_object_class *me=g1_dynamic_object_class::get(li_car(o));
  
  me->grab_old();
  me->unoccupy_location();
  


  me->x += cos(me->theta) * xy_v();
  me->y += sin(me->theta) * xy_v();

  if (dest_x()>=0.0)
  {
    float desired_theta=atan2(me->y-dest_y(), me->x-dest_x());
    me->theta = i4_interpolate_angle(me->theta, desired_theta, 0.1);
  }
  else
    me->theta+=turn_speed();

  xy_v() *= 0.99;     // air friction
  
  if (flapping()==li_get_symbol("yes", sym_yes))
  {
    zv() += 0.001;
    if (zv()>0.03)
      zv()=0.03;
    xy_v() += 0.001;
  }
  else
  {
    zv() += -0.001;
    if (zv()<-0.05)
      zv()=-0.05;

    xy_v() += 0.001;
    if (xy_v()>0.2)
      xy_v()=0.2;
  }

  float z_vel=zv();
  me->pitch = z_vel * (i4_pi()/8) / 0.2;
  me->h += zv();

  float agh=me->height_above_ground();

  if (agh<1.5)
    flapping()=li_get_symbol("yes", sym_yes);
  else if (agh>2.0)
    flapping() = li_get_symbol("no", sym_no);


  if (flapping()==li_get_symbol("yes", sym_yes))
  {
    flap_theta()+=1.0;
    
    me->mini_objects[0].rotation.x=cos(flap_theta()) * i4_pi()/8 + i4_pi()/16;
    me->mini_objects[1].rotation.x=cos(flap_theta()) * -i4_pi()/8 + i4_pi()/16;
  }
  else
  {
    me->mini_objects[0].rotation.x *= 0.5;
    me->mini_objects[1].rotation.x *= 0.5;
  }
    

  if (me->occupy_location())
  {
    if (ticks_to_think())
    {
      ticks_to_think()--;
      me->request_think();
    }
  }

  return 0;
}

li_automatic_add_function(g1_bird_message, "bird_message");
li_automatic_add_function(g1_bird_think, "bird_think");
li_automatic_add_function(g1_draw_and_think, "draw_and_think");
