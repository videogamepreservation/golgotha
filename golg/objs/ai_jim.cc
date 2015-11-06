/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "team_api.hh"
#include "player.hh"
#include "memory/array.hh"
#include "math/angle.hh"
#include "math/trig.hh"
#include "map.hh"
#include "resources.hh"
#include "time/profile.hh"
#include "objs/stank.hh"
#include "objs/path_object.hh"
#include "objs/bases.hh"
#include "g1_rand.hh"
#include "tick_count.hh"
#include "saver.hh"
#include "lisp/lisp.hh"

//(Oli) hack for reloading this AI
extern w16 ai_jim_dll_ref;

char *unit_name[] = {
  "stank",            //a
  "peon_tank",        //b
  "electric_car",     //c
  "rocket_tank",      //d
  "tank_buster",      //e
  "trike",            //f
  "engineer",         //g
  "bomb_truck",       //h
  "helicopter",       //i
  "jet",              //j
  "bomber",           //k
};
const int num_unit_types=sizeof(unit_name)/sizeof(char*);
w16 unit_type[num_unit_types];

char *formation[] =
{
  // LAND

  "bcbcbcedddg",                // stock
  "cccccc",                     // electric brigade
  "bbcdebbcdegheddd",           // hole puncher
  "b b b ",                     // gene's checker
  "bbbbbdd",                    // peon V
  "ccbbedddddd",                // rockets R us
  "bbdg",                       // small band
  "ccdg",                       // yet another small band
  "ddeg",                       // yet another small band
  "g",                          // wild shot
  "bbbbcccceh",                 // ouch!
  "h",                          // woohoo!

  // AIR

  "iii",                        // heli hell
  "jikjikjik",                  // airraid
  "jjiiiikkkkkkkk",             // the sky is falling
  "kkk",                        // bombsquad
};
int num_formations=sizeof(formation)/sizeof(char*);

class ai_jim : public g1_team_api_class
{
public:
  enum
  {
    DATA_VERSION=1
  };
  enum
  {
    BUILD,
    BUILDWAIT,
    RESURRECT
  };

  sw16 mode;
  w16 time;
  sw16 form, spot;
  w8 reload;

  ai_jim(g1_loader_class *f)
  {
    reload=0;
  }

  virtual void init()
  //{{{
  {
    for (int i=0; i<num_unit_types; i++)
      unit_type[i] = object_type(unit_name[i]);

    mode = BUILD;
    form = -1;
    spot = 0;
  }
  //}}}

  virtual void load(g1_loader_class *f)
  //{{{
  {
    w16 ver=0,data_size;

    f->get_version(ver,data_size);

    switch (ver)
    {
      case DATA_VERSION:
        f->read_format("22228",
                       &mode, &time, &form, &spot, &reload);
        break;
      default:
        f->seek(f->tell() + data_size);
        break;
    }
    f->end_version(I4_LF);
  }
  //}}}

  virtual void save(g1_saver_class *f)
  //{{{
  {
    f->start_version(DATA_VERSION);
    f->write_format("22228",
                    &mode, &time, &form, &spot, &reload);
    f->end_version();
  }
  //}}}

  void find_new_target()
  //{{{
  {
    g1_path_object_class *p;

    for (int tries=0; tries<30; tries++)
    {
      int t=0;
      for (p=g1_path_object_list.first(); p; p=p->next)
        t++;

      g1_team_type my_team=g1_player_man.get(team())->get_team();

      if (t)
      {
        int i=g1_rand(tries)%t;
        for (p=g1_path_object_list.first(); i--; p=p->next);

        set_current_target(p->global_id);

        int bad_path=0;
        for (g1_factory_class *f=g1_factory_list.first(); f; f=f->next)
        {
          for (p=f->get_start();p;)
          {
            if (p->get_bridge_status()==g1_path_object_class::HAS_BRIDGE)
              bad_path=1;

            p=p->get_recent_link(my_team, 0);            
          }
        }

        if (!bad_path)
          return;
      }
    }
  }
  //}}}

  virtual void object_added(w32 id)
  //{{{
  {
    // use this to get path_objects
  }
  //}}}

  void guide_hero()
  //{{{
  {
    return ;    // no stank for demo

    if (!commander() || commander()->health<=0)
    {
      build_unit(unit_type[0]);
      return;
    }

    if (ammo0()==0 && ammo1()==0 && ammo2()==0 && !reload)
    {
      /*
        jc fixme : send supertank to home base
      tx = map()->player_pad_info[player->get_player_num()].bases[0].x;
      ty = map()->player_pad_info[player->get_player_num()].bases[0].y;
      
      commander()->deploy_to(tx,ty);
      reload = 1;
      */
    }
    
    if (reload)
    {
      if (full0() && full1() && full2())
        reload=0;

      return;
    }

    if (!commander()->attack_target.valid())
    {
      commander()->find_target();
      if (!commander()->attack_target.valid() && !commander()->path)
      {
        i4_float tx,ty;

        tx = i4_float(g1_rand(1)%(map()->width()-4))  + 2.5;
        ty = i4_float(g1_rand(2)%(map()->height()-4)) + 2.5;

        deploy_unit(commander()->global_id, tx, ty);
      }
    }
    else
    {
      commander()->set_path(0);
	      
      g1_object_class *mp = commander()->attack_target.get();

      if (mp)
      {
        i4_float dx,dy,angle, adiff, dist2;

        i4_3d_point_class pos;
        commander()->lead_target(pos,0);
	
        //this will obviously only be true if me->attack_target.ref != NULL    
        dx = (pos.x - (commander()->x+commander()->turret->x));
        dy = (pos.y - (commander()->y+commander()->turret->y));
	dist2 = dx*dx+dy*dy;

        //aim the turet
	
        angle = i4_atan2(dy,dx);
        i4_normalize_angle(angle);    

        i4_float dangle = (angle - commander()->base_angle);
        if (dangle>i4_pi())
          dangle -= 2*i4_pi();
        else if (dangle<-i4_pi())
          dangle += 2*i4_pi();

        turn(dangle);

        if (dangle>-0.04 && dangle<0.04)
          if      (mp->id == g1_supertank_type && in_range1() && fire1()) ;
          else if (in_range0() && fire0()) ;
          else if (in_range2() && fire2()) ;
        
        if (dist2<9.0)
          strafe(-0.7);
        else if (dist2<64.0)
        {
          strafe(0.5);
          accelerate(0.1);
        }
        else
        {
          strafe(-0.1);
          accelerate(1);
        }
      }
    }
  }
  //}}}

  virtual void think()
  {
    if ((g1_rand(0)&7)==0)
    {
      li_object *list=li_get_value("enemy_buildable");
      int t=li_length(list, 0);
      if (t)
      {
        int type=g1_get_object_type(li_symbol::get(li_nth(list, g1_rand(1)%t, 0),0));
        build_unit(type);
      }
    }

    if ((g1_rand(2)&8)==0)
      find_new_target();

#if 0
    guide_hero();

    if (form<0)
    {
      find_new_target();
      form = (g1_rand(4))%num_formations;
      spot = 0;
    }
    switch (mode)
    {
      case BUILD:
        if (formation[form][spot]!=' ')
          build_unit(unit_type[formation[form][spot] - 'a']);

        if (formation[form][++spot]==0)
          form = -1;

        time=10;
        mode = BUILDWAIT;
        break;
      case BUILDWAIT:
        if (time>0)
          time--;
        else
          mode = BUILD;
        break;
    }
#endif
  }
};
g1_team_api_definer<ai_jim> ai_jim_def("ai_jim");

//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}
