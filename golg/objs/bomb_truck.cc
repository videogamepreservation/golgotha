/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "objs/map_piece.hh"
#include "object_definer.hh"
#include "lisp/lisp.hh"
#include "objs/explode_model.hh"
#include "li_objref.hh"
#include "objs/path_object.hh"
#include "map_man.hh"
#include "map.hh"
#include "saver.hh"
#include "objs/shockwave.hh"
#include "image_man.hh"
#include "tick_count.hh"
#include "sound/sfx_id.hh"
#include "controller.hh"

static g1_team_icon_ref radar_im("bitmaps/radar/bomb_truck.tga");

S1_SFX(bomb_coming, "computer_voice/bomb_truck_approaching_22khz.wav", S1_STREAMED, 200);

S1_SFX(bleep, "misc/bleep_22khz.wav", 0, 100);


static li_symbol_ref reached("reached");
li_symbol_ref shockwave("shockwave");

class g1_bomb_truck_class : public g1_map_piece_class
{
public:
  int warning_level;
  i4_bool user_warned;

  g1_bomb_truck_class(g1_object_type id, g1_loader_class *fp)
    : g1_map_piece_class(id,fp)
  {
    warning_level=0;
    draw_params.setup("bomb_truck",0,"bomb_truck_lod");

    allocate_mini_objects(1,"Bomb Truck Mini-Objects");
    user_warned=i4_F;

    radar_image=&radar_im;
    radar_type=G1_RADAR_VEHICLE;
    set_flag(BLOCKING      |
             TARGETABLE    |
             GROUND        | 
             SHADOWED      |
             DANGEROUS, 1);
  }

  void think()
  {
    g1_map_piece_class::think();
    if (warning_level>9)
      warning_level=9;

    if (g1_player_man.local_player!=player_num &&
        warning_level && (g1_tick_counter+global_id)%(10-warning_level)==0)
      bleep.play();
      
  }
  


  void damage(g1_object_class *who_is_hurting,
              int how_much_hurt, i4_3d_vector damage_dir)  
  {
    g1_object_class::damage(who_is_hurting, how_much_hurt, damage_dir);
    if (health<=0)
    {
      g1_shockwave_class *shock = NULL;
      shock = (g1_shockwave_class *)g1_create_object(g1_get_object_type(shockwave.get()));
      if (shock)
        shock->setup(i4_3d_vector(x,y,h), 0.5);

      g1_apply_damage(this, this, 0, i4_3d_vector(0,0,1));
    }
  }


  li_object *message(li_symbol *message_name,
                             li_object *message_params, 
                             li_environment *env)
  {
    if (message_name==reached.get())
    {
      g1_object_class *who=li_g1_ref::get(message_params,env)->value();
      if (who)
      {
        g1_path_object_class *po=g1_path_object_class::cast(who);
        
        if (po->bomb_warning_level()>warning_level)
          warning_level=po->bomb_warning_level();

        if (warning_level && g1_player_man.local_player!=player_num &&
            !user_warned)
        {
          user_warned=i4_T;
          if (g1_current_controller.get())
            g1_current_controller->scroll_message(i4gets("bomb_coming"));
          bomb_coming.play();
        }

        if (po && !po->total_links(get_team()))
        {
          for (int i=0; i<po->total_controlled_objects(); i++)
          {
            g1_object_class *who=po->get_controlled_object(i);
            
            int e_type=g1_get_object_type("explode_model");
            g1_explode_model_class *e = (g1_explode_model_class *)g1_create_object(e_type);
            if (e && who && who->draw_params.model)
            {
              g1_explode_params params;              
              e->setup(who, i4_3d_vector(who->x, who->y, who->h), params);

              who->unoccupy_location();
              who->request_remove();
            }
          }

          who->change_player_num(player_num);
        }
      }
    
    }
  
    return g1_map_piece_class::message(message_name, message_params, env);
  }
};

g1_object_definer<g1_bomb_truck_class>
g1_bomb_truck_def("bomb_truck", 
                  g1_object_definition_class::EDITOR_SELECTABLE |
                  g1_object_definition_class::TO_MAP_PIECE |
                  g1_object_definition_class::MOVABLE);


