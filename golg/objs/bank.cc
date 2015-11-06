/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "player.hh"
#include "objs/def_object.hh"
#include "lisp/li_class.hh"
#include "li_objref.hh"
#include "controller.hh"
#include "object_definer.hh"
#include "objs/moneycrate.hh"
#include "objs/bank.hh"
#include "image_man.hh"

static g1_team_icon_ref bank_radar_im("bitmaps/radar/bank.tga");

static li_symbol_ref crate_obj("moneycrate"), moneyplane("moneyplane");

static li_int_class_member li_income_time("income_time"),
  li_moneyplanes("moneyplanes"),
  li_time("time"),
  li_crates("crates"),
  li_crate_value("crate_value"),
  li_crate_capacity("crate_capacity"),
  li_ticks_till_next_deploy("ticks_till_next_deploy"),
  li_reset_time_in_ticks("reset_time_in_ticks"); 

static li_g1_ref_class_member li_crate("crate");
static li_symbol_ref li_sym_crate("moneycrate");

static g1_model_ref model_ref("bank");
static i4_3d_vector crate_platform;

i4_isl_list<g1_bank_class> g1_bank_list[G1_MAX_PLAYERS];
int g1_bank_list_count[G1_MAX_PLAYERS] = { 0 };

S1_SFX(bank_secured, "narrative/bank_captured_22khz.wav", S1_STREAMED, 200);
S1_SFX(bank_lost, "narrative/bank_lost_22khz.wav", S1_STREAMED, 200);

static void g1_bank_init()
{
  crate_platform.set(0,0,3.0);
  model_ref()->get_mount_point("crate", crate_platform);
}

g1_object_definer<g1_bank_class>
g1_bank_def("bank", g1_object_definition_class::EDITOR_SELECTABLE, g1_bank_init);

g1_bank_class::g1_bank_class(g1_object_type id, g1_loader_class *fp)
  : g1_factory_class(id,fp)
{
  radar_type=G1_RADAR_BUILDING;

  radar_image=&bank_radar_im;

  set_flag(BLOCKING      |
           TARGETABLE    |
           SELECTABLE, 1);
}
  
g1_moneycrate_class* g1_bank_class::crate()
{
  li_class_context context(vars);
  g1_moneycrate_class *c=(g1_moneycrate_class *)li_crate()->value();
  return c;
}

void g1_bank_class::set_crate(g1_moneycrate_class *cr)
{
  vars->set(li_crate, new li_g1_ref(cr));
}

i4_bool g1_bank_class::occupy_location()
{
  if (!g1_factory_class::occupy_location())
    return i4_F;

  i4_transform_class t;
  calc_world_transform(1.0,&t);
  t.transform(crate_platform,crate_pos);

  g1_bank_list[player_num].insert(*this);
  g1_bank_list_count[player_num]++;

  request_think();
  return i4_T;
}

void g1_bank_class::unoccupy_location()
{
  g1_factory_class::unoccupy_location();
  g1_bank_list[player_num].find_and_unlink(this);
  g1_bank_list_count[player_num]--;
}
  
void g1_bank_class::change_player_num(int new_team)
{ 
  if (new_team!=player_num)
  {
    g1_bank_list[player_num].find_and_unlink(this);
    g1_bank_list_count[player_num]--;
    g1_bank_list[new_team].insert(*this);
    g1_bank_list_count[new_team]++;
      
    if (new_team==g1_player_man.local_player)
    {
      if (g1_current_controller.get())
        g1_current_controller->add_spin_event("powerup_bank", 0);
      bank_secured.play();
    }
      
    if (player_num==g1_player_man.local_player)
    {
      if (g1_current_controller.get())
        g1_current_controller->add_spin_event("powerup_bank", 1);
      bank_lost.play();
    }
  }
  g1_factory_class::change_player_num(new_team);
}

void g1_bank_class::think()
{
  if (!death.think())
    return;

  int time = li_time();
  if (time>0)
    time--;
  else
  {
    if (li_crates()<li_crate_capacity())
    {
      li_crates()++;
      time=li_income_time();
    }
  }
  li_time() = time;

  if (li_crates()>0 && !crate())
    create_crate();

  if (li_ticks_till_next_deploy())
    li_ticks_till_next_deploy()--;
  else
  {
    if (li_moneyplanes()>0 && player_num!=0)
    {
      g1_object_class *o=g1_create_object(g1_get_object_type(moneyplane.get()));

      if (o)
      {
        li_ticks_till_next_deploy() = li_reset_time_in_ticks();
        li_moneyplanes()--;
        o->player_num=player_num;
        o->x=x; o->y=y; o->h=h;
        o->theta=theta;
        
        o->grab_old();
        o->occupy_location();
        o->request_think();
        
        g1_player_man.get(o->player_num)->add_object(o->global_id);
      }
    }
  }

  request_think();
}


void g1_bank_class::request_remove()
{
  if (crate())
  {
    crate()->unoccupy_location();
    crate()->request_remove();
  }
  
  g1_factory_class::request_remove();
}

void g1_bank_class::crate_location(i4_3d_vector &pos)
{
  pos = crate_pos;
}

void g1_bank_class::create_crate()
{
  if (crate())
    return;

  li_class_context c(vars);

  set_crate((g1_moneycrate_class *)g1_create_object(g1_get_object_type(crate_obj.get())));

  i4_transform_class t;
  calc_world_transform(1.0,&t);
  
  i4_3d_point_class p;

  t.transform(crate_platform, p);
  
  if (crate())
    crate()->setup(p, li_crate_value());
}

void g1_bank_class::detach_crate()
{
  li_class_context c(vars);

  if (!crate())
    return;

  set_crate(0);

  li_crates()--;
  if (li_crates()>0)
    create_crate();
}

i4_bool g1_bank_class::build(int type)
{
  li_class_context c(vars);

  if (type!=g1_get_object_type(moneyplane.get()))
    return i4_F;
      
  int cost=g1_object_type_array[type]->defaults->cost;      
  if (cost<=g1_player_man.get(player_num)->money())
  {
    g1_player_man.get(player_num)->money()-=cost;
    li_moneyplanes()++;
    return i4_T;
  }
  return i4_F;
}
