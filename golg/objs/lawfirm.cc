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
#include "lisp/li_init.hh"

static li_int_class_member income_rate("income_rate"), 
  counter("counter"), reset_time("reset_time");
static li_float_class_member commision("commision");

li_object *g1_lawfirm_think(li_object *o, li_environment *env)
{
  g1_dynamic_object_class *me=g1_dynamic_object_class::get(li_car(o,env),env);

  if (!counter())
  {
    counter()=reset_time();

    int take_away_total=0;

    for (int i=0; i<G1_MAX_PLAYERS; i++)
      if (i!=me->player_num)
      {
        int take_away=income_rate();
        if (g1_player_man.get(i)->money() + take_away<0)
          take_away=-g1_player_man.get(i)->money();
           
        g1_player_man.get(i)->money() += take_away;
        take_away_total-=take_away;
      } 

    if (take_away_total>0)
      g1_player_man.get(me->player_num)->money() += i4_f_to_i(take_away_total * commision());

  }
  else counter()--;

  me->request_think();
  return 0;
}


li_automatic_add_function(g1_lawfirm_think, "lawfirm_think");
