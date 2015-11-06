/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

// Golgatha Generic game object class
//
// The g1_object_class is the base type for all non-building/ground objects in the game.


#ifndef G1_OBJECT_HH
#define G1_OBJECT_HH

#include "arch.hh"
#include "math/num_type.hh"
#include "g1_limits.hh"
#include "math/transform.hh"
#include "image/context.hh"

#include "obj3d.hh"
#include "reference.hh"
#include "draw_context.hh"
#include "error/error.hh"
#include "player_type.hh"
#include "objs/model_id.hh"
#include "objs/model_draw.hh"
#include "range.hh"
#include "global_id.hh"


class g1_loader_class;
class g1_saver_class;
class g1_poly_list;
class g1_saver_class;
class g1_loader_class;
class i4_graphical_style_class;
class i4_event_handler_class;         // defined in device/device.hh
class li_object;
class li_symbol;
class li_environment;
class li_class;
class g1_object_definition_class;
class g1_screen_box;
struct g1_object_defaults_struct;     // objs/defaults.hh
class g1_damage_map_struct;            // objs/defaults.hh
struct g1_team_icon_ref;
class g1_player_piece_class;

//g1_mini_object stuff
class g1_mini_object
{
public:
  void draw(g1_draw_context_class *context, 
            i4_transform_class *transform,
            g1_screen_box *bound_box,
            g1_player_type player,
            i4_transform_class *use_this_transform=0, //if you want to supply a local space transform
            i4_bool pass_world_space_transform=i4_T,  // if you don't want lighting pass i4_F
            i4_bool use_lod_model=i4_F);
            
  i4_3d_vector offset;       // tells where the center of mini_object model is (where to rotate)
  i4_3d_vector loffset;

  i4_float x,y;              // location in game map in game coordinates
  i4_float lx,ly;            // last tick position (use to interpolate frames)
	
  i4_float h;                // height above (or below ground)
  i4_float lh;               // last tick height

  i4_3d_vector rotation;     // contains rotation information
  i4_3d_vector lrotation;
  
  g1_model_id_type defmodeltype; //model used by this mini object. can be overridden with draw()
  g1_model_id_type lod_model;

  w16 frame;          // current animation frame
  w16 animation;      // current animation number in model
  
  void calc_transform(i4_float ratio, i4_transform_class *trans);
  // calculates transform to parent system

  void position(const i4_3d_vector &pos) { lx=x=pos.x; ly=y=pos.y; lh=h=pos.z; }
  
  void grab_old()
  //be sure this is called by any parent object grab_old()'s
  {
    lx=x;
    ly=y;
    lh=h;
    lrotation = rotation;    
    loffset = offset;
  }
};



class g1_object_chain_class
{
public:
  g1_object_class       *object;
  g1_object_chain_class *next;
  w16 offset;

  inline g1_object_chain_class *next_solid();  // defined after g1_object_class
  g1_object_chain_class *next_non_solid() { return next; }  
};


enum g1_radar_type
{
  G1_RADAR_NONE,
  G1_RADAR_VEHICLE,
  G1_RADAR_STANK,
  G1_RADAR_WEAPON,
  G1_RADAR_BUILDING,
  G1_RADAR_PATH_OBJECT
};


typedef sw16 g1_object_type;

class g1_object_class
{
protected:
  friend class g1_reference_class;
  friend class g1_map_class;
  friend class g1_map_cell_class;
  friend class g1_remove_manager_class;

  void load_v2(g1_loader_class *fp);
  void load_v3(g1_loader_class *fp);
  void load_v4(g1_loader_class *fp);
  void load_v5(g1_loader_class *fp);
  void load_v6(g1_loader_class *fp);
  void load_v7(g1_loader_class *fp);
  void load_v8(g1_loader_class *fp);
  void load_v9(g1_loader_class *fp);

  // a list of all objects that reference us, so we can remove the reference when we delete ourself
  i4_isl_list<g1_reference_class> ref_list;
  
