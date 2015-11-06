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
#include "objs/rocktank.hh"
#include "input.hh"
#include "math/pi.hh"
#include "math/trig.hh"
#include "math/angle.hh"
#include "g1_rand.hh"
#include "resources.hh"
#include "saver.hh"
#include "map_cell.hh"
#include "map.hh"
#include "objs/vehic_sounds.hh"
#include "sound/sfx_id.hh"
#include "object_definer.hh"
#include "objs/fire.hh"


S1_SFX(rumble, "rumble/missile_truck_lp.wav",   S1_3D, 1);
S1_SFX(raising, "misc/missile_truck_raise.wav", S1_3D, 5);
S1_SFX(lowering, "misc/missile_truck_lower.wav", S1_3D, 5);


enum { DATA_VERSION=1 };

enum eRackState
{
  LOWERED,
  RAISE,
  RAISING,
  RAISED,
  LOWER,
  LOWERING,
};

static g1_model_ref model_ref("rocket_truck"),
  shadow_ref("rocket_truck_shadow"),
  rack_ref("rocket_truck_top"),
  base_lod("rocket_truck_lod"),
  top_lod("rocket_truck_top_lod");


static i4_3d_vector rack_attach, rack_offset;

static void g1_rocket_tank_init()
{
  rack_attach.set(0,0,0);
  model_ref()->get_mount_point("Rack", rack_attach);
  rack_offset.set(0,0,0);
  rack_ref()->get_mount_point("Rack", rack_offset);
  rack_offset.reverse();
}

g1_object_definer<g1_rocket_tank_class>
g1_rocket_tank_def("rocket_tank",
                   g1_object_definition_class::TO_MAP_PIECE |
                   g1_object_definition_class::MOVABLE |
                   g1_object_definition_class::EDITOR_SELECTABLE,
                   g1_rocket_tank_init);

static char *chunk_list[3]={"chunk_missiletruck_body", "chunk_missiletruck_launcher",
                             "chunk_missiletruck_tread"};


int g1_rocket_tank_class::get_chunk_names(char **&list) { list=chunk_list; return 3; }

g1_rocket_tank_class::g1_rocket_tank_class(g1_object_type id,
                                           g1_loader_class *fp)
  : g1_map_piece_class(id, fp)
{  
  draw_params.setup(model_ref.id(), shadow_ref.id(), base_lod.id());

  allocate_mini_objects(2,"Rocket Tank Mini-objects");

  missile_rack = &mini_objects[0];
  missile_rack->defmodeltype = rack_ref.id();
  missile_rack->lod_model = top_lod.id();


  missile_rack->position(rack_attach);
  missile_rack->offset = rack_offset;
  
  w16 ver,data_size;
  if (fp)
  {
    fp->get_version(ver,data_size);
    switch (ver)
    {
      case DATA_VERSION:
        fp->read_8();
        fp->read_format("fff",
                         &missile_rack->rotation.x,
                         &missile_rack->rotation.y,
                         &missile_rack->rotation.z);

        missile_rack->grab_old();
        break;
      case 0:
        fp->read_8();
        fp->read_8();
        fp->read_8();
        fp->read_8();      
        
        fp->read_16();      
        
        missile_rack->rotation.x = fp->read_float();
        missile_rack->rotation.y = fp->read_float();
        missile_rack->rotation.z = fp->read_float();
        
        missile_rack->lrotation.x = fp->read_float();
        missile_rack->lrotation.y = fp->read_float();
        missile_rack->lrotation.z = fp->read_float();
        break;
      default:
        fp->seek(fp->tell() + data_size);
        missile_rack->rotation.x = 0;
        missile_rack->rotation.y = 0;
        missile_rack->rotation.z = 0;
        missile_rack->lrotation = missile_rack->rotation;
        fire_delay = 15;
        break;
    }
    fp->end_version(I4_LF);
  }
  else
  {
    missile_rack->rotation.x = 0;
    missile_rack->rotation.y = 0;
    missile_rack->rotation.z = 0;
    missile_rack->lrotation = missile_rack->rotation;
    fire_delay = 15;
  }  

  init_rumble_sound(G1_RUMBLE_GROUND);

  guard_angle = 0;
  guard_pitch = 0;
  guard_time = 0;

  radar_type=G1_RADAR_VEHICLE;
  set_flag(BLOCKING      |
           SELECTABLE    |
           TARGETABLE    |
           GROUND        | 
           HIT_AERIAL    |
           HIT_GROUND    |
           DANGEROUS, 1);
}
   
void g1_rocket_tank_class::save(g1_saver_class *fp)
{
  g1_map_piece_class::save(fp);

  fp->start_version(DATA_VERSION);

  fp->write_8(0);
  fp->write_format("fff",
                   &missile_rack->rotation.x,
                   &missile_rack->rotation.y,
                   &missile_rack->rotation.z);
  
  fp->end_version();
}


void g1_rocket_tank_class::fire()
{
  g1_object_class *target=attack_target.get();
  if (target)
  {
    i4_float bx,by,bz;

    i4_3d_vector pos, dir;

    i4_transform_class btrans,tmp1;
      
    btrans.translate(x,y,h);

    tmp1.rotate_x(groundroll);
    btrans.multiply(tmp1);

    tmp1.rotate_y(groundpitch);
    btrans.multiply(tmp1);
  
    tmp1.rotate_z(theta);
    btrans.multiply(tmp1);

    tmp1.translate(missile_rack->x, missile_rack->y, missile_rack->h);
    btrans.multiply(tmp1);

    tmp1.rotate_z(missile_rack->rotation.z);
    btrans.multiply(tmp1);

    tmp1.rotate_y(missile_rack->rotation.y);
    btrans.multiply(tmp1);  

    btrans.transform(i4_3d_vector(0.1, 0, 0), pos);
    btrans.transform(i4_3d_vector(1.0, 0, 0), dir);
    dir-=pos;
  
    g1_fire(defaults->fire_type, this, target, pos, dir);
    fire_delay = defaults->fire_delay;
  }
}

void g1_rocket_tank_class::think()
{  
  g1_map_piece_class::think();

  if (!alive())
    return;

  //aim the missile rack
  if (attack_target.valid()) 
  {
    request_think();

    i4_float dx,dy,dangle;
    
    //this will obviously only be true if attack_target.ref != NULL    
    dx = ((x+missile_rack->x) - attack_target->x);
    dy = ((y+missile_rack->y) - attack_target->y);

    //aim the vehicle    
    guard_angle = i4_atan2(dy,dx) + i4_pi() - theta;
    i4_normalize_angle(guard_angle);    

    i4_float dist = dx*dx+dy*dy;
    if (dist>25.0)
      guard_pitch = -i4_pi()/8;
    else if (dist>9.0)
      guard_pitch = -(dist - 9.0)*i4_pi()/8/(25.0-9.0);
    else
      guard_pitch = 0;
    i4_normalize_angle(guard_pitch);

    i4_bool aimed = i4_F;
    
    dangle = i4_rotate_to(missile_rack->rotation.z, guard_angle, 0.1);
    aimed = (dangle<0.1 && dangle>-0.1);
    dangle = i4_rotate_to(missile_rack->rotation.y, guard_pitch, 0.1);
    aimed &= (dangle<0.1 && dangle>-0.1);

    if (aimed && !fire_delay)
      fire();

    guard_time = 40;
  }  
  else
  {
    guard_pitch = 0;
    guard_angle = 0;

    request_think();            // move this to draw function

    i4_rotate_to(missile_rack->rotation.z, guard_angle, 0.1);
    i4_rotate_to(missile_rack->rotation.y, guard_pitch, 0.1);
  }
}

