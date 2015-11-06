/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "g1_object.hh"
#include "obj3d.hh"
#include "load3d.hh"
#include "error/alert.hh"
#include "memory/malloc.hh"
#include "objs/model_id.hh"
#include "error/error.hh"
#include "math/angle.hh"
#include "saver.hh"
#include "map.hh"
#include "map_man.hh"
#include "map_cell.hh"
#include "m_flow.hh"
#include "light.hh"
#include "objs/map_piece.hh"
#include "g1_render.hh"
#include "objs/stank.hh"
#include "player.hh"
#include "lisp/li_class.hh"
#include "lisp/li_dialog.hh"
#include "lisp/li_load.hh"
#include "lisp/li_init.hh"
#include "file/ram_file.hh"
#include "li_objref.hh"
#include "objs/damager.hh"
#include "time/profile.hh"
#include "controller.hh"
#include "objs/explode_model.hh"
#include "sound/sfx_id.hh"
#include "objs/explosion1.hh"
#include "objs/chunk_explosion.hh"
#include "math/random.hh"
#include "objs/model_collide.hh"
#include "resources.hh"
#include "objs/crate.hh"
#include "g1_rand.hh"

enum { 
  OLD_DATA_VERSION_2=2,
  OLD_DATA_VERSION_3,
  OLD_DATA_VERSION_4,          // includes movie path follow info
  OLD_DATA_VERSION_5,          // "improved" occupied_squares handling
  OLD_DATA_VERSION_6,          // doesn't save last position or movie path, saves flags as w32
  OLD_DATA_VERSION_7,          // saves global_id
  OLD_DATA_VERSION_8,          // saves 'vars'
  DATA_VERSION                 // saves 'health'
};

S1_SFX(explosion_ground_vehicle, "explosion/ground_vehicle.wav", S1_3D, 20);

static li_symbol_ref vortex("vortex"), explosion1("explosion1");
static li_symbol_ref explode_model("explode_model"), chunk_explosion("chunk_explosion");
static g1_explode_params def_params;

li_symbol_ref g1_crate("crate");
i4_profile_class   pf_unoccupy_location("unoccupy_location");

g1_object_type g1_last_object_type=-1;  // largest object number assigned
g1_object_definition_class *g1_object_type_array[G1_MAX_OBJECT_TYPES];
// registered object definitions (starts at 1, since 0 is used for invalid entries)




void g1_object_class::draw(g1_draw_context_class *context)
{
  g1_model_draw(this, draw_params, context);
}


i4_float g1_object_class::occupancy_radius() const
{
  if (draw_params.model)
    return draw_params.model->extent;
  else
    return 0;
}

i4_window_class *g1_object_definition_class::create_edit_dialog()
{
  w32 selected_objects[G1_MAX_OBJECTS];
  int t_sel=g1_get_map()->make_selected_objects_list(selected_objects, G1_MAX_OBJECTS);

  if (t_sel==1)
  {
    g1_object_class *go=g1_global_id.get(selected_objects[0]);

    if (go->vars && li_get_type(go->vars->type())->editor)
    {     
      li_function_type fun=li_function::get(li_get_fun(li_get_symbol("object_changed"),0),0)->value();

      return new li_dialog_window_class(go->name(),
                                        go->vars,
                                        0,fun,0);
    }
  }

  return 0;
}

g1_damage_map_struct *g1_object_definition_class::get_damage_map()
{
  if (!damage)
  {
    damage=g1_find_damage_map(type);
    if (!damage)
    {
      i4_warning("damage map missing %s, using default", name());
      damage=g1_find_damage_map(0);
      if (!damage)
        i4_error("no default");
    }
  }
  
  return damage;
}

g1_object_definition_class::g1_object_definition_class(char *_name,
                                                       w32 type_flags,
                                                       function_type _init,
                                                       function_type _uninit)
  : _name(_name),
    init_function(_init), uninit_function(_uninit)
{
  flags=type_flags;

  type = g1_add_object_type(this); 
  vars=0;
  var_class=0;
  damage=0;
}

void g1_object_definition_class::init() 
{ 
  char buf[200];
  sprintf(buf,"%s_vars", _name);
  if (li_find_symbol(buf))
    var_class=li_find_type(buf, 0);
  else if (flags & TO_MAP_PIECE) 
    var_class=li_find_type("map_piece_vars");
  else
    var_class=0;

  sprintf(buf,"%s_type_vars", _name);
  if (li_find_symbol(buf))
  {
    int type_vars_type=li_find_type(buf);
    if (type_vars_type)
      vars=li_class::get(li_get_type(type_vars_type)->create(0,0),0);
    else
      vars=0;
  }
  else vars=0;

  defaults=g1_get_object_defaults(_name, i4_F);
  
  if (init_function) 
    (*init_function)();

}

