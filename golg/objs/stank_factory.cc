/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "g1_object.hh"
#include "lisp/li_class.hh"
#include "g1_object.hh"
#include "li_objref.hh"
#include "math/random.hh"
#include "object_definer.hh"
#include "player.hh"
#include "controller.hh"
#include "objs/stank.hh"
#include "objs/path_object.hh"
#include "objs/bases.hh"
#include "app/app.hh"
#include "device/event.hh"
#include "mess_id.hh"
#include "device/kernel.hh"
#include "math/pi.hh"
#include "border_frame.hh"

static li_symbol_ref stank("stank");

class g1_stank_factory_class : public g1_factory_class
{
public:
  g1_stank_factory_class(g1_object_type id, g1_loader_class *fp)
    : g1_factory_class(id,fp)
  {
    flags|=THINKING;
  }
  
  i4_bool occupy_location() 
  { 
    if (occupy_location_center())
    {
      g1_factory_list.insert(*this);
      return i4_T;
    }
    else return i4_F;
  }


  void continue_game()
  {
    
  }

  void think()
  {
    request_think();

    if (!g1_player_man.get(player_num)->get_commander() &&
        !g1_player_man.get(player_num)->continue_wait)
    {
      if (g1_player_man.get(player_num)->num_stank_lives())
      {
        g1_object_class *o=g1_create_object(g1_get_object_type(stank.get()));
        g1_player_piece_class *stank=g1_player_piece_class::cast(o);
        if (stank)
        {
          g1_player_man.get(player_num)->num_stank_lives()--;
          g1_player_man.get(player_num)->calc_upgrade_level();

          stank->player_num=player_num;
          stank->x=x; stank->y=y; stank->h=h;
          stank->theta=theta-i4_pi()/2.0;
          stank->turret->rotation.z = stank->base_angle = stank->theta;

          stank->grab_old();
          stank->occupy_location();

          g1_player_man.get(player_num)->add_object(stank->global_id);

          g1_player_man.get(player_num)->set_commander(stank);



          if (player_num==g1_player_man.get_local()->get_player_num() && 
              g1_current_controller.get())
          {
            if (g1_border.get())
            {
              if (g1_border->strategy_on_top)
                li_call("strategy_toggle");
              else
                g1_current_controller->view.suggest_camera_mode(G1_ACTION_MODE);
            }
          }


          stank->request_think();    
        }
        else delete o;
      }
      else 
      {
        if (g1_player_man.local_player==player_num)
        {
          i4_user_message_event_class loser(G1_YOU_LOSE);
          i4_kernel.send_event(i4_current_app, &loser);
        }
      }
    }
  }

  virtual i4_bool build(int type)
  {
    if (type!=g1_supertank_type || g1_player_man.get(player_num)->num_stank_lives()>=5)
      return i4_F;

    if (g1_player_man.get(player_num)->money()>=g1_object_type_array[type]->defaults->cost)
    {
      g1_player_man.get(player_num)->money()-=g1_object_type_array[type]->defaults->cost;
      g1_player_man.get(player_num)->num_stank_lives()++;
      return i4_T;
    }
    else return i4_F;



  }
  
};

static g1_object_definer<g1_stank_factory_class>
mainbase_def("mainbasepad", g1_object_definition_class::EDITOR_SELECTABLE);

