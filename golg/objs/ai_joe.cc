/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "team_api.hh"
#include "memory/array.hh"
#include "math/random.hh"
#include "math/angle.hh"
#include "math/trig.hh"
#include "time/profile.hh"
#include "objs/stank.hh"
#include "objs/goal.hh"

//(OLI) hack for reloading this AI
extern w16 ai_joe_dll_ref;

char *unit_name[] = 
{
  "stank",                      //0
  "peon_tank",                  //1
  "electric_car",               //2
  "rocket_tank",                //3
  "",
  "",
//  "helicopter",                 //4
//  "jet",                        //5
  "engineer",                   //6
  "trike"                       //7
};
const int num_unit_types=sizeof(unit_name)/sizeof(char*);
w16 unit_type[num_unit_types];

char *goodies_name[] = 
{
  "takeover_pad",               //1
  "factory_pad"                 //2
};
const int num_goodies_types=sizeof(goodies_name)/sizeof(char*);
w16 goodies_type[num_goodies_types];

char *formation[] =
//{{{
{
  "      "
  "      "
  "121212"
  "      "
  "1 63 1"
  "  33  ", // stock

  "222222"
  "      "
  "      "
  "      "
  "      "
  "      ", // electric brigade

  "  11  "
  " 1221 "
  "123321"
  "      "
  "      "
  "      ", // hole puncher

  "12221 "
  "23632 "
  "      "
  "      "
  "      "
  "      ", // armored convoy

  "      "
  "      "
  "1 1 1 "
  " 1 1 1"
  "7 7 7 "
  " 7 7 7", // gene's checker

  "  1   "
  " 161  "
  "1   1 "
  "      "
  "      "
  "      ", // peon V

  "  4   "
  " 4 4  "
  "      "
  "      "
  "      "
  "      ", // heli hell

  "111111"
  " 3663 "
  " 3333 "
  "      "
  "      "
  "      ", // rockets R us

  "22    "
  "66    "
  "      "
  "      "
  "      "
  "      ", // small band

  "22    "
  "36    "
  "      "
  "      "
  "      "
  "      ", // yet another small band

  "33    "
  "66    "
  "      "
  "      "
  "      "
  "      ", // yet another small band

  "6     "
  "      "
  "      "
  "      "
  "      "
  "      ", // long shot

  "2222  "
  "3333  "
  "      "
  "      "
  "      "
  "      ", // ouch!
};
//}}}
const int num_formations=sizeof(formation)/sizeof(char*);

enum eDir
{
  N, E, S, W
};

sw16 dirx[] = {0, 1, 0, -1};
sw16 diry[] = {1, 0, -1, 0};


i4_profile_class pf_ai_joe_setup_production("ai_joe_setup_production");
i4_profile_class pf_ai_joe_setup_new_pieces("ai_joe_setup_new_pieces");
i4_profile_class pf_ai_joe_cleanup("ai_joe_cleanup");
i4_profile_class pf_ai_joe_next_spot("ai_joe_next_spot");
i4_profile_class pf_ai_joe_guide_hero("ai_joe_guide_hero");
i4_profile_class pf_ai_joe_think("ai_joe_think");

class ai_joe : public g1_team_api_class
{
public:
  enum eBuildMode 
  { INIT, INITWAIT, IDLE, BUILDWAIT, BUILD, WAITFORPIECE, FORMANDMOVE, RESURRECT };

  i4_float tx,ty;
  sw16 bx,by;
  i4_float prodx,prody;
  eDir dir1, dir2;

  i4_array<unit_class> troop;
  i4_array<sw32> time;
  i4_array<unit_class> build;
  i4_array<object_class> goal;
  int form, place, formation_size;
  int build_count;
  int build_wait, retry;
  eBuildMode build_mode;

  w16 goal_type;

  int reload;  // supertank is going to reload ammo

  ai_joe(g1_loader_class *f=0) 
    : troop(100,20), time(50,20), build(18,0), goal(20,10)
  //{{{
  {
    build_mode = INIT;
    form = -1;
    place = 0;
  }
  //}}}

  ~ai_joe()
  //{{{
  {
  }
  //}}}