const char *g1_object_class::name() const
{
  return g1_object_type_array[id]->name();
}


i4_bool g1_object_class::out_of_bounds(i4_float x, i4_float y) const
{
  return (x<0 || x>=g1_get_map()->width() || y<0 || y>=g1_get_map()->height());
}


void g1_object_class::change_player_num(int new_player)
{
  if (new_player!=player_num)
  {
    g1_player_man.get(player_num)->remove_object(global_id);
    player_num=new_player;
    g1_player_man.get(new_player)->add_object(global_id);
  }
}

g1_team_type g1_object_class::get_team() const
{
  return g1_player_man.get(player_num)->get_team();
}


void g1_object_class::grab_old()     // grab info about the current tick for interpolation
{
  lx=x;
  ly=y;
  lh=h;
  ltheta = theta;
  lpitch = pitch;
  lroll  = roll;
  for (w32 i=0;i<num_mini_objects;i++) 
    mini_objects[i].grab_old();
}


void g1_object_class::save(g1_saver_class *fp)
{
  fp->start_version(DATA_VERSION);

  li_save_object(fp, vars,0);

  if (!fp->write_global_id(global_id))
    i4_warning("I'm invalid!?!?!  Help me!!!!\n");

  w32 save_flags = flags & SAVE_FLAGS;
  fp->write_format("ffffff421",
                   &x,&y,&h, 
                   &theta, &pitch, &roll,
                   &save_flags, &health, &player_num);
  fp->end_version();
}


void g1_object_class::load_v9(g1_loader_class *fp)
{
  vars=(li_class *)li_load_typed_object(get_type()->var_class, fp, fp->li_remap,0);
  global_id = fp->read_global_id();
  g1_global_id.assign(global_id, this);

  fp->read_format("ffffff421",
                  &x,&y,&h, 
                  &theta, &pitch, &roll,
                  &flags, &health, &player_num);

#if 0
  //(OLI) health limiting hack for dave
  if (health>get_type()->defaults->health)
    health=get_type()->defaults->health;
#endif

  g1_object_class::grab_old();
}


void g1_object_class::load_v8(g1_loader_class *fp)
{
  vars=(li_class *)li_load_typed_object(get_type()->var_class, fp, fp->li_remap,0);
  load_v7(fp);
}

void g1_object_class::load_v7(g1_loader_class *fp)
{
  global_id = fp->read_global_id();
  g1_global_id.assign(global_id, this);

  x=lx=fp->read_float();
  y=ly=fp->read_float();
  h=lh=fp->read_float();
  theta=ltheta=fp->read_float();
  pitch=lpitch=fp->read_float();
  roll=lroll=fp->read_float();
  flags=fp->read_32();
  
  player_num=fp->read_8();   
}


void g1_object_class::load_v6(g1_loader_class *fp)
{
  global_id=g1_global_id.alloc(this);
  x=lx=fp->read_float();
  y=ly=fp->read_float();
  h=lh=fp->read_float();
  theta=ltheta=fp->read_float();
  pitch=lpitch=fp->read_float();
  roll=lroll=fp->read_float();
  flags=fp->read_32();
  
  player_num=fp->read_8();   
}


void g1_object_class::load_v5(g1_loader_class *fp)
{
  global_id=g1_global_id.alloc(this);
  fp->read_16();    // id - load removed... why was it there? -jc
  fp->read_16();    // draw_id -load removed.. why was it there? -jc


  fp->read_16();
  fp->read_16();
  
  x=fp->read_float();
  lx=fp->read_float();

  y=fp->read_float();
  ly=fp->read_float();

  h=fp->read_float();
  lh=fp->read_float();
  
  theta=fp->read_float();
  ltheta=fp->read_float();
      
  pitch  = fp->read_float();
  lpitch = fp->read_float();
  roll   = fp->read_float();
  lroll  = fp->read_float();
  
  player_num=fp->read_8();

  if (player_num >= G1_MAX_PLAYERS) 
    player_num = g1_default_player;


  flags=fp->read_8();

  w8 t_scene_refs=fp->read_8();
  if (t_scene_refs)
  {
    for (int i=0; i<t_scene_refs; i++)
    {
      fp->read_8();
      fp->read_8();
    }
  }
}

void g1_object_class::load_v4(g1_loader_class *fp)
{
  load_v3(fp);
  w8 t_scene_refs=fp->read_8();
  if (t_scene_refs)
  {
    for (int i=0; i<t_scene_refs; i++)
    {
      fp->read_8();
      fp->read_8();
    }
  }
}

