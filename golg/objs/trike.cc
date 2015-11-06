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
#include "input.hh"
#include "math/pi.hh"
#include "math/trig.hh"
#include "math/angle.hh"
#include "objs/bullet.hh"
#include "resources.hh"
#include "saver.hh"
#include "map_cell.hh"
#include "map.hh"
#include "math/angle.hh"
#include "objs/vehic_sounds.hh"
#include "sound/sfx_id.hh"
#include "objs/trike.hh"
#include "objs/shrapnel.hh"
#include "math/random.hh"
#include "object_definer.hh"


static g1_model_ref model_ref("trike_body"),
  shadow_ref("trike_shadow"),
  wheel_ref("trike_wheels"),
  lod_ref("trike_lod");

static g1_object_type shrapnel_type;
static i4_3d_vector wheel_attach, wheel_offset;

void g1_trike_init()
{
  shrapnel_type = g1_get_object_type("shrapnel");

  wheel_attach.set(0.1,0,0.03);
  model_ref()->get_mount_point("Wheel", wheel_attach);

  wheel_offset.set(0.1,0,0.03);
  wheel_ref()->get_mount_point("Wheel", wheel_offset);
  wheel_offset.reverse();
}

g1_object_definer<g1_trike_class>
g1_trike_def("trike",
             g1_object_definition_class::TO_MAP_PIECE |
             g1_object_definition_class::MOVABLE |
             g1_object_definition_class::EDITOR_SELECTABLE, 
             g1_trike_init);


//think and draw functions

g1_trike_class::g1_trike_class(g1_object_type id, 
                                     g1_loader_class *fp)
  : g1_map_piece_class(id, fp)
{  
  radar_type=G1_RADAR_VEHICLE;
  set_flag(BLOCKING      |
           TARGETABLE    |
           GROUND        | 
           HIT_GROUND    |
           DANGEROUS, 1);

  draw_params.setup(model_ref.id(), 0, lod_ref.id());
        
  allocate_mini_objects(2,"Trike Mini-Objects");

  wheels = &mini_objects[0];
  wheels->defmodeltype = wheel_ref.id();
  wheels->position(wheel_attach);
  wheels->offset = wheel_offset;

  w16 ver,data_size;
  if (fp)
  {
    fp->get_version(ver,data_size);
    if (ver==DATA_VERSION)
    {
      wheels->rotation.x = fp->read_float();
      wheels->rotation.y = fp->read_float();
      wheels->rotation.z = fp->read_float();
  
      wheels->lrotation.x = fp->read_float();
      wheels->lrotation.y = fp->read_float();
      wheels->lrotation.z = fp->read_float();  
    }
    else
    {
      fp->seek(fp->tell() + data_size);
    }

    fp->end_version(I4_LF);
  }
  else
  {
    wheels->rotation.x = 0;
    wheels->rotation.y = 0;
    wheels->rotation.z = 0;  
    wheels->lrotation = wheels->rotation;
  }  

  init_rumble_sound(G1_RUMBLE_GROUND);
}
   
void g1_trike_class::save(g1_saver_class *fp)
{
  g1_map_piece_class::save(fp);

  fp->start_version(DATA_VERSION);

  fp->write_float(wheels->rotation.x);
  fp->write_float(wheels->rotation.y);
  fp->write_float(wheels->rotation.z);

  fp->write_float(wheels->lrotation.x);
  fp->write_float(wheels->lrotation.y);
  fp->write_float(wheels->lrotation.z);

  fp->end_version();
}

void g1_trike_class::think()
{
  g1_map_piece_class::think();

  if (!alive())
    return;

  if (attack_target.valid())
  {    
    request_think();

    dest_x = attack_target->x;
    dest_y = attack_target->y;

    i4_float dist, dtheta, dx, dy;
    suggest_move(dist, dtheta, dx, dy, 0);
    move(dx,dy);

    const i4_float KILL_RADIUS=0.5;

    //if we've run into the guy
    if (x > dest_x-KILL_RADIUS  &&  y > dest_y-KILL_RADIUS  &&
        x < dest_x+KILL_RADIUS  &&  y < dest_y+KILL_RADIUS)
    {
      int damage=g1_trike_def.get_damage_map()->get_damage_for(attack_target.get()->id);

      //hurt the dude
      attack_target->damage(this,damage,i4_3d_vector(dx,dy,0));
      
      //throw some gibs
      g1_shrapnel_class *shrapnel = NULL;
      shrapnel = (g1_shrapnel_class *)g1_create_object(shrapnel_type);
      if (shrapnel)
      {
        i4_float rx,ry,rh;
        rx = (i4_rand()&0xFFFF)/((i4_float)0xFFFF) * 0.2;
        ry = (i4_rand()&0xFFFF)/((i4_float)0xFFFF) * 0.2;
        rh = (i4_rand()&0xFFFF)/((i4_float)0xFFFF) * 0.2;
        shrapnel->setup(x+rx,y+ry,h + rh,6,i4_T);
      }
      unoccupy_location();
      request_remove();
    }      
  }
}

