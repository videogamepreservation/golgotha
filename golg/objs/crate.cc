/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "objs/crate.hh"
#include "objs/model_draw.hh"
#include "lisp/li_class.hh"
#include "objs/model_id.hh"
#include "object_definer.hh"
#include "map.hh"
#include "map_man.hh"
#include "g1_render.hh"
#include "math/pi.hh"
#include "r1_api.hh"
#include "objs/stank.hh"
#include "player.hh"
#include "resources.hh"


static li_symbol_class_member type("type"), amount("amount");
static li_symbol_ref li_health("health"), li_missile("missile"), 
  li_bullet("bullet"), li_money("money"), li_small("small"), li_large("large"),
  li_chain("chain_gun");

static g1_model_ref 
  health_model("crate_health"), 
  missile_model("crate_missiles"), 
  money_model("crate_money"), 
  chain_model("crate_minigun"),
  bullet_model("crate_bullets"), 
  health_flare_model("powerup_flare_blue"),
  missile_flare_model("powerup_flare_white"),
  money_flare_model("powerup_flare_green"),
  bullet_flare_model("powerup_flare_red"),
  chain_flare_model("powerup_flare_yellow");

static li_int_class_member li_ticks_left("ticks_left");
static li_float_class_member li_yvel("yvel");

S1_SFX(bullet_sfx, "misc/main_barrel_powerup_22khz.wav", 0, 200);
S1_SFX(health_sfx, "misc/health_powerup_three_22khz.wav", 0, 200);
S1_SFX(missile_sfx, "misc/missle_powerup_22khz.wav", 0, 200);
S1_SFX(chain_sfx, "misc/supertank_chain_gun_refuel.wav", 0, 200);
S1_SFX(money_sfx, "misc/powerup_money_22khz.wav", 0, 200);


float &g1_crate_class::yvel() { return vars->get(li_yvel); }
int &g1_crate_class::ticks_left() { return vars->get(li_ticks_left); }


g1_object_definer<g1_crate_class>
g1_create_def("crate", g1_object_definition_class::EDITOR_SELECTABLE);

i4_bool g1_crate_class::occupy_location()
{
  h=lh=g1_get_map()->terrain_height(x,y)+float_height();
  return g1_object_class::occupy_location_corners();
}


int g1_crate_class::added_money() 
{ 
  return get_amount()==SMALL ? 
    g1_resources.small_money_added : 
    g1_resources.large_money_added; 
}

int g1_crate_class::added_bullets() 
{ 
  return get_amount()==SMALL ? 
    g1_resources.small_bullets_added : 
    g1_resources.large_bullets_added; 
}

int g1_crate_class::added_health() 
{ 
  return get_amount()==SMALL ? 
    g1_resources.small_health_added : 
    g1_resources.large_health_added; 
}

int g1_crate_class::added_missiles() 
{ 
  return get_amount()==SMALL ? 
    g1_resources.small_missiles_added : 
    g1_resources.large_missiles_added; 
}

int g1_crate_class::added_chain() 
{ 
  return get_amount()==SMALL ? 
    g1_resources.small_chain_added : 
    g1_resources.large_chain_added; 
}



g1_crate_class::ctype g1_crate_class::get_type()
{
  li_symbol *s=vars->get(type);
  if (s==li_health.get())
    return HEALTH;
  else if (s==li_missile.get())
    return MISSILE;
  else if (s==li_bullet.get())
    return BULLET;
  else if (s==li_chain.get())
    return CHAIN;
  else return MONEY;    
}

void g1_crate_class::set_type(ctype x)
{
  li_symbol *s;
  int sub_type;

  switch (x)
  {
    case HEALTH : 
    { 
      s=li_health.get(); 
      draw_params.setup(health_model.get());  
      sub_type=health_flare_model.value;
    } break;

    case MISSILE : 
    { 
      s=li_missile.get(); 
      draw_params.setup(missile_model.get());  
      sub_type=missile_flare_model.value;
    } break;

    case BULLET : 
    { 
      s=li_bullet.get(); 
      draw_params.setup(bullet_model.get());  
      sub_type=bullet_flare_model.value;
    } break;

    case CHAIN :
    {
      s=li_chain.get(); 
      draw_params.setup(chain_model.get());  
      sub_type=chain_flare_model.value;
    } break;

    case MONEY :
    { 
      s=li_money.get(); 
      draw_params.setup(money_model.get());  
      sub_type=money_flare_model.value;
    } break;
  }



//   if (get_amount()==LARGE)
//     num_mini_objects=3;
//   else
//     num_mini_objects=3;


//   for (int i=0; i<num_mini_objects; i++)
//     mini_objects[i].defmodeltype=sub_type;



  vars->set(type, s);
}

g1_crate_class::atype g1_crate_class::get_amount()
{
  if (vars->get(amount)==li_small.get())
    return SMALL;
  else
    return LARGE;
}


void g1_crate_class::set_amount(atype x)
{
  switch (x)
  {
    case SMALL :  
      vars->set(amount, li_small.get());
      break;
    case LARGE :  
      vars->set(amount, li_large.get());
      break;
  }

}