void g1_object_class::load_v3(g1_loader_class *fp)
{
  global_id=g1_global_id.alloc(this);
  id=fp->read_16();
  fp->read_16();
  fp->read_16();
  fp->read_16();
  
  x=fp->read_float();
  lx=fp->read_float();

  y=fp->read_float();
  ly=fp->read_float();

  h=fp->read_float();
  lh=fp->read_float();
  
  theta=fp->read_float();
  ltheta=fp->read_float();
      
  pitch  = fp->read_float();
  lpitch = fp->read_float();
  roll   = fp->read_float();
  lroll  = fp->read_float();

  player_num=fp->read_8();
  if (player_num >= G1_MAX_PLAYERS) 
    player_num = g1_default_player;

  flags=fp->read_8();
  
  fp->read_16();
  fp->read_16();
  fp->read_16();
  fp->read_16();

}

void g1_object_class::load_v2(g1_loader_class *fp)
{
  global_id=g1_global_id.alloc(this);
  id=fp->read_16();
  fp->read_16();
  fp->read_16();
  fp->read_16();
  
  x=fp->read_float();
  lx=fp->read_float();

  y=fp->read_float();
  ly=fp->read_float();

  h=fp->read_float();
  lh=fp->read_float();
  
  theta=fp->read_float();
  ltheta=fp->read_float();


  //the up variable was removed
  fp->read_float();  // x
  fp->read_float();  // y
  fp->read_float();  // z

  pitch  = 0;
  lpitch = 0;
  roll   = 0;
  lroll  = 0;

  player_num=fp->read_8();
  if (player_num >= G1_MAX_PLAYERS) 
    player_num = g1_default_player;

  flags=fp->read_8();
  
  fp->read_16();
  fp->read_16();
  fp->read_16();
  fp->read_16();     
}


g1_object_class::g1_object_class(g1_object_type id,
                                 g1_loader_class *fp) 
  : id(id), occupied_squares(4,0), vars(0), world_transform(0),
    mini_objects(0), num_mini_objects(0)
{
  w16 ver=0;
  w16 data_size;

  radar_image=0;
  radar_type=G1_RADAR_NONE;

  if (fp) 
    fp->get_version(ver,data_size);

  //read in the correct version #
  switch (ver)
  {
    case DATA_VERSION:       load_v9(fp); break;  
    case OLD_DATA_VERSION_8: load_v8(fp); health=get_type()->defaults->health; break;  
    case OLD_DATA_VERSION_7: load_v7(fp); health=get_type()->defaults->health; break;  
    case OLD_DATA_VERSION_6: load_v6(fp); health=get_type()->defaults->health; break;  
    case OLD_DATA_VERSION_5: load_v5(fp); health=get_type()->defaults->health; break;  
    case OLD_DATA_VERSION_4: load_v4(fp); health=get_type()->defaults->health; break;
    case OLD_DATA_VERSION_3: load_v3(fp); health=get_type()->defaults->health; break;
    case OLD_DATA_VERSION_2: load_v2(fp); health=get_type()->defaults->health; break;
       
    default:
      //if the file has an unrecognized version, seek past it
      if (fp) fp->seek(fp->tell() + data_size);

      health=get_type()->defaults->health;
      global_id=g1_global_id.alloc(this);
      lx=x=g1_get_map()->width()/2;
      ly=y=g1_get_map()->height()/2;
      lh=h=0;
      ltheta=theta=0;
      player_num=g1_default_player;
      flags=0;        
      pitch = roll = lpitch = lroll = 0;
      break;
  }

  if (flags & SCRATCH_BIT)
    i4_error("object created with scratch bit set, shouldn't happen");

  if (fp) fp->end_version(I4_LF);

  //bad lvariable check
  if (fabs(lx-x)>1) lx=x;
  if (fabs(ly-y)>1) ly=y;
  if (fabs(lh-h)>1) lh=h;
  
  if (vars==0 && get_type()->var_class)
    vars=li_class::get(li_get_type(get_type()->var_class)->create(0,0),0);
}

void g1_object_class::request_think()
{
  if (get_flag(THINKING | DELETED)==0)
  {
    g1_get_map()->request_think(this);
    set_flag(THINKING,1);
  }
}

void g1_object_class::request_remove()
{
  g1_get_map()->request_remove(this);
}

i4_bool g1_object_class::occupy_location()
{
  if (get_flag(MAP_OCCUPIED))
    i4_error("occupy_location called twice");

  if (get_flag(DELETED))
    i4_error("occupy_location called on a deleted object!");
  
  if (occupancy_radius()<0.6)
    return occupy_location_corners();
  else
    return occupy_location_model(draw_params);
}

