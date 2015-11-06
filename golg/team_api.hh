/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef TEAM_API_HH
#define TEAM_API_HH

#include "global_id.hh"
#include "objs/map_piece.hh"

class i4_file_class;                       // file/file.hh
class g1_player_piece_class;               // objs/stank.hh
class g1_player_info_class;                // player.hh
class g1_team_api_definition_class;        // later in file

class g1_team_api_class
{
private:
  friend class g1_player_info_class;
  friend class g1_team_api_definition_class;

  const g1_team_api_definition_class *def;

protected:
  i4_file_class *record, *playback;
public:
  g1_player_info_class *player;

  // required by human to set mode of stank
  friend class g1_human_class;
  g1_player_piece_class *commander() const;       // get supertank pointer (NULL = dead)

  const g1_team_api_definition_class *definer() const { return def; }
  g1_team_api_class();
  virtual ~g1_team_api_class();

  // super tank info
  sw16 health() const;
  w16 ammo0() const;            // amount of ammo
  w16 ammo1() const;
  w16 ammo2() const;
  i4_bool full0() const;        // ammo fully loaded?
  i4_bool full1() const;
  i4_bool full2() const;
  i4_bool in_range0() const;    // attack target in range?
  i4_bool in_range1() const;
  i4_bool in_range2() const;

  // super tank input (mainly useful to human player)
  void turn(i4_float angle);                      // turn supertank clockwise
  void accelerate(i4_float ratio);                // accelerate ratio of maximum acceleration
  void strafe(i4_float ratio);                    // slide right ratio of maximum acceleration

  void look(i4_float dax, i4_float day);          // set turret direction relative to heading
  i4_bool fire0();                                // fire first weapon
  i4_bool fire1();                                // fire second weapon
  i4_bool fire2();                                // fire third weapon

  i4_bool continue_game();                        // when dead/game over etc, this indicates game continues

  // interface to units
  class unit_class
  //{{{
  {
  protected:
    friend class g1_team_api_class;

    w32 global_id;
    unit_class(w32 id) : global_id(0)
    {
      if (g1_global_id.check_id(id) && g1_map_piece_class::cast(g1_global_id.get(id)))
        // living map pieces only
        global_id = id;
    }
    g1_map_piece_class *cast() const { return (g1_map_piece_class *)g1_global_id.get(global_id); }
  public:
    unit_class() : global_id(0) {}
    unit_class(const unit_class &a) : global_id(a.global_id) {}

    i4_bool moving() const 
    {
      g1_map_piece_class *p=cast();
      return (p->x==p->lx && p->y==p->ly);
    }

    i4_float x() const        { return cast()->x; }
    i4_float y() const        { return cast()->y; }
    i4_float height() const   { return cast()->h; }
    i4_float dir() const      { return cast()->theta; } 
    sw16 health() const       { return cast()->health; }
    w8 team() const           { return cast()->player_num; }

    w32 id() const            { return global_id; }
    w32 type() const          { return cast()->id; }

    i4_bool alive() const     { return g1_global_id.check_id(global_id) && health()>0; }
    i4_bool built() const     { return g1_global_id.check_id(global_id) && cast()->alive(); }
  };
  //}}}
  unit_class unit(w32 id) { return unit_class(id); }

  // interface to objects
  class object_class
  //{{{
  {
  protected:
    friend class g1_team_api_class;

    w32 global_id;
    object_class(w32 id) : global_id(id)
    {
      if (!alive())
        global_id = 0;
    }
    g1_object_class *cast() const { return g1_global_id.get(global_id); }
  public:
    object_class() : global_id(0) {}
    object_class(const object_class &a) : global_id(a.global_id) {}

    i4_float x() const        { return cast()->x; }
    i4_float y() const        { return cast()->y; }
    i4_float height() const   { return cast()->h; }
    i4_float dir() const      { return cast()->theta; } 
    sw16 health() const       { return cast()->health; }
    w8 team() const           { return cast()->player_num; }

    w32 id() const            { return global_id; }
    w32 type() const          { return cast()->id; }

    i4_bool alive() const     { return g1_global_id.check_id(global_id) && health()>0; }
  };
  //}}}
  object_class object(w32 id) { return object_class(id); }

  // strategic info

  g1_player_type team() const;
  sw32 money() const;

  g1_map_class *map() const;                      // get strategic map
  w32 map_width() const;
  w32 map_height() const;
  i4_float terrain_height(i4_float x, i4_float y,     // get map height below location
                          i4_float z=0) const;

  i4_bool is_building() const;                    // still building objects?

  w16 object_type(const char *name) const;

  // ******************** COMMANDS (sent to server) *******************
  i4_bool deploy_unit(w32 global_id, i4_float x, i4_float y);  // send object to location 
                   
  int build_unit(g1_object_type type); 
  // try to build unit

  i4_bool set_current_target(w32 global_id); // should be a path_object!

  // ******************** RESPONSES (come back from server) ************
  virtual void object_built(w32 id)
  {
  }
  
  virtual void object_lost(w32 id)     {}
  virtual void object_added(w32 id)    {}

  virtual void init()                       {} 
  // should handle reinitialization as well.  assume that everything about the level disappeared.

  virtual void save(g1_saver_class *fp) {}
  virtual void load(g1_loader_class *fp) {}

  // playback handling
  i4_bool record_start(char *name);
  void record_end();
  i4_bool playback_start(i4_file_class *fp);     // this file pointer will be closed when teamapi is done with it
  void playback_end();
  i4_bool playback_think();                       // think function to implement playbacks
  void post_think();                              // stuff to do after thinking (recording)

  // team think
  virtual void think() = 0;                       // AI's think function
};

// AI definition class to add new AI's to game
class g1_team_api_definition_class
{
protected:
  static g1_team_api_definition_class *first;
  g1_team_api_definition_class *next;

  virtual g1_team_api_class *create(g1_loader_class *f)=0;
  const char *_name;
  g1_team_api_class* own_team_api(g1_team_api_class *ai) { ai->def = this; return ai; }
public:
  g1_team_api_definition_class(const char *name);
  ~g1_team_api_definition_class();

  const char *name() const { return _name; }
  static g1_team_api_class *create(const char *name, g1_loader_class *f=0);
};

// give a simpler alias to the create mechanism
inline g1_team_api_class *g1_create_ai(const char *name, g1_loader_class *f=0)
{
  return g1_team_api_definition_class::create(name,f);
}

// template magic to ease adding new AI's into game
template <class T>
class g1_team_api_definer : public g1_team_api_definition_class
{
public:
  g1_team_api_definer<T>(const char *name)
    : g1_team_api_definition_class(name) {}

  virtual g1_team_api_class *create(g1_loader_class *f) { return own_team_api(new T(f)); }
};

#endif

//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}
