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
#include "resources.hh"
#include "saver.hh"
#include "map_cell.hh"
#include "map.hh"
#include "sound/sfx_id.hh"
#include "objs/engineer.hh"
#include "objs/verybiggun.hh"
#include "object_definer.hh"
#include "map_man.hh"
#include "lisp/lisp.hh"
#include "li_objref.hh"
#include "objs/path_object.hh"
#include "player.hh"

#include "image_man.hh"
static g1_team_icon_ref radar_im("bitmaps/radar/engineer.tga");

enum {DATA_VERSION=2};

static g1_model_ref model_ref("engineer_body"),
  shadow_ref("engineer_shadow"),
  back_wheels_ref("engineer_back_wheels"),
  front_wheels_ref("engineer_front_wheels"),
  lod_ref("engineer_lod");

static g1_object_type takeover_pad;
static i4_3d_vector back_attach, back_offset, front_attach, front_offset;




void g1_engineer_init()
{
  takeover_pad = g1_get_object_type("takeover_pad");

  back_attach.set(-0.12,0.0,0.1);
  model_ref()->get_mount_point("Back Wheels", back_attach);
  back_wheels_ref()->get_mount_point("Back Wheels", back_offset);
  back_offset.reverse();

  front_attach.set(0.11,0.0,0.1);
  model_ref()->get_mount_point("Front Wheels", front_attach);
  front_wheels_ref()->get_mount_point("Front Wheels", front_offset);
  front_offset.reverse();
}

g1_object_definer<g1_engineer_class>
g1_engineer_def("engineer",
                g1_object_definition_class::TO_MAP_PIECE |
                g1_object_definition_class::EDITOR_SELECTABLE |
                g1_object_definition_class::MOVABLE,
                g1_engineer_init);


g1_engineer_class::g1_engineer_class(g1_object_type id, g1_loader_class *fp)
  : g1_map_piece_class(id,fp)
{  
  draw_params.setup(model_ref.id(),shadow_ref.id(), lod_ref.id());

  allocate_mini_objects(3,"Enemy Engineer Mini-Objects");

  back_wheels = &mini_objects[0];
  back_wheels->defmodeltype = front_wheels_ref.id();
  back_wheels->position(back_attach);
  back_wheels->offset = front_offset;

  front_wheels = &mini_objects[1];  
  front_wheels->defmodeltype = front_wheels_ref.id();
  front_wheels->position(front_attach);
  front_wheels->offset = front_offset;
  
  w16 ver,data_size;
  if (fp)
    fp->get_version(ver,data_size);
  else
    ver = 0;
  
  switch (ver)
  {
    case DATA_VERSION:
      fp->read_format("ff", &front_wheels->rotation.y, &front_wheels->rotation.z);
      front_wheels->rotation.x = 0;
      front_wheels->grab_old();
      back_wheels->rotation.x = 0;
      back_wheels->rotation.y = front_wheels->rotation.y;
      back_wheels->rotation.z = 0;
      back_wheels->grab_old();
      break;

    case 1:
      back_wheels->rotation.x = fp->read_float();
      back_wheels->rotation.y = fp->read_float();
      back_wheels->rotation.z = fp->read_float();
      back_wheels->lrotation.x = fp->read_float();
      back_wheels->lrotation.y = fp->read_float();
      back_wheels->lrotation.z = fp->read_float();
      
      front_wheels->rotation.x = fp->read_float();
      front_wheels->rotation.y = fp->read_float();
      front_wheels->rotation.z = fp->read_float();
      front_wheels->lrotation.x = fp->read_float();
      front_wheels->lrotation.y = fp->read_float();
      front_wheels->lrotation.z = fp->read_float();
      break;
    default:
      if (fp) 
        fp->seek(fp->tell() + data_size);
      back_wheels->rotation.x = 0;
      back_wheels->rotation.y = 0;
      back_wheels->rotation.z = 0;  
      back_wheels->grab_old();
      front_wheels->rotation.x = 0;
      front_wheels->rotation.y = 0;
      front_wheels->rotation.z = 0;
      front_wheels->grab_old();
      break;
  }

  if (fp)
    fp->end_version(I4_LF);
  
  init_rumble_sound(G1_RUMBLE_GROUND);
  

  radar_image=&radar_im;
  radar_type=G1_RADAR_VEHICLE;
  set_flag(BLOCKING      |
           TARGETABLE    |
           GROUND        | 
           SHADOWED      |
           DANGEROUS, 1);
}
   
void g1_engineer_class::save(g1_saver_class *fp)
{
  g1_map_piece_class::save(fp);

  fp->start_version(DATA_VERSION);

  fp->write_format("ff", &front_wheels->rotation.y, &front_wheels->rotation.z);

  fp->end_version();
}



void g1_engineer_class::think()
{
  g1_map_piece_class::think();

  if (!alive())
    return;

  i4_float dangle = (theta-ltheta);
  if (dangle>i4_pi())
    dangle -= 2*i4_pi();
  else if (dangle<-i4_pi())
    dangle += 2*i4_pi();

  back_wheels->rotation.y  += speed*0.2;
  front_wheels->rotation.y += speed*0.2;

  front_wheels->rotation.z = dangle*0.2;
}


static li_symbol_ref reached("reached");

li_object *g1_engineer_class::message(li_symbol *message_name,
                                      li_object *message_params, 
                                      li_environment *env)
{
  if (message_name==reached.get())
  {
    g1_object_class *who=li_g1_ref::get(message_params,env)->value();
    if (who)
    {
      g1_path_object_class *po=g1_path_object_class::cast(who);
      if (po && po->total_links(get_team()))
        who->change_player_num(player_num);
    }    
  }
  
  return g1_map_piece_class::message(message_name, message_params, env);
}