i4_bool g1_object_class::occupy_location_center()
{
  g1_map_class *map = g1_get_map();
  int width=map->width();

  if (!(x>=0 && x<width && y>=0 && y<map->height()))
  {
    request_remove();
    return i4_F;
  }

  map->add_object(*new_occupied_square(), i4_f_to_i(x), i4_f_to_i(y));  

  set_flag(MAP_OCCUPIED,1);

  return i4_T;
}

i4_bool g1_object_class::occupy_location_corners()
{
  g1_map_class *map = g1_get_map();
  int width=map->width();

  // standard occupy location (4 corners)
  sw32 x_left,x_right,y_top,y_bottom;
  
  if (!(x>=0 && x<width && y>=0 && y<map->height()))
  {
    request_remove();
    return i4_F;
  }

  i4_float rad = occupancy_radius();


  x_left   = i4_f_to_i(x-rad); if (x_left<0)                x_left=0;
  x_right  = i4_f_to_i(x+rad); if (x_right>=map->width())   x_right=map->width()-1;
  y_top    = i4_f_to_i(y-rad); if (y_top<0)                 y_top=0;
  y_bottom = i4_f_to_i(y+rad); if (y_bottom>=map->height()) y_bottom=map->height()-1;

  map->add_object(*new_occupied_square(), x_left,y_top);  

  if (x_right != x_left)
    map->add_object(*new_occupied_square(), x_right, y_top);

  if (y_top != y_bottom)
  {
    map->add_object(*new_occupied_square(), x_left, y_bottom);

    if (x_right != x_left)
      map->add_object(*new_occupied_square(), x_right, y_bottom);
  }

  set_flag(MAP_OCCUPIED,1);


  return i4_T;
}

i4_bool g1_object_class::occupy_location_model(const g1_model_draw_parameters &draw_params)
{
  g1_map_class *map = g1_get_map();
  int width=map->width();

  i4_transform_class trans;
  calc_world_transform(1.0, &trans);
  i4_3d_point_class p;
  int ix,iy, offset;
  int i,j;

  g1_quad_object_class *model = draw_params.model;
  if (!model)
    return g1_object_class::occupy_location_center();

  
  g1_vert_class *vert = model->get_verts(draw_params.animation, draw_params.frame);

  // large object occupy - occupy multiple points
  enum { MAX_SQUARES=50 };
  w16 sx[MAX_SQUARES], sy[MAX_SQUARES], num_squares=0;

  // build list of occupied squares
  for (i=0; i<model->num_vertex; i++)
  {
    trans.transform(vert->v, p);
    ix = int(p.x); iy = int(p.y);
    if (ix<0 || ix>=width || iy<0 || iy>=map->height())
    {
      // off the map, remove me!
      request_remove();
      return i4_F;
    }
    
    // check for repeats
    g1_map_cell_class *cell = map->cell(ix,iy);
    if ((cell->flags & g1_map_cell_class::SCRATCH1)==0)
    {
      // nonrepeating square, add it
      sx[num_squares]=ix; sy[num_squares]=iy;
      cell->flags |= g1_map_cell_class::SCRATCH1;
      num_squares++;
    }
    vert++;
  }

  // grow list to required size
  if (num_squares>occupied_squares.max_size())
    occupied_squares.resize(num_squares);

    // we assume that the occupied square list is empty, since unoccupy should'be been called

  for (i=0; i<num_squares; i++)
  {
    g1_object_chain_class *chn = new_occupied_square();
    map->add_object(*chn, sx[i], sy[i]);  
    map->cell(chn->offset)->flags &= ~g1_map_cell_class::SCRATCH1;
  }


  set_flag(MAP_OCCUPIED,1);


  return i4_T;
}



i4_bool g1_object_class::occupy_location_model_extents(const g1_model_draw_parameters &draw_params)
{
  g1_map_class *map = g1_get_map();
  int width=map->width();

  i4_transform_class trans;
  calc_world_transform(1.0, &trans);
  i4_3d_point_class p;

  g1_quad_object_class *model = draw_params.model;
  if (!model)
    return g1_object_class::occupy_location_center();

  
  g1_vert_class *vert = model->get_verts(draw_params.animation, draw_params.frame);
  float x1=10000,y1=10000,x2=-1,y2=-1;

  // build list of occupied squares
  for (int i=0; i<model->num_vertex; i++)
  {
    trans.transform(vert->v, p);
    if (p.x<x1) x1=p.x;
    if (p.y<y1) y1=p.y;
    if (p.x>x2) x2=p.x;
    if (p.y>y2) y2=p.y;
    vert++;
  }
  
  if (x2==-1)
  {
    request_remove();
    return i4_F;
  }

  int ix1=i4_f_to_i(x1), iy1=i4_f_to_i(y1),
      ix2=i4_f_to_i(x2), iy2=i4_f_to_i(y2);
  if (ix1<0) ix1=0;
  if (iy1<0) iy1=0;
  if (ix2>=map->width()) ix2=map->width()-1;
  if (iy2>=map->height()) iy2=map->height()-1;
  

  int num_squares=(ix2-ix1+1)*(iy2-iy1+1);

  // grow list to required size
  if (num_squares>occupied_squares.max_size())
    occupied_squares.resize(num_squares);

    // we assume that the occupied square list is empty, since unoccupy should'be been called

  for (int y=iy1; y<=iy2; y++)
    for (int x=ix1; x<=ix2; x++)
    {    
      g1_object_chain_class *chn = new_occupied_square();
      map->add_object(*chn, x,y);  
    }
  

  set_flag(MAP_OCCUPIED,1);


  return i4_T;
}