  void setup_production(i4_float _prodx,i4_float _prody)
  //{{{
  {
    pf_ai_joe_setup_production.start();
    // setup around production site at location

    prodx = _prodx;
    prody = _prody;

    int
      dx = int(prodx - map_width()/2),
      dy = int(prody - map_height()/2);

    int num=0;

    num |= (dx<0)  ?0:8;
    num |= (dy<0)  ?0:4;
    num |= (dx<dy) ?0:2;
    num |= (dx<-dy)?0:1;

    switch (num)
    {
      case  5:  //N quadrant
      case 13:  dir1 = S;  break;
      case 15:  //E quadrant
      case 11:  dir1 = W;  break;
      case 10:  //S quadrant
      case  2:  dir1 = N;  break;
      case  0:  //W quadrant
      case  4:  dir1 = E;  break;
    }
    dir2 = eDir((dir1+1)%4);

    reload=0;
    pf_ai_joe_setup_production.stop();
  }
  //}}}

  virtual void init()
  //{{{
  {
    int i;

    troop.clear();
    time.clear();
    build.clear();

    for (i=0; i<num_unit_types; i++)
      unit_type[i] = object_type(unit_name[i]);
    for (i=0; i<num_goodies_types; i++)
      goodies_type[i] = object_type(goodies_name[i]);
    goal = object_type("goal");

    build_mode = INIT;
    form = -1;
    place = 0;

    dir1 = eDir(g1_rand(20)%4);
    dir2 = eDir((dir1+1)%4);
  }
  //}}}

  void cleanup()
  //{{{
  {
    pf_ai_joe_cleanup.start();
    int i;
    
    for (i=troop.size()-1; i>=0; i--)
      if (!troop[i].alive())
        troop.remove(i);
    pf_ai_joe_cleanup.stop();
  }
  //}}}

  int next_spot()
  //{{{ sets next formation spot.. returns false if no more left
  {
    pf_ai_joe_next_spot.start();

    while (place<6*6 && formation[form][place]==' ')
      place++;

    if (place==6*6)
    {
      pf_ai_joe_next_spot.stop();
      return 0;
    }

    int x=(place%6)*2-5, y=5-(place/6)*2;

    bx = (5 + dirx[dir1]*y + dirx[dir2]*x)/2;
    by = (5 + diry[dir1]*y + diry[dir2]*x)/2;

    pf_ai_joe_next_spot.stop();
    return 1;
  }
  //}}}

  void guide_hero()
  //{{{
  {
    pf_ai_joe_guide_hero.start();

    if (health()<=0)
    {
#if 0
      if (build_mode == IDLE)
        build_mode = RESURRECT;
#endif      
      pf_ai_joe_cleanup.stop();
      return;
    }

    if (ammo1()==0 && ammo2()==0 && ammo3()==0 && !reload)
    {
      /*
        jc fixme : send supertank to home base
      tx = map()->player_pad_info[player->get_player_num()].bases[0].x;
      ty = map()->player_pad_info[player->get_player_num()].bases[0].y;

      
      send_convoy(commander()->convoy->global_id, tx, ty);
      reload = 1;
      */
    }
    
    if (reload)
    {
      if (full1() && full2() && full3())
        reload=0;

      pf_ai_joe_cleanup.stop();
      return;
    }

    if (!commander()->attack_target.valid())
    {
      commander()->find_target();
      if (!commander()->attack_target.valid())
      {
        tx = g1_rand(50)%map_width()  + 0.5;
        ty = g1_rand(60)%map_height() + 0.5;

        deploy_unit(commander()->global_id, tx, ty);
      }
    }
    else
    {
      g1_object_class *mp = commander()->attack_target.get();

      if (mp)
      {

        i4_float dx,dy,angle, adiff;
	
        //this will obviously only be true if me->attack_target.ref != NULL    
        dx = (mp->x - (commander()->x+commander()->turet->x));
        dy = (mp->y - (commander()->y+commander()->turet->y));
	
        //aim the turet
	
        angle = i4_atan2(dy,dx);
        i4_normalize_angle(angle);    
        adiff = i4_angle_diff(commander()->base_angle, angle);

        if (adiff<0.04)
          if (mp->id == g1_supertank_type)
            fire2() || fire1() || fire3();
          else
            fire1() || fire3();
        if (angle>commander()->base_angle)
          turn(adiff);
        else
          turn(-adiff);
        strafe(0.4);
      }
    }
    
    pf_ai_joe_guide_hero.stop();
  }
  //}}}

