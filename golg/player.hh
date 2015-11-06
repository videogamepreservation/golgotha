/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef __PLAYER_HH
#define __PLAYER_HH

#include "g1_object.hh"
#include "g1_limits.hh"
#include "player_type.hh"
#include "memory/array.hh"
#include "init/init.hh"
#include "lisp/li_optr.hh"
#include "reference.hh"
#include "objs/stank.hh"

class g1_map_class;
class g1_map_piece_class;
class g1_player_piece_class;
struct i4_pixel_format;
class r1_render_api_class;
class g1_team_api_class;
class g1_team_api_definition_class;
class g1_border_frame_class;
class g1_takeover_pad_class;
class g1_path_object_class;          // objs/path_object.hh

class g1_player_info_class
{
public:
  friend class g1_team_api_class;

private:
  g1_player_type player_num;
  g1_team_api_class *ai;
  g1_team_type team;

  // I want g1_player manager to call init_colors and new game on each player
  friend class g1_player_manager_class;  
  void init_colors(w32 player_num, const i4_pixel_format *fmt);
  void new_game(g1_player_type num);
  void think();

  int &num_points();
  int &num_stank_deaths();
  
  g1_typed_reference_class<g1_player_piece_class> commander;
  
public:
  i4_bool continue_wait;

  int &num_stank_lives();
  li_object_pointer vars;

  float kill_ratio();
  
  g1_team_type get_team() const { return team; }
  
  li_class *get_vars() { return (li_class *)vars.get(); }
  void set_vars(li_class *v) { vars=(li_object *)v; }

  int supertank_upgrade_level;
  void calc_upgrade_level();

  g1_model_ref team_flag;
  int refresh_money;
  int &money();
  int &income_rate();
  float &damage_multiplier();


  i4_array<w32> owned_objects;
  void remove_object(w32 global_id);
  void add_object(w32 global_id);
  

  g1_player_info_class();    

  g1_team_api_class *get_ai() const { return ai; }
  void set_ai(g1_team_api_class *ai);

  g1_player_type get_player_num() const { return player_num; }

  g1_player_piece_class* get_commander() { return commander.get(); }

  void set_commander(g1_player_piece_class *who) { commander=who; }

  i4_bool load(g1_loader_class *fp);
  void save(g1_saver_class *fp);

  w16 vehicle_pal[256];  

  i4_color map_player_building;
  i4_color map_player_color;
  i4_color map_attacking_player_color;  
  i4_color map_select_player_color;  

  // next pad and location on pad to build objects
  int next_pad;
  int next_location;


  
  void supertank_died(g1_object_class *killer);
  void add_points(int points);
  
  void allow_building(g1_object_type type,
                      int cost,
                      int sfx_to_play_when_built,
                      i4_image_class *normal_button_image,
                      i4_image_class *active_button_image,
                      i4_image_class *pressed_button_image);
};

class g1_player_manager_class : i4_init_class
{
  g1_player_info_class *list[G1_MAX_PLAYERS];
  const char *default_ai;
public:
  g1_player_type local_player;

  g1_player_manager_class();

  void show_message(const i4_const_str &message, w32 color, int player_for);

  // set the name of the default computer ai
  void set_default_ai(const char *ai);


  // the player on the local machine
  g1_player_info_class *get_local()
  {
    return list[local_player];
  }

  g1_player_info_class *get(g1_player_type player)
  {
    if (player>=G1_MAX_PLAYERS)
      player=g1_default_player;

    return list[player];
  }

  void save(g1_saver_class *fp)
  {
    for (w32 i=0;i<G1_MAX_PLAYERS;i++)
      list[i]->save(fp);
  }

  i4_bool load(g1_loader_class *fp)
  {
    i4_bool ret=i4_T;
    for (w32 i=0;i<G1_MAX_PLAYERS;i++)
      ret=(i4_bool)(ret & list[i]->load(fp));
    return ret;
  }

  void init_colors(const i4_pixel_format *fmt, r1_render_api_class *api);

  void new_game()
  {   
    for (w32 i=0; i<G1_MAX_PLAYERS; i++)
      list[i]->new_game((g1_player_type)i);
  }

  void think()
  {
    for (w32 i=0; i<G1_MAX_PLAYERS; i++)
      list[i]->think();
  }

  void recalc_from_map(g1_map_class *map);

  virtual void init();
  virtual void uninit();

  // for unloading dll AIs
  void unload_ai(g1_team_api_definition_class *definer);
};

extern g1_player_manager_class g1_player_man;

#endif


