/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "object_definer.hh"
#include "lisp/li_init.hh"
#include "lisp/li_class.hh"
#include "li_objref.hh"
#include "map_man.hh"
#include "map.hh"
#include "math/pi.hh"
#include "objs/model_draw.hh"
#include "math/angle.hh"
#include "li_objref.hh"
#include "g1_render.hh"
#include "lisp/li_vect.hh"
#include "tick_count.hh"

static li_int_class_member ticks_to_think("ticks_to_think"),
                           damage_per_tick("damage_per_tick"); 

static li_object_class_member person_being_damaged("person_being_damaged"),
                              person_giving_damage("person_giving_damage"),
                              attach_pos("attach_pos"); 

static li_symbol_class_member smoke_type("smoke_type");

static li_symbol_ref acid("acid"), napalm("napalm");
static r1_texture_ref acid_smoke("acid");

class g1_damager_class : public g1_object_class
{
public:
  
  g1_damager_class(g1_object_type id, g1_loader_class *fp)
    : g1_object_class(id, fp)
  {
    draw_params.setup("trigger");   
  }
  
  void draw(g1_draw_context_class *context)
  {
    if (smoke_type()==acid.get())
    {
      float size=0.4;
      float sw=size*0.25, sh=size;
      i4_3d_vector pos(i4_interpolate(lx,x, g1_render.frame_ratio),
                       i4_interpolate(ly,y, g1_render.frame_ratio),
                       i4_interpolate(lh,h, g1_render.frame_ratio)+sh/2.0), t_pos;
      context->transform->transform(pos, t_pos);  
      g1_render.render_sprite(t_pos, acid_smoke.get(), sw, sh,
                               (g1_tick_counter%4)*0.25, 0,
                               (g1_tick_counter%4)*0.25+0.25, 1);
    }
   
  }
  
  void think()
  {
    g1_object_class *hurt_this_guy = li_g1_ref::get(person_being_damaged(),0)->value();
    g1_object_class *this_guys_hurting_him = li_g1_ref::get(person_giving_damage(),0)->value();
    if (!this_guys_hurting_him)
      this_guys_hurting_him=this;

    if (ticks_to_think() && hurt_this_guy && hurt_this_guy->valid())
    {
      unoccupy_location();
      grab_old();
      
      i4_3d_vector ap=li_vect::get(attach_pos(),0)->value();
      x=hurt_this_guy->x+ap.x;
      y=hurt_this_guy->y+ap.y;
      h=hurt_this_guy->h+ap.z;
      
      occupy_location();
      
      ticks_to_think()--;

      hurt_this_guy->damage(this_guys_hurting_him, damage_per_tick(), i4_3d_vector(0,0,1));
      if (this_guys_hurting_him)
        this_guys_hurting_him->notify_damage(hurt_this_guy, damage_per_tick());
        
      request_think();
    }
    else
    {
      unoccupy_location();
      request_remove();
    }
  }
};


g1_object_definer<g1_damager_class>
g1_damager_def("damager", g1_object_definition_class::EDITOR_SELECTABLE);


g1_object_class *g1_create_damager_object(const i4_3d_vector &pos,
                                          int _damage_per_tick, int _ticks,
                                          g1_object_class *_person_giving_damage,
                                          g1_object_class *_person_being_damaged,
                                          li_symbol *_smoke_type)
{
  g1_damager_class *d=(g1_damager_class *)g1_create_object(g1_damager_def.type);
  if (!d) return 0;

  
  d->x=d->lx=pos.x;
  d->y=d->ly=pos.y;
  d->h=d->lh=pos.z;
  
  d->vars->get(attach_pos)=new li_vect(i4_3d_vector(pos.x-_person_being_damaged->x,
                                                    pos.y-_person_being_damaged->y,
                                                    pos.z-_person_being_damaged->h));
  
  d->vars->get(damage_per_tick)=_damage_per_tick;
  d->vars->get(person_being_damaged)=new li_g1_ref(_person_being_damaged->global_id);
  if (_person_giving_damage)
    d->vars->get(person_giving_damage)=new li_g1_ref(_person_giving_damage->global_id);
  else
    d->vars->get(person_giving_damage)=new li_g1_ref(g1_global_id.invalid_id());

  d->vars->get(smoke_type)=_smoke_type;
  d->vars->get(ticks_to_think)=_ticks;
  d->occupy_location();
  d->request_think();
  return d;
}