  void guide_units()
  //{{{
  {
    for (int i=0; i<time.size(); i++)
    {
      if (time[i]>0)
        time[i]--;
      else 
      {
        int went=0;

        if (goal.size()>0)
        {
          tx = g1_rand(70)%(map_width()-10) + 5.5;
          ty = g1_rand(80)%(map_height()-10) + 5.5;
          deploy_unit(group[i].id(), tx,ty);
          time[i]=g1_rand(9)%500+500;
        }

        tx = g1_rand(90)%(map_width()-10) + 5.5;
        ty = g1_rand(100)%(map_height()-10) + 5.5;
        deploy_unit(group[i].id(), tx,ty);
        time[i]=g1_rand5()%500+500;
      }
    }
  }
  //}}}

  virtual void object_built(w32 id)
  //{{{
  {
    g1_team_api_class::object_built(id);

    build.add(unit(id));
  }
  //}}}
  
  virtual void object_added(w32 id)
  //{{{
  {
    object_class obj(object(id));
    unit_class u(unit(id));
    int i;

    if (obj.alive())
    {
      if (obj.id()==goodies_type[0])
        goal.add(obj);
      else if (u.alive())
      {
        for (i=0; i<num_unit_types && obj.id()!=unit_type[i]; i++) ;

        if (i<num_unit_types)
        {
          troop.add(u);
          for (i=0; i<group.size() && group[i].id()!=u.convoy_id(); i++) ;

          if (i==group.size() && !u.built())
          {
            group.add(convoy(u.convoy_id()));
            time.add(0);
          }
        }
      }
    }
  }
  //}}}

  virtual void think()
  //{{{
  {
    pf_ai_joe_think.start();

    cleanup();

    if (form<0)
    {
      // determine next formation
      form = g1_rand(99)%num_formations;
      place = 0;
      build_count = 0;
      build.clear();
    }

    guide_units();

    guide_hero();

    switch (build_mode)
    {
      case INIT:
        //{{{
      {
        /* jc fixme
        w32 p = build_unit(unit_names[3]);  // rocket sentinel

        if (p)
        {
          int i=troop.add(unit(p));
          setup_production(troop[i].x(), troop[i].y());
          int c=group.add(convoy(built_convoy_id()));
          time.add(-1);

          bx = prodx - dirx[dir1]*3;
          by = prody - diry[dir1]*3;
          send_convoy(built_convoy_id(), bx, by);
          build_mode = INITWAIT;
          build_wait = 50;
        } */
        build_mode = INITWAIT;
        build_wait = 50;
      } break;
      //}}}
      case INITWAIT:
        //{{{
        if (--build_wait<=0)
          build_mode = IDLE;
        break;
        //}}}
      case IDLE:
        //{{{
      {
        if (troop.size()<50)
          build_mode = BUILD;
      } break;
        //}}}
      case BUILDWAIT:
        //{{{
        if (--build_wait<=0)
          build_mode = BUILD;
        break;
        //}}}
      case BUILD:
        //{{{
        if (build_unit(unit_type[formation[form][place] - '0'], bx, by) == G1_BUILD_OK)
          build_count++;
        place++;
        break;
        //}}}
      case WAITFORPIECE:
        //{{{
        // wait for object_build or build_error to complete
        break;
        //}}}
      case FORMANDMOVE:     // jc:fixme
        build_mode=INITWAIT;
        build_wait=g1_rand(103)%500+100;
        break; 

      case RESURRECT:
        //{{{
        build_unit(unit_type[0],0,0);
        build_wait=20;
        build_mode=INITWAIT;
        break;
        //}}}
    }
    
    pf_ai_joe_think.stop();
  }
  //}}}
};

g1_team_api_definer<ai_joe> ai_joe_def("ai_joe");


// AI joe is now chosen as the default through (g1_set_deault_ai "joe") in scheme/start.scm

//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}

