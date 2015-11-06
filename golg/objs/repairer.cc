/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "sound_man.hh"
#include "objs/model_id.hh"
#include "objs/model_draw.hh"
#include "objs/repairer.hh"
#include "input.hh"
#include "math/pi.hh"
#include "math/angle.hh"
#include "math/trig.hh"
#include "math/random.hh"
#include "resources.hh"
#include "saver.hh"
#include "map_cell.hh"
#include "map.hh"
#include "map_man.hh"
#include "object_definer.hh"

enum {DATA_VERSION=1};

g1_object_definer<g1_repairer_class>
g1_repairer_def("repairer", g1_object_definition_class::EDITOR_SELECTABLE);

g1_repairer_class::g1_repairer_class(g1_object_type id,
                                     g1_loader_class *fp)
  : g1_map_piece_class(id,fp)
{  
  draw_params.setup("repair_base");
  defaults = g1_repairer_def.defaults;
  
  allocate_mini_objects(3,"repair_objects");

  turret = &mini_objects[0];
  turret->x = turret->lx = 0;
  turret->y = turret->ly = 0;
  turret->h = turret->lh = 0;
  turret->rotation.set(0,0,0);
  turret->defmodeltype = g1_model_list_man.find_handle("repair_turret");
  turret->grab_old();
  
  boom = &mini_objects[1];
  boom->x = turret->lx = turret->x;
  boom->y = turret->ly = turret->y;
  boom->h = turret->lh = turret->h;
  boom->rotation.set(0,0,0);
  boom->defmodeltype = g1_model_list_man.find_handle("repair_boom");
  boom->grab_old();
  
  tip = &mini_objects[2];
  tip->x = tip->lx = g1_resources.repairer_tip_attach.x;
  tip->y = tip->ly = g1_resources.repairer_tip_attach.y;
  tip->h = tip->lh = g1_resources.repairer_tip_attach.z;
  tip->rotation.set(0,-i4_pi()/2.0,0);
  tip->defmodeltype = g1_model_list_man.find_handle("repair_tip");
  tip->grab_old();
  
  w16 ver,data_size;
  if (fp)
    fp->get_version(ver,data_size);
  else
    ver =0;
  switch (ver)
  {
    case DATA_VERSION:
      break;
    default:
      if (fp) fp->seek(fp->tell() + data_size);
      health = defaults->health;
      break;
  }

  if (fp)
    fp->end_version(I4_LF);

  length = g1_resources.repairer_boom_offset;

  set_flag(BLOCKING      |
           SHADOWED,
           1);
}

void g1_repairer_class::save(g1_saver_class *fp)
{
  g1_map_piece_class::save(fp);
  
  fp->start_version(DATA_VERSION);

  fp->end_version();
}

i4_bool g1_repairer_class::can_attack(g1_object_class *who) const
{
  if (who->player_num == player_num)
  {
    g1_map_piece_class *mp = g1_map_piece_class::cast(who);
    return (mp && mp->get_flag(GROUND) && mp->health>0 && mp->health<mp->defaults->health);
  }
  return i4_F;
}

void g1_repairer_class::think()
{  
  find_target();

  if (health < defaults->health)
    health++;

  if (fire_delay>0)
    fire_delay--;

  pitch = 0;
  roll  = 0;
  h = terrain_height;

  if (attack_target.valid()) 
  {
    i4_float dx,dy;

    //this will obviously only be true if attack_target.ref != NULL    
    dx = (attack_target->x - x);
    dy = (attack_target->y - y);

    //aim the turet
    
    repair_angle = i4_atan2(dy,dx);
    i4_normalize_angle(repair_angle);
    repair_length = sqrt(dx*dx + dy*dy);
  }
  else
  {
    repair_angle = 0;
    repair_length = 1.5;
  }

  int aimed;
  i4_float dangle;
  
  aimed = (i4_rotate_to(turret->rotation.z,repair_angle,defaults->turn_speed)==0.0);
  if (length<repair_length) 
  {
    length += speed;
    if (length>repair_length) length=repair_length; 
  }
  else
  {
    length -= speed;
    if (length<repair_length) length=repair_length; 
  }
  i4_float 
    cs = cos(turret->rotation.z),
    sn = sin(turret->rotation.z),
    ex = length - g1_resources.repairer_boom_offset;
  boom->rotation.z = turret->rotation.z;
  tip->rotation.z = turret->rotation.z;
  boom->x = cs*ex;
  boom->y = sn*ex;
  ex += g1_resources.repairer_tip_attach.x;
  tip->x = cs*ex;
  tip->y = sn*ex;

  if (attack_target.valid() && aimed)
  {
    if (i4_rotate_to(tip->rotation.y,-i4_pi()/6.0,0.1)==0.0 && fire_delay==0)
    {
      g1_map_piece_class *mp = g1_map_piece_class::cast(attack_target.get());
      
      if (mp->health<mp->defaults->health && mp->health>0)
      {
        fire_delay = defaults->fire_delay;
        mp->health+=get_type()->get_damage_map()->get_damage_for(mp->id);
        
        if (mp->health>mp->defaults->health)
          mp->health = mp->defaults->health;
      }
      else
        attack_target = 0;
    }
  }
  else
    // return to stopped position
    i4_rotate_to(tip->rotation.y,-i4_pi()/2.0,0.1);

  // don't rethink if i'm not healing or moving
  if (attack_target.valid() ||
      tip->rotation.y != tip->lrotation.y ||
      tip->rotation.z != tip->lrotation.z ||
      tip->x != tip->lx ||
      tip->y != tip->ly)
    request_think();
}

void g1_repairer_class::draw(g1_draw_context_class *context)
{
  g1_model_draw(this, draw_params, context);
}