  // list of squares this object is on (max of 4), if value is 0xffff then it is not a reference
  // the last one holds the center of the object, if necessary (ie, the object takes 
  // up more than 1 square and his corners are far enough apart)
  i4_array<g1_object_chain_class> occupied_squares;

  g1_object_chain_class *new_occupied_square()
  {
    g1_object_chain_class *ret = occupied_squares.add();
    ret->object = this; 
    return ret;
  }

  i4_bool occupy_location_model(const g1_model_draw_parameters& draw_params);
  i4_bool occupy_location_corners();
  i4_bool occupy_location_center();     // only adds the center of the object
  i4_bool occupy_location_model_extents(const g1_model_draw_parameters &draw_params);
public:

  // generic type flags for run time object caster, defaults to can't cast
  enum
  {
    TO_MAP_PIECE      = (1<<0),
    TO_PLAYER_PIECE   = (1<<1),
    TO_DYNAMIC_OBJECT = (1<<2),
    TO_PATH_OBJECT    = (1<<3),
    TO_DECO_OBJECT    = (1<<4),
  };

  virtual w32 private_safe_to_cast() const { return 0; }

  w32 global_id;                // unique to the object, used to reference via networking

  g1_object_type id;            // this is the object's 'type' id

  i4_float x,y;                 // location in game map in game coordinates
  i4_float lx,ly;               // last tick position (use to interpolate frames)

  i4_float h;                   // height above (or below ground)
  i4_float lh;                  // last tick height

  i4_float theta;               // facing direction in the x-y game plane
  i4_float ltheta;              // last tick theta (yaw)

  i4_float pitch;               // pitch (y-z)
  i4_float lpitch;              // last tick pitch

  i4_float roll;                // roll (x-z)
  i4_float lroll;               // last tick roll

  sw16 health;                  // the health its got left

  li_class *vars;               // lisp variables used for dialog's.  These load and save automatically.

  g1_player_type player_num;
  g1_radar_type radar_type;
  g1_team_icon_ref *radar_image;


  virtual void change_player_num(int new_player_num);  // don't change player_num directly
  virtual g1_team_type get_team() const;
  
  enum flag_type
  {
    SELECTED      =1<<0,   // if object has been selected by player
    THINKING      =1<<1,   // if object is in a level's think list
 
    SHADOWED      =1<<2,   // if object should have shadows drawn for it
    MAP_INVISIBLE =1<<3,   // if object is invisible
    DELETED       =1<<4,   // if object has been removed, but remove has not been processed yet
    IN_TUNNEL     =1<<5,   // if the object is in a tunnel

    SELECTABLE    =1<<6,   // if the user can select the object in the game
    TARGETABLE    =1<<7,   // if object can be attacked

    GROUND        =1<<8,
    UNDERWATER    =1<<9,
    AERIAL        =1<<10,

    HIT_GROUND    =1<<11,
    HIT_UNDERWATER=1<<12,
    HIT_AERIAL    =1<<13,  // can attack objects in the air

    BLOCKING      =1<<14,  // if object blocks passage of other objects

    SCRATCH_BIT   =1<<15,  // use this for temporary calculations, but set it back to 0 when done
    MAP_OCCUPIED  =1<<16,  // makes sure you don't call occupy_location/unoccupy_location twice
    
    DANGEROUS     =1<<17,  // object should be killed

    RADAR_REMOVE  =1<<18,  // object needs to be removed from radar
    CAN_DRIVE_ON  =1<<19,  // if objects can drive over / stand on you
    ON_WATER      =1<<20,  // if objects is on water
  };

  enum { SAVE_FLAGS= SELECTED | THINKING };

  w32 flags;
  int get_flag(int x) const { return flags & x; }
  void set_flag(int x, int value) { if (value) flags|=x; else flags&=(~x); }  
  i4_bool selected() const { return flags & SELECTED; }
  i4_bool out_of_bounds(i4_float x, i4_float y) const;
  i4_bool moved() const { return (lx!=x || ly!=y); }
  i4_bool valid() const { return get_flag(MAP_OCCUPIED)!=0; }