void g1_object_class::unoccupy_location()
{
  pf_unoccupy_location.start();
  
  g1_map_class *map = g1_get_map();

  if (!get_flag(MAP_OCCUPIED))
  {
    i4_warning("unoccupy_location called twice");
  }

  set_flag(MAP_OCCUPIED,0);

  I4_ASSERT(x>=0 && x<map->width() && y>=0 && y<map->height(), 
            "Object is off the map");

  for (int i=0; i<occupied_squares.size(); i++)
    map->remove_object(occupied_squares[i]);
  occupied_squares.clear();

  pf_unoccupy_location.stop();
}

//g1_mini_object stuff
void g1_mini_object::calc_transform(i4_float ratio, i4_transform_class *transform)
{  
  i4_3d_vector          rot, t;

  // interpolated position
  t.interpolate(i4_3d_vector(lx,ly,lh), i4_3d_vector(x,y,h), ratio);
  transform->translate(t);  

  // interpolated rotations
  rot.z = i4_interpolate_angle(lrotation.z,rotation.z,ratio);
  transform->mult_rotate_z(rot.z);

  rot.y = i4_interpolate_angle(lrotation.y,rotation.y,ratio);
  transform->mult_rotate_y(rot.y);

  rot.x = i4_interpolate_angle(lrotation.x,rotation.x,ratio);
  transform->mult_rotate_x(rot.x);

  // interpolated center offset
  t.interpolate(loffset, offset, ratio);
  transform->mult_translate(t);
}

inline void msvc50_hack(i4_transform_class &m1, i4_transform_class *m2, i4_transform_class *m3)
{
  m1.multiply((*m2),(*m3));
}

void g1_mini_object::draw(g1_draw_context_class *context,
                          i4_transform_class *parent_transform,
                          g1_screen_box *bound_box,
                          g1_player_type player_num,
                          i4_transform_class *use_this_transform,
                          i4_bool pass_world_space_transform,
                          i4_bool use_lod_model)
{
  if (use_lod_model && lod_model<=0)
    return;

  g1_quad_object_class *model;
  if (use_lod_model)
    model=g1_model_list_man.get_model(lod_model);
  else
    model=g1_model_list_man.get_model(defmodeltype);
  
  i4_transform_class local_transform;
  
  if (use_this_transform==0)
  {
    use_this_transform = &local_transform;
    calc_transform(g1_render.frame_ratio, use_this_transform);
  }
  
  i4_transform_class world_transform;
  
  msvc50_hack(world_transform, parent_transform, use_this_transform);

//world_transform.multiply(*parent_world_transform, *use_this_transform);
  i4_transform_class view_transform;

  msvc50_hack(view_transform, context->transform, use_this_transform);

//view_transform.multiply(*(context->transform),*use_this_transform);
  
  g1_render.render_object(model,
                          &view_transform,
                          pass_world_space_transform ? &world_transform : 0,
                          1,
                          player_num,
                          0,//_this->frame,
                          bound_box,
                          0);
}

void g1_object_class::mark_as_selected()
{
  set_flag(SELECTED,1);
}


void g1_object_class::mark_as_unselected()
{
  set_flag(SELECTED,0);
}


//note: must be power of 2
#define TRIG_TABLE_SIZE 1024

i4_float g1_cos_lookup[TRIG_TABLE_SIZE];
i4_float g1_sin_lookup[TRIG_TABLE_SIZE];

i4_bool g1_trig_table_init = i4_F;

const i4_float g1_trig_table_factor = (TRIG_TABLE_SIZE>>1) / 3.1415927f;
const i4_float g1_ootrig_table_factor = 3.1415927f / (TRIG_TABLE_SIZE>>1);

void g1_init_trig_tables()
{
  if (!g1_trig_table_init)
  {
    g1_trig_table_init = i4_T;
    w32 i;
    for (i=0;i<TRIG_TABLE_SIZE;i++)
    {
      g1_cos_lookup[i] = cos(i*g1_ootrig_table_factor);
      g1_sin_lookup[i] = sin(i*g1_ootrig_table_factor);
    }
  }
}

