/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "human.hh"
#include "resources.hh"
#include "g1_speed.hh"
#include "input.hh"
#include "objs/stank.hh"
#include "map.hh"
#include "map_man.hh"
#include "controller.hh"
#include "cwin_man.hh"
#include "time/profile.hh"
#include "player.hh"
#include "sfx_id.hh"
#include "lisp/lisp.hh"
#include "objs/path_object.hh"

g1_human_class *g1_human = 0;

void g1_human_class::load(g1_loader_class *fp)
{
}

class g1_human_def_class : public g1_team_api_definition_class
{
public:
  g1_human_def_class()
    : g1_team_api_definition_class("human") {}

  virtual g1_team_api_class *create(g1_loader_class *f)
    {
    if (!g1_human)
      own_team_api(new g1_human_class(f));

    return g1_human; 
  }
} g1_human_def;

g1_human_class::g1_human_class(g1_loader_class *f)
{
  mouse_look_increment_x = 0;
  mouse_look_increment_y = 0;
  g1_human = this;
}

g1_human_class::~g1_human_class()
{
  g1_human = 0;
}

void g1_human_class::send_selected_units(i4_float x, i4_float y)
{    
  if (selected_object.get())
    deploy_unit(selected_object->global_id, x,y);
}


static li_symbol_ref allow_follow_mode("allow_follow_mode");
static li_symbol_ref supergun("supergun");

void g1_human_class::clicked_on_object(g1_object_class *o)
{ 
  if (o && g1_path_object_class::cast(o))
    set_current_target(o->global_id);
}

w8 g1_human_class::determine_cursor(g1_object_class *object_mouse_is_on)
{
  if (selected_object.valid())
  {
    if (selected_object->id == g1_get_object_type(supergun.get()))
      return G1_TARGET_CURSOR;
    else
      return G1_MOVE_CURSOR;
  }
  else if (object_mouse_is_on)
  {    
    if (g1_path_object_class::cast(object_mouse_is_on))
        //        g1_path_object_class::cast(object_mouse_is_on)->valid_destination)
      return G1_MOVE_CURSOR;
    else
      return G1_DEFAULT_CURSOR;
  }
  else
    return G1_DEFAULT_CURSOR;
}

void g1_human_class::player_clicked(g1_object_class *obj, float gx, float gy)
{
  if (obj && obj->player_num==team())
  {
    if (selected_object.valid())
      selected_object->mark_as_unselected();

    if (obj == commander())
      selected_object = 0;
    else
    {
      selected_object = obj;
      selected_object->mark_as_selected();
    }
  }
  else if (selected_object.valid())
    send_selected_units(gx,gy);
  else if (gx>=0 && gy>=0 && gx<g1_get_map()->width() && gy<g1_get_map()->height())
  {
    g1_path_object_class *best=0, *best2=0;
    float best_dist=10000;
    for (g1_path_object_class *o=g1_path_object_list.first(); o; o=o->next)
    {
      int t=o->total_links(player->get_team());
      for (int i=0; i<t; i++)
      {
        g1_path_object_class *o2=o->get_link(player->get_team(), i);
        if (o2 && !o2->get_flag(g1_object_class::SCRATCH_BIT))
        {
          i4_2d_vector p1(o->x, o->y), p2(o2->x, o2->y);
          i4_2d_vector a=p2-p1;
          i4_2d_vector x=i4_2d_vector(gx,gy);

          g1_path_object_class *use1=0, *use2=0;
          float dist;

          float alen=a.length();
          float t=(x-p1).dot(a)/(alen*alen);
                                
          if (t<0)
            dist=(x-p1).length();
          else if (t>1)
            dist=(x-p2).length();
          else
            dist=i4_fabs(a.perp_dot(x-p1)/a.length());

          if (dist<best_dist)
          {
            best_dist=dist;
            best=o;
            best2=o2;
          }
        }              
      }
    }        

    if (best)
    {
      set_current_target(best->global_id);
      if (best2)
        set_current_target(best2->global_id);
    }
  }
}

int g1_human_class::build_unit(g1_object_type type)
{
  return (playback)? G1_BUILD_PLAYBACK : g1_team_api_class::build_unit(type);
}

void g1_human_class::think()
{
  i4_bool process_input = i4_T;

  if (playback_think()) 
    process_input=i4_F;


  g1_player_piece_class *stank = commander();  
  if ((g1_current_controller->view.get_view_mode()!=G1_ACTION_MODE &&
       g1_current_controller->view.get_view_mode()!=G1_FOLLOW_MODE) ||
      !stank || stank->health<=0)
    process_input=i4_F;
  
  if (!process_input)
  {
    g1_input.deque_time(g1_input_class::LEFT);
    g1_input.deque_time(g1_input_class::RIGHT);
    g1_input.deque_time(g1_input_class::UP);
    g1_input.deque_time(g1_input_class::DOWN);
    g1_input.deque_time(g1_input_class::STRAFE_LEFT);
    g1_input.deque_time(g1_input_class::STRAFE_RIGHT);
  }
  else
  {
    //keys are buffered in the order pressed, so do a reversed comparison
    if (memcmp(g1_input.grab_cheat_keys(),"DOG",3)==0)
    {
      g1_input.clear_cheat_keys();

      stank->toggle_stank_flag(g1_player_piece_class::ST_GODMODE);
    }
    
    look(mouse_look_increment_x, mouse_look_increment_y);
    mouse_look_increment_y = mouse_look_increment_x = 0;
    
    sw32
      left_ms=g1_input.deque_time(g1_input_class::LEFT),
      right_ms=g1_input.deque_time(g1_input_class::RIGHT),
      up_ms=g1_input.deque_time(g1_input_class::UP),
      down_ms=g1_input.deque_time(g1_input_class::DOWN),
      sleft_ms=g1_input.deque_time(g1_input_class::STRAFE_LEFT),
      sright_ms=g1_input.deque_time(g1_input_class::STRAFE_RIGHT);

    turn( g1_resources.player_turn_speed*(left_ms-right_ms)*G1_HZ/1000.0 );
    accelerate( (up_ms-down_ms)*G1_HZ/1000.0 );
    strafe( (sright_ms-sleft_ms)*G1_HZ/1000.0 );

    if (sright_ms>0)
      sright_ms=sright_ms+1;

    if (g1_input.button_1()) fire0();
    if (g1_input.button_2()) fire1();
    if (g1_input.button_3()) fire2();
  }


  if (g1_input.key_pressed)  
    continue_game();

  g1_input.key_pressed=i4_F;
}

//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}