  g1_object_definition_class *get_type();     // inlined below

  virtual void mark_as_selected();  
  virtual void mark_as_unselected();

  virtual i4_float occupancy_radius() const;

  // called when the object is on a map cell that is about to be drawn
  virtual void predraw() {}

  // called when the object is on a map cell that is drawn
  virtual void draw(g1_draw_context_class *context);

  // called for each object after everything else has been drawn and in editor mode
  virtual void editor_draw(g1_draw_context_class *context);

  virtual void note_stank_near(g1_player_piece_class *s) { ; }

  // called every game tick
  virtual void think() = 0;
  virtual void post_think() {;}

  g1_object_class(g1_object_type id, g1_loader_class *fp);
  virtual void validate() {}
  virtual void save(g1_saver_class *fp);

  void request_think();
  virtual void request_remove();

  // call to add object to a map (adds to cell x,y where object is standing)
  virtual i4_bool occupy_location();

  // call to remove an object from a map
  virtual void unoccupy_location();

  virtual i4_bool deploy_to(float x, float y) { return i4_F; }

  virtual void damage(g1_object_class *who_is_hurting,
                      int how_much_hurt, i4_3d_vector damage_dir);

  virtual i4_bool check_collision(const i4_3d_vector &start, 
                                  i4_3d_vector &ray);

  enum {NOTIFY_DAMAGE_KILLED=1};

  virtual void notify_damage(g1_object_class *obj, sw32 hp) {}
  
  virtual void calc_world_transform(i4_float ratio, i4_transform_class *transform=0);
  // calculates the transform from object coordinates to the world and stores it in 
  //   'transform'.  if transform is null, it stores it into the object's internal storage

  i4_transform_class *world_transform; //calculated and linearly allocated at draw time

  g1_model_draw_parameters draw_params;
  g1_mini_object *mini_objects;
  w16 num_mini_objects;

  void allocate_mini_objects(int num,char *reason)
  {
    mini_objects = (g1_mini_object *)i4_malloc(sizeof(g1_mini_object)*num,reason);
    memset(mini_objects,0,sizeof(g1_mini_object) * num);
    num_mini_objects = num;
  }

  virtual void grab_old();     // grab info about the current tick for interpolation

  //  to be consistant, every g1_object should have an init
  virtual void init()  {}

  // show in editor if mouse cursor stays still on object
  virtual i4_str *get_context_string();  

  virtual ~g1_object_class();

  virtual i4_bool can_attack(g1_object_class *who) const { return i4_F; }

  virtual void stop_thinking();

  float height_above_ground();      // calls map->terrain_height

  const char *name() const;
  
  virtual li_object *message(li_symbol *message_name,
                             li_object *message_params, 
                             li_environment *env) { return 0; }

  // called for every object when you click 'ok' in the vars edit dialog for an object
  virtual void object_changed_by_editor(g1_object_class *who, li_class *old_values) {}
  virtual int get_chunk_names(char **&list) { return 0; }
};


inline g1_object_chain_class *g1_object_chain_class::next_solid()
{
  if (!next || !next->object->get_flag(g1_object_class::BLOCKING))
    return 0;
  else return next;
}


enum eBuildError
{
  G1_BUILD_OK=0,                       // successful build
  G1_BUILD_WAIT,                       // needs more time to build
  G1_BUILD_TOO_EXPENSIVE,              // needs more money to build
  G1_BUILD_OCCUPIED,                   // space to build is occupied
  G1_BUILD_NO_SPACE,                   // no space to build
  G1_BUILD_INVALID_OBJECT,             // invalid object or object parameters
  G1_BUILD_ALREADY_EXISTS,             // unique object already exists
  G1_BUILD_PLAYBACK,                   // playback in progress, can't build manually
};

class g1_object_definition_class;

// this is the call to add a new object_type to the game.
g1_object_type g1_add_object_type(g1_object_definition_class *def);