inline sw32 f_to_i(float x)
{  
  sw32 result;

#ifdef WIN32

  _asm {
    fld       x ;
    fistp     result ;
  }

#else
  
  result = (sw32)x;

#endif

  return result;
}

i4_transform_class& i4_transform_class::rotate_x_y_z(i4_float x_rot, i4_float y_rot, i4_float z_rot, i4_bool use_lookup)
{
  i4_float cos_x,sin_x,cos_y,sin_y,cos_z,sin_z;

  if (use_lookup)
  {
    g1_init_trig_tables();
    w32 x_l = (f_to_i(x_rot*g1_trig_table_factor)) & (TRIG_TABLE_SIZE-1);
    w32 y_l = (f_to_i(y_rot*g1_trig_table_factor)) & (TRIG_TABLE_SIZE-1);
    w32 z_l = (f_to_i(z_rot*g1_trig_table_factor)) & (TRIG_TABLE_SIZE-1);
  
    cos_x=g1_cos_lookup[x_l];
    sin_x=g1_sin_lookup[x_l];
    cos_y=g1_cos_lookup[y_l];
    sin_y=g1_sin_lookup[y_l];
    cos_z=g1_cos_lookup[z_l];
    sin_z=g1_sin_lookup[z_l];
  }
  else
  {
    cos_x=cos(x_rot);
    sin_x=sin(x_rot);
    cos_y=cos(y_rot);
    sin_y=sin(y_rot);
    cos_z=cos(z_rot);
    sin_z=sin(z_rot);
  }

  x.x = cos_z*cos_y;
  x.y = sin_z*cos_y;
  x.z = -sin_y;

  y.x = cos_z*sin_y*sin_x - sin_z*cos_x;
  y.y = sin_z*sin_y*sin_x + cos_z*cos_x;
  y.z = cos_y*sin_x;

  z.x = cos_z*sin_y*cos_x + sin_z*sin_x;
  z.y = sin_z*sin_y*cos_x - cos_z*sin_x;
  z.z = cos_y*cos_x;

  t.set (0,0,0);
  return *this;
}


float g1_object_class::height_above_ground()
{
  return h-g1_get_map()->terrain_height(x,y);
}

void g1_object_class::calc_world_transform(i4_float ratio, i4_transform_class *transform)
{
  if (!transform)
    transform = world_transform;

  i4_float z_rot = i4_interpolate_angle(ltheta,theta, ratio);
  i4_float y_rot = i4_interpolate_angle(lpitch,pitch, ratio);
  i4_float x_rot = i4_interpolate_angle(lroll ,roll , ratio);

  i4_3d_vector t;
  t.interpolate(i4_3d_vector(lx,ly,lh), i4_3d_vector(x,y,h), ratio);

  transform->translate(t);  
  transform->mult_rotate_z(z_rot);
  transform->mult_rotate_y(y_rot);
  transform->mult_rotate_x(x_rot);
} 

g1_object_class::~g1_object_class() 
{ 
  if (mini_objects) 
  {
    i4_free(mini_objects);
    mini_objects = 0;
  }
  
  g1_global_id.free(global_id);
}

// this is the call to add a new object_type to the game.
g1_object_type g1_add_object_type(g1_object_definition_class *def)
{
  for (int i=1; i<G1_MAX_OBJECT_TYPES; i++)
  {
    if (g1_object_type_array[i]==0)
    {
      if (i>g1_last_object_type)
        g1_last_object_type=i;

      g1_object_type_array[i]=def;
      return i;
    }
  }

  i4_error("Object type limit exceeded (%d)",G1_MAX_OBJECT_TYPES);
  return 0;
}

g1_object_type g1_get_object_type(const char *name)
{
  return g1_get_object_type(li_get_symbol(name)); 
}

g1_object_type g1_get_object_type(li_symbol *name)
{
  li_object *o=li_get_value(name);
  if (o)
    return li_int::get(o,0)->value();
  else
    return 0;
}

void g1_remove_object_type(g1_object_type type)
{
  I4_ASSERT(type!=0, "tried to remove invalid object");

  if (g1_map_is_loaded())
  {
    g1_map_class *map=g1_get_map();
    map->remove_object_type(type);
  }

  g1_object_type_array[type]=0;
  while (g1_last_object_type>=0 && g1_object_type_array[g1_last_object_type]==0)
    g1_last_object_type--;
}

void g1_initialize_loaded_objects()
{
  int i;
  
  for (i=1; i<G1_MAX_OBJECT_TYPES; i++)
    if (g1_object_type_array[i])
      li_set_value(li_get_symbol(g1_object_type_array[i]->name()), new li_int(i));
            
  for (i=1; i<G1_MAX_OBJECT_TYPES; i++)
  {
    if (g1_object_type_array[i])
      g1_object_type_array[i]->init();
  }
}