void g1_crate_class::think()
{
  if (ticks_to_think || ticks_left())
  {
    if (ticks_left()>0)
    {
      ticks_left()--;
      if (ticks_left()==0)
      {
        unoccupy_location();
        request_remove();
        return ;
      }
    }
    else
      ticks_to_think--;

    request_think();

    theta+=0.1;
    pitch+=0.01;
    
//     mini_objects[0].rotation.x-=0.2;
//     mini_objects[0].rotation.y-=0;
//     mini_objects[0].rotation.z-=0;

//     mini_objects[1].rotation.x-=0;
//     mini_objects[1].rotation.y-=0.2;
//     mini_objects[1].rotation.z-=0;

//     mini_objects[2].rotation.x-=0.2;
//     mini_objects[2].rotation.y-=0.15;
//     mini_objects[2].rotation.z-=0.0;

//     float mh=g1_get_map()->map_height(x,y,h);
//     if (mh!=h+float_height())
//     {
//       h+=yvel();
//       yvel()-=g1_resources.gravity;
//       if (h+float_height()<mh)
//       {
//         h=mh+float_height();
//         yvel()=0;
//         request_remove();
//       }
//     }    
  }
}

void g1_crate_class::go_away()
{
  unoccupy_location();
  request_remove();
}

void g1_crate_class::note_stank_near(g1_player_piece_class *s)
{
  float dist_sqrd=(s->x-x)*(s->x-x)+(s->y-y)*(s->y-y)+(s->h-h)*(s->h-h);

  if (dist_sqrd<0.5*0.5)
  {    
    char msg[100];
    switch (get_type())
    {
      case HEALTH : 
        if (s->health!=s->ammo[3].amount)
        {
          int old=s->health;

          s->health+=added_health();
          if (s->health>s->ammo[3].amount)
            s->health=s->ammo[3].amount;
          go_away();

          sprintf(msg, "+%d Armor", s->health-old);
          health_sfx.play();
        }
        break;
        
      case CHAIN :
        if (s->ammo[2].amount!=s->ammo[2].ammo_type->max_amount)
        {
          int old=s->ammo[2].amount;
          s->ammo[2].amount+=added_bullets();
          if (s->ammo[2].amount>s->ammo[2].ammo_type->max_amount)
            s->ammo[2].amount=s->ammo[2].ammo_type->max_amount;
          go_away();

          sprintf(msg, "+%d Mini Gun", s->ammo[2].amount-old);        
        }
        break;

      case BULLET :
        if (s->ammo[0].amount!=s->ammo[0].ammo_type->max_amount)
        {         
          int old=s->ammo[0].amount;
          s->ammo[0].amount+=added_chain();
          if (s->ammo[0].amount>s->ammo[0].ammo_type->max_amount)
            s->ammo[0].amount=s->ammo[0].ammo_type->max_amount;
          go_away();

          sprintf(msg, "+%d Main Rounds", s->ammo[0].amount-old);
          chain_sfx.play();
        }
        break;

      case MISSILE :
        if (s->ammo[1].amount!=s->ammo[1].ammo_type->max_amount)
        {         
          int old=s->ammo[1].amount;
          s->ammo[1].amount+=added_missiles();
          if (s->ammo[1].amount>s->ammo[1].ammo_type->max_amount)
            s->ammo[1].amount=s->ammo[1].ammo_type->max_amount;
          go_away();

          sprintf(msg, "+%d Missiles", s->ammo[1].amount-old);
          missile_sfx.play();
        }
        break;

      case MONEY :        
        g1_player_man.get(s->player_num)->money()+=added_money();
        go_away();
        sprintf(msg, "+ $%d ", added_money());
        money_sfx.play();
        break;
    }

    g1_player_man.show_message(msg, 0x00ff00, s->player_num);

  }    
}

void g1_crate_class::draw(g1_draw_context_class *context)
{
  i4_transform_class *old = context->transform;  
  i4_transform_class view_transform;
  context->transform = &view_transform;
  
  i4_3d_vector my_interp_position = world_transform->t;
  view_transform.multiply(*old,*(world_transform));

  // draw without lighting
  g1_render.render_object(draw_params.model,
                          &view_transform,
                          0,
                          1,
                          player_num,
                          0,
                          0,
                          0);


    
//   for (int i=0; i<num_mini_objects; i++) 
//   {
//     g1_quad_object_class *model = g1_model_list_man.get_model(mini_objects[i].defmodeltype);

//     i4_transform_class l2w, w2v, tmp, l2v;
//     mini_objects[i].calc_transform(g1_render.frame_ratio, &l2w);
//     tmp.translate(x,y,h);
//     tmp.multiply(l2w);


//     l2v.multiply(*old, tmp);
//     g1_render.render_object(model,
//                             &l2v,
//                             0,
//                             1,
//                             player_num,
//                             0,
//                             0,
//                             0);

//   }


  ticks_to_think=10;
  request_think();

  context->transform=old;
}

g1_crate_class::g1_crate_class(g1_object_type id, g1_loader_class *fp)
  : g1_object_class(id, fp)
{
  ticks_to_think=0;

  //  allocate_mini_objects(3,"crate mini objects");

//   g1_mini_object *a=mini_objects;
//   int i;
//   for (i=0; i<3; i++)
//   {
//     a->x=0; a->y=0; a->h=0;
//     a->offset = i4_3d_vector(0,0,0);
//     a++;
//   }

  set_type(get_type());     // to setup the draw params

//   mini_objects[1].rotation.y=i4_pi()/2;
//   mini_objects[2].rotation.z=i4_pi()/2;
  
//   for (i=0; i<3; i++)
//     mini_objects[i].grab_old();
}


void g1_crate_class::setup(const i4_3d_vector &pos, ctype t, atype amount, int ticks)
{
  ticks_left()=ticks;

  lx=x=pos.x;
  ly=y=pos.y;
  lh=h=pos.z;
  set_type(t);
  set_amount(amount);

  if (!get_flag(MAP_OCCUPIED))
  {
    if (!occupy_location())
      return;
  }
}

void g1_crate_class::object_changed_by_editor(g1_object_class *who, li_class *old_values)
{
  if (who==this)
    set_type(get_type());
}
