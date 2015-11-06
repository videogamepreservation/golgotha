/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "sound_man.hh"
#include "input.hh"
#include "math/pi.hh"
#include "math/trig.hh"
#include "math/angle.hh"
#include "resources.hh"
#include "saver.hh"
#include "map_cell.hh"
#include "map.hh"
#include "objs/vehic_sounds.hh"
#include "sound/sfx_id.hh"
#include "object_definer.hh"

#include "objs/model_id.hh"
#include "objs/model_draw.hh"
#include "objs/fire.hh"
#include "objs/tank_buster.hh"
#include "lisp/lisp.hh"

#include "image_man.hh"
static g1_team_icon_ref radar_im("bitmaps/radar/tank_buster.tga");


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


static g1_model_ref model_ref("buster_body"), shadow_ref("buster_body_shadow"), 
  missile_ref("buster_rocket"), bot_lod("buster_body_lod");

static g1_object_type buster_rocket_type;
static i4_3d_vector missile_attach, missile_offset;

void g1_tank_buster_init()
{
  buster_rocket_type = g1_get_object_type("buster_rocket");

  missile_attach.set(-0.1984,0,0.1468);
  model_ref.get()->get_mount_point("Missile", missile_attach);

  missile_offset.set(0,0,0);
  missile_ref.get()->get_mount_point("Offset", missile_offset);
  missile_offset.reverse();
}

g1_object_definer<g1_tank_buster_class>
g1_tank_buster_def("tank_buster",
                   g1_object_definition_class::TO_MAP_PIECE |
                   g1_object_definition_class::EDITOR_SELECTABLE |
                   g1_object_definition_class::MOVABLE,
                   g1_tank_buster_init);

g1_tank_buster_class::g1_tank_buster_class(g1_object_type id,
                                           g1_loader_class *fp)
  : g1_map_piece_class(id, fp)
{  
  draw_params.setup(model_ref.id(),0,bot_lod.id());

  allocate_mini_objects(2,"Tank Buster Mini-objects");

  missile = &mini_objects[0];
  missile->defmodeltype = missile_ref.id();
  missile->position(missile_attach);
  missile->offset = missile_offset;
  
  i4_bool use_defaults = i4_T;

  w16 ver,data_size;
  if (fp)
  {
    fp->get_version(ver,data_size);
    if (ver==DATA_VERSION)
    {
      use_defaults = i4_F;
      fp->read_format("1fff",
                      &rack_state, 
                      &missile->rotation.x,
                      &missile->rotation.y,
                      &missile->rotation.z);
      
      missile->lrotation = missile->rotation;
    }
    else
      fp->seek(fp->tell() + data_size);

    fp->end_version(I4_LF);
  }
  
  if (use_defaults)
  {
    rack_state = LOWERED;
    missile->rotation  = i4_3d_vector(0,0,0);
    missile->lrotation = missile->rotation;
    fire_delay = 0; //no delay time
    explode_delay = -1;
  }



  init_rumble_sound(G1_RUMBLE_GROUND);
  radar_type=G1_RADAR_VEHICLE;
  radar_image=&radar_im;
  set_flag(BLOCKING      |
           TARGETABLE    |
           GROUND        | 
           HIT_AERIAL    |
           HIT_GROUND    |
           DANGEROUS, 1);
}
   
void g1_tank_buster_class::save(g1_saver_class *fp)
{
  g1_map_piece_class::save(fp);

  fp->start_version(DATA_VERSION);

  fp->write_format("1fff",
                   &rack_state, 
                   &missile->rotation.x,
                   &missile->rotation.y,
                   &missile->rotation.z);
  
  fp->end_version();
}

void g1_tank_buster_class::fire()
{
  g1_object_class *target=attack_target.get();

  if (target)
  {

    i4_transform_class o2w;    
    calc_world_transform(1,&o2w);

    i4_3d_vector local_fire_point;    
    i4_3d_vector world_fire_point, world_fire_dir;
    o2w.transform(i4_3d_vector(0.0, 0, 0.0), world_fire_point);
    o2w.transform(i4_3d_vector(1,0,0), world_fire_dir);
    world_fire_dir-=world_fire_point;
    
    g1_fire(defaults->fire_type, this, attack_target.get(),
            world_fire_point, world_fire_dir);    

  
    //kill the missile, mainly so that it doesnt draw anymore
    i4_free(mini_objects);
    mini_objects     = 0;
    num_mini_objects = 0;
    missile          = 0;

    explode_delay = 15;
  }
}

void g1_tank_buster_class::raise_missile() 
{
  if (rack_state==LOWERED || rack_state==LOWERING)
    rack_state=RAISE;

  request_think();
}

void g1_tank_buster_class::lower_missile() 
{
  if (rack_state==RAISED || rack_state==RAISING)
    rack_state=LOWER;

  request_think();
}

void g1_tank_buster_class::think()
{  
  if (explode_delay >= 0)
  {
    if (explode_delay==0)
      damage(this, health, i4_3d_vector(0,0,1));
    else
    {
      explode_delay--;
      request_think();
    }
    
    return;
  }

  if (!check_life())
    return;

  g1_map_piece_class::think();

  i4_bool keep_going = i4_F;

  switch (rack_state)
  {
    case LOWERED:
      break;

    case RAISE:
      raising.play(x,y,h);
      rack_state = RAISING;
    
    case RAISING:      
      if (i4_rotate_to(missile->rotation.y,-i4_pi()/8 + i4_2pi(),0.1)!=0.0)
        keep_going = i4_T;

      if (missile->x < 0)
      {
        missile->x += 0.02;
        keep_going = i4_T;
      }

      if (missile->x > 0)
        missile->x = 0;

      if (keep_going)
        request_think();
      else 
        rack_state = RAISED;
      break;

    case RAISED:
      break;

    case LOWER:
      lowering.play(x,y,h);
      rack_state = LOWERING;

    case LOWERING:
      if (i4_rotate_to(missile->rotation.y,0,0.1)!=0.0)
        keep_going = i4_T;

      if (i4_rotate_to(missile->rotation.z,0,0.1)!=0.0)
        keep_going = i4_T;

      if (missile->x > -0.1984)
      {
        missile->x -= 0.02;
        keep_going = i4_T;
      }
        
      if (missile->x < -0.1984)
        missile->x = -0.1984;

      if (keep_going)
        request_think();
      else
        rack_state = LOWERED;
      break;
  }
  
  //aim the missile rack
  if (attack_target.valid()) 
  {
    request_think();

    raise_missile();

    if (rack_state==RAISED)
      fire();
  }  
  else
    lower_missile();
}