void g1_uninitialize_loaded_objects()
{
  for (int i=1; i<G1_MAX_OBJECT_TYPES; i++)
  {
    if (g1_object_type_array[i])
    {
      g1_object_type_array[i]->uninit();
      g1_object_type_array[i]=0;
    }
  }
}

i4_str *g1_object_class::get_context_string()
{
  char buf[101];
  i4_ram_file_class rf(buf,100);
  rf.printf("%s", name());
  if (vars)
    li_printf(&rf, " %O", vars);

  buf[rf.tell()]=0;
  return new i4_str(buf);
}

void g1_object_class::stop_thinking()
{
  if (get_flag(THINKING))
  {
    //clears this object out of the think que
    if (g1_get_map())
      g1_get_map()->remove_from_think_list(this);
    
    //clears the THINKING flag
    set_flag(THINKING,0);
  }
}

// will mark all the objects & object types in the game
class g1_object_marker_class : li_gc_object_marker_class
{
public:
  virtual void mark_objects(int set)
  {
    int i;

    for (i=0; i<=g1_last_object_type; i++)    
      if (g1_object_type_array[i])
      {
        li_class *v=g1_object_type_array[i]->vars;
        if (v)
        { 
          if (v->is_marked()!=set)
            li_get_type(v->unmarked_type())->mark(v, set);
        }
      }


    for (i=0; i<G1_MAX_OBJECTS; i++)
    {
      g1_object_class *o=g1_global_id.get(i);
      if (!g1_global_id.preassigned(i) && ((w32)o>G1_MAX_OBJECTS))
      {
        li_class *c=o->vars;

        if (c && c->is_marked()!=set)
          li_get_type(c->unmarked_type())->mark(c, set);
      }
    }

  }
} g1_object_marker;


void g1_object_class::editor_draw(g1_draw_context_class *context)
{
  if (vars)
  {
    w32 colors[]={0xff0000, 0x00ff00, 0x0000ff, 0xffff00, 0x00ffff, 0xff00ff, 0xffffff};
    int color_on=0;
    li_type_number type=vars->type();
    int tvars=li_class_total_members(type);

    for (int j=0; j<tvars; j++)
    {
      li_object *v=li_class_get_default(type, li_class_get_symbol(type,j));
      if (v && v->type()==li_g1_ref_type_number)
      {
        li_g1_ref::get(vars->value(j),0)->draw(this, colors[color_on], context);
        if (color_on<6) 
          color_on++;
      }
      else if (v && v->type()==li_g1_ref_list_type_number)
      {
        li_g1_ref_list *r=li_g1_ref_list::get(vars->value(j),0);
        r->draw(this, colors[color_on], context);                      
        if (color_on<6) 
          color_on++;
      }
    }
  }  
}


static li_symbol_ref acid("acid");

i4_bool g1_object_class::check_collision(const i4_3d_vector &start, 
                                         i4_3d_vector &ray)
{
  i4_3d_vector normal;
  if (occupancy_radius()<0.5)
    return g1_model_collide_radial(this, draw_params, start, ray);
  else
    return g1_model_collide_polygonal(this, draw_params, start, ray, normal);
}


void g1_apply_damage(g1_object_class *used,
                     g1_object_class *fired,
                     g1_object_class *hit,
                     const i4_3d_vector &dir)
{
  if (!used) return;

  g1_damage_map_struct *dmap=used->get_type()->get_damage_map();

  if (dmap->hurt_type==g1_damage_map_struct::HURT_SINGLE_GUY)
  {
    if (!hit || !hit->valid() || hit->player_num==used->player_num) return ;
    
    int amount=dmap->get_damage_for(hit->id);
    if (fired)
      fired->notify_damage(hit, amount);
    
    hit->damage(used, amount, dir);

    if (dmap->special_damage)
      g1_create_damager_object(i4_3d_vector(hit->x, hit->y, hit->h),
                               amount, dmap->special_damage, fired, hit,
                               acid.get());
  }
  else
  {
    // damage what we hit
    if (hit && hit->player_num!=used->player_num)
    {
      int amount=dmap->get_damage_for(hit->id);

      if (fired)
        fired->notify_damage(hit, amount);
      
      hit->damage(used, amount, dir);        
    }

    // damage things in the radius
    g1_object_class *list[G1_MAX_OBJECTS];
    
    float radius=dmap->special_damage;
    float oo_radius=1.0/radius;
    
    int t=g1_get_map()->get_objects_in_range(used->x, used->y,
                                             radius, list, G1_MAX_OBJECTS,
                                             g1_object_class::TARGETABLE);

    for (int i=0; i<t; i++)
    {
      g1_object_class *o=list[i];
      //      if (o->player_num!=used->player_num && o!=hit)  // hit only enemies
      if (o!=hit)          // smack up everyone
      {
        float dist=sqrt((used->x-o->x)*(used->x-o->x)+
                        (used->y-o->y)*(used->x-o->x)+
                        (used->h-o->h)*(used->h-o->h));
        
        if (dist<radius)
        {
          float max_amount=dmap->get_damage_for(o->id);
          float amount=(radius-dist)*oo_radius * max_amount;
          
          if (fired)
            fired->notify_damage(o, i4_f_to_i(amount));
          
          o->damage(used, i4_f_to_i(amount), dir);        
        }     
      }             
    }
  }                                   
}