// this is the call to find the object_type of a specified object
g1_object_type g1_get_object_type(const char *name);
g1_object_type g1_get_object_type(li_symbol *name);

// remove probably doesn't need to used during a normal game, but can be useful for
// adding and removing a dll during a running session instead of restarting the game
void g1_remove_object_type(g1_object_type type);


// this class defines an object to golgotha, primarly how to create such an object and
// load it from disk.  The object then has virtual functions that allow it do things like
// think, save, and draw.  Objects can be created at anytime in the game, and should be
// able to linked in through dlls.
class g1_object_definition_class
{
protected:
  typedef void (*function_type)(void);
  char *_name;
  function_type init_function, uninit_function;
  
  g1_damage_map_struct          *damage;     // loaded from scheme/balance.scm

public:
  enum 
  {
    EDITOR_SELECTABLE = (1<<0),      // object shows up in editor
    DELETE_WITH_LEVEL = (1<<1),      // delete this type with the map
    MOVABLE           = (1<<2),      // objects
    TO_MAP_PIECE      = (1<<3),
    TO_PLAYER_PIECE   = (1<<4),
    TO_DYNAMIC_OBJECT = (1<<5),
    TO_PATH_OBJECT    = (1<<6),
    TO_DECO_OBJECT    = (1<<7),
    HAS_ALPHA         = (1<<8)      // if object has alpha polys it will draw after non-alpha objects
  };

  
  w32                        var_class;   // class type to create for object's vars
  li_class                   *vars;       // variables specific to the type
  g1_object_defaults_struct  *defaults;   // loaded from scheme/balance.scm
  
  g1_damage_map_struct        *get_damage_map();
  
  w32 flags;
  int get_flag(int x) const { return flags & x; }
  void set_flag(int x, int value) { if (value) flags|=x; else flags&=(~x); }  

  g1_object_type type;

  g1_object_definition_class(char *_name,
                             w32 type_flags = EDITOR_SELECTABLE,
                             function_type _init = 0,
                             function_type _uninit = 0);

  virtual ~g1_object_definition_class() { g1_remove_object_type(type); }

  // create_object should return a new initialized instance of an object, if fp is null then
  // default values should be supplied, otherwise the object should load itself from the file
  virtual g1_object_class *create_object(g1_object_type id,
                                         g1_loader_class *fp) = 0;

  // the name on an object should be unique, so you might want to be creative,  the name
  // is used to match up the object types in save files since object type id's are dynamically
  // created
  const char *name() { return _name; }

  i4_bool editor_selectable() { return flags & EDITOR_SELECTABLE; }

  virtual void init();
  virtual void uninit() { if (uninit_function) (*uninit_function)();}
  
  virtual void save(g1_saver_class *fp) { ; }  // save info about type
  virtual void load(g1_loader_class *fp) { ; } // load info about type

  // this is called when the object is right-clicked, the dialog is added to a draggable frame
  virtual i4_window_class *create_edit_dialog();

};

void g1_apply_damage(g1_object_class *kinda_gun_being_used,
                     g1_object_class *who_pulled_the_trigger,
                     g1_object_class *whos_on_the_wrong_side_of_the_gun,
                     const i4_3d_vector &direction_hurten_is_commin_from);

// increase this number if you change the load/save structure of g1_object_class
#define G1_OBJECT_FORMAT_VERSION 1 


// this table has an array of pointers to object definitions
// this is used by the border frame to find object with build info so it 
// can add buttons for them
extern g1_object_definition_class *g1_object_type_array[G1_MAX_OBJECT_TYPES];
extern g1_object_type g1_last_object_type;  // largest object number assigned

// this is prefered way to create new object in the game
inline g1_object_class *g1_create_object(g1_object_type type)
{ 
  if (g1_object_type_array[type])
    return g1_object_type_array[type]->create_object(type, 0);
  else 
    return 0;
}

inline g1_object_definition_class *g1_object_class::get_type() { return g1_object_type_array[id]; }

void g1_initialize_loaded_objects(); 
void g1_uninitialize_loaded_objects();

#endif