void g1_object_class::damage(g1_object_class *who_is_hurting,
                             int how_much_hurt, i4_3d_vector damage_dir)
{
  if (!valid())
    return;
 
  if (health-how_much_hurt<=0)
  {          
    health=0;

    if (player_num!=g1_player_man.local_player)
    {
      g1_crate_class *c=(g1_crate_class *)g1_create_object(g1_get_object_type(g1_crate.get()));

      // jc fixme : this is not determanistic if global_id's are not

      if ((g1_rand(global_id) & 7)==0)
        c->setup(i4_3d_vector(x,y,h), 
                 (g1_crate_class::ctype)(g1_rand(global_id) % g1_crate_class::MAX_TYPES),
                 g1_crate_class::SMALL,
                 200);       // 20 seconds to get the crate
    }



    i4_3d_vector spot=i4_3d_vector(x, y, h);
    float r=g1_resources.visual_radius();

    // don't explode or play sound if we are too far away from the camera
    if (g1_current_view_state()->dist_sqrd(spot)<r*r)
    {
      if (who_is_hurting && who_is_hurting->id==g1_get_object_type(vortex.get()))
      {
         
        g1_explode_model_class *e;
        e=(g1_explode_model_class *)g1_create_object(g1_get_object_type(explode_model.get()));
        g1_explode_params params;
        i4_3d_vector e_pos=i4_3d_vector(who_is_hurting->x, who_is_hurting->y, who_is_hurting->h);
        
        params.stages[1].setup(5, 0);
        params.stages[2].setup(50, -0.1);
        params.t_stages=3;
        e->setup(this, e_pos, params);

        g1_camera_event cev;
        cev.type=G1_WATCH_EXPLOSION;
        cev.follow_object=e;
        g1_current_controller->view.suggest_camera_event(cev);
      }
      else
      {
        g1_chunk_explosion_class *ce;

        char **chunk_names;
        int t_chunks=get_chunk_names(chunk_names);
        if (!t_chunks)
        {
          ce=(g1_chunk_explosion_class *)g1_create_object(g1_get_object_type(chunk_explosion.get()));
          ce->setup(i4_3d_vector(x,y,h), 
                    i4_3d_vector(roll, pitch, theta),
                    draw_params.model, damage_dir*0.3, 1);
        }
        else
        {          
          for (int t=0; t<t_chunks; t++)
          {      
            ce=(g1_chunk_explosion_class *)g1_create_object(g1_get_object_type(chunk_explosion.get()));
            int model_id=g1_model_list_man.find_handle(chunk_names[t]);
            ce->setup(i4_3d_vector(x,y,h), 
                      i4_3d_vector(roll, pitch, theta),
                      g1_model_list_man.get_model(model_id), damage_dir, t*5+1);
          }
        }


        g1_camera_event cev;
        cev.type=G1_WATCH_EXPLOSION;
        cev.follow_object=ce;
        g1_current_controller->view.suggest_camera_event(cev);
      }

      explosion_ground_vehicle.play(x,y,h);
    }
    unoccupy_location();
    request_remove();
  }
  else
  {
    health-=how_much_hurt;

    g1_camera_event cev;
    cev.type=G1_WATCH_HIT;
    cev.follow_object=this;
    g1_current_controller->view.suggest_camera_event(cev);
  }
}


li_object *g1_setup_stages(li_object *o, li_environment *env)
{
  int t=0;
  while (o)
  {
    def_params.stages[t].setup(li_get_int(li_eval(li_first(o,env), env),env),
                               li_get_float(li_eval(li_second(o,env), env),env),
                               (g1_stage_type)(li_get_int(li_eval(li_third(o, env), env),env))
                               );
    t++;
    o=li_cdr(o,env);
    o=li_cdr(o,env);
    o=li_cdr(o,env);
  }
  
  def_params.t_stages=t;
  return 0;
}

li_automatic_add_function(g1_setup_stages, "stages");
