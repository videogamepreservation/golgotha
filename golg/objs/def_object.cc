/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "lisp/lisp.hh"
#include "objs/miscobjs.hh"
#include "g1_object.hh"
#include "lisp/li_class.hh"
#include "objs/model_id.hh"
#include "saver.hh"
#include "objs/model_draw.hh"
#include "objs/def_object.hh"
#include "li_objref.hh"
#include "lisp/li_vect.hh"
#include "lisp/li_dialog.hh"
#include "map_man.hh"
#include "map.hh"

static li_symbol *s_model_name=0, *s_mini_object=0,
  *s_offset=0, *s_position=0, *s_rotation=0,
  *s_object_flags=0, *s_type_flags=0;


static li_symbol *s_ofun[G1_F_TOTAL];


////////////////////  li_def_object setup code ///////////////////////////////////////

// these flag value are assigned ot their symbol names for easy ussage in script files
static w32 g1_oflag_values[]={g1_object_class::THINKING,
                              g1_object_class::SELECTABLE,
                              g1_object_class::TARGETABLE,
                              g1_object_class::GROUND,
                              g1_object_class::UNDERWATER,
                              g1_object_class::AERIAL,
                              g1_object_class::HIT_GROUND,
                              g1_object_class::HIT_UNDERWATER,
                              g1_object_class::HIT_AERIAL,
                              g1_object_class::BLOCKING,
                              g1_object_class::DANGEROUS,
                              g1_object_class::CAN_DRIVE_ON
};

static char *g1_oflag_names[]={"thinking",
                               "selectable",
                               "targetable",
                               "ground",
                               "underwater",
                               "aerial",
                               "hit_ground",
                               "hit_underwater",
                               "hit_aerial",
                               "blocking",
                               "dangerous",
                               "can_drive_on",
                               0};


static char *g1_ofun_names[G1_F_TOTAL]={"think",
                                         "damage",
                                         "notify_damage",
                                         "enter_range",
                                         "draw",
                                         "message",
                                         "deploy_to",
                                         "change_player_num",
                                         "occupy_location",
                                         "unoccupy_location"};

li_object *li_def_object(li_object *o, li_environment *env);
li_object *li_def_buildings(li_object *o, li_environment *env)
{
  for (; o; o=li_cdr(o,env))
    g1_create_deco_object(li_string::get(li_eval(li_car(o,env), env),env)->value());
  return 0;
}

class g1_create_object_init : public i4_init_class
{
public:  
  void init()
  {
    int i=0;
    for (char **a=g1_oflag_names; *a; a++)
      li_set_value(*a, new li_int(g1_oflag_values[i++]));

    for (i=0; i<G1_F_TOTAL; i++)
      s_ofun[i]=li_get_symbol(g1_ofun_names[i]);
    
    li_set_value("editor_selectable", new li_int(g1_object_definition_class::EDITOR_SELECTABLE));
    li_set_value("movable", new li_int(g1_object_definition_class::MOVABLE));


    li_add_function("def_object", li_def_object);
    li_add_function("def_buildings", li_def_buildings);
  }

} g1_create_object_init_instance;



struct g1_object_def_struct
{
  char *name;  
  char *model_name;
  i4_array<g1_mini_object_def> *minis;
  li_type_number var_class;
  w32 flags;
  
  g1_object_def_struct()
  {
    name=0;
    minis=0;
    var_class=0;
    flags=0;
  }  
} ;



/////////////////////////////  Dynamic object code ///////////////////////////////

void g1_mini_object_def::assign_to(g1_mini_object *m)
{
  m->offset=offset;
  m->x=m->lx=position.x;
  m->y=m->ly=position.y;
  m->h=m->lh=position.z;

  m->rotation=m->lrotation=rotation;
  m->defmodeltype=defmodeltype;
  m->frame=0;
  m->animation=0;
}

g1_dynamic_object_type_class::g1_dynamic_object_type_class(li_symbol *sym)
  : g1_object_definition_class(sym->name()->value()),
    minis(0,1)

{
  obj_flags=g1_object_class::CAN_DRIVE_ON;
  var_class=0;
  flags |=  TO_DYNAMIC_OBJECT;

  memset(&funs, 0, sizeof(funs));
}


li_object *g1_dynamic_object_class::message(li_symbol *name, li_object *params, li_environment *env)
{
  g1_dynamic_object_type_class *t=get_type();
  if (t->funs[G1_F_MESSAGE])
  {
    li_class_context context(vars);
    li_g1_ref _me(global_id);
      
    li_list *p=(li_list *)params;
    li_push(p, name);
    li_push(p, &_me);

    return t->funs[G1_F_MESSAGE](p , env);
  }
  return 0;
}


g1_draw_context_class *g1_draw_context;

// called when the object is on a map cell that is drawn
void g1_dynamic_object_class::draw(g1_draw_context_class *context)
{
  g1_dynamic_object_type_class *t=get_type();
  if (t->funs[G1_F_DRAW])
  {
    g1_draw_context=context;
    li_class_context context(vars);
    li_g1_ref _me(global_id);
    t->funs[G1_F_DRAW](li_make_list(&_me,0),0);
  }
  else
  {
    g1_model_draw_parameters mp;
    mp.model=get_type()->model;
    g1_model_draw(this, mp, context);
  }
}


// void g1_dynamic_object_class::note_enter_range(g1_object_class *who, g1_fire_range_type range)
// {
//   g1_dynamic_object_type_class *t=get_type();
//   if (t->funs[G1_F_ENTER_RANGE])
//   {
//     li_class_context context(vars);
//     t->funs[G1_F_ENTER_RANGE](li_make_list(new li_g1_ref(global_id),
//                                            new li_g1_ref(who->global_id),
//                                            new li_int(range), 0), 0);
//   }
// }

void g1_dynamic_object_class::damage(g1_object_class *obj, int hp, i4_3d_vector damage_dir)
{
  g1_dynamic_object_type_class *t=get_type();
  if (t->funs[G1_F_DAMAGE])
  {
    li_class_context context(vars);    
    t->funs[G1_F_DAMAGE](li_make_list(new li_g1_ref(global_id),
                                      new li_g1_ref(obj->global_id),
                                      new li_int(hp),
                                      new li_vect(damage_dir), 0),0);
  }
  else
    request_think();
}

void g1_dynamic_object_class::notify_damage(g1_object_class *obj, int hp, w32 notes)
{
  g1_dynamic_object_type_class *t=get_type();
  if (t->funs[G1_F_NOTIFY_DAMAGE])
  {
    li_class_context context(vars);    
    t->funs[G1_F_NOTIFY_DAMAGE](li_make_list(new li_g1_ref(global_id),
                                             new li_g1_ref(obj->global_id),
                                             new li_int(hp),
                                             new li_int(notes), 0),0);
  }
  else
    g1_object_class::notify_damage(obj, hp);
}

void g1_dynamic_object_class::think()
{
  g1_dynamic_object_type_class *t=get_type();
  if (t->funs[G1_F_THINK])
  {
    li_class_context context(vars);   
    t->funs[G1_F_THINK](li_make_list(new li_g1_ref(global_id), 0), 0);
  }
}


static li_float_class_member dest_x("dest_x"), dest_y("dest_y");

i4_bool g1_dynamic_object_class::deploy_to(float x, float y)
{
  g1_dynamic_object_type_class *t=get_type();
  li_class_context context(vars);

  if (t->funs[G1_F_DEPLOY_TO])
    t->funs[G1_F_DEPLOY_TO](li_make_list(new li_g1_ref(global_id),
                                         new li_float(x),
                                         new li_float(y),0), 0);
  else
  {
    if (vars && vars->member_offset(dest_x)!=-1)
    {
      dest_x()=x;
      dest_y()=y;
    }
  }
  return 0;
}



void g1_dynamic_object_class::change_player_num(int new_player)
{
  g1_dynamic_object_type_class *t=get_type();
  if (t->funs[G1_F_CHANGE_TEAMS])
  {
    li_class_context context(vars);
    t->funs[G1_F_CHANGE_TEAMS](li_make_list(new li_g1_ref(global_id),
                                            new li_int(new_player), 0), 0);
  }
  else
    g1_object_class::change_player_num(new_player);  
}

i4_bool g1_dynamic_object_class::occupy_location()
{
  g1_dynamic_object_type_class *t=get_type();
  if (t->funs[G1_F_OCCUPY_LOCATION])
  {
    li_class_context context(vars);

    if (t->funs[G1_F_OCCUPY_LOCATION](li_make_list(new li_g1_ref(global_id),0), 0))
      return i4_T;
    else return i4_F;
  }
  else
    return g1_object_class::occupy_location();  
}


void g1_dynamic_object_class::unoccupy_location()
{
  g1_dynamic_object_type_class *t=get_type();
  if (t->funs[G1_F_UNOCCUPY_LOCATION])
  {
    li_class_context context(vars);
    t->funs[G1_F_UNOCCUPY_LOCATION](li_make_list(new li_g1_ref(global_id), 0), 0);
  }
  else
    g1_object_class::unoccupy_location();  
}


g1_dynamic_object_class::g1_dynamic_object_class(g1_object_type type, g1_loader_class *fp)
  : g1_object_class(type, fp)

{
  g1_dynamic_object_type_class *dtype=get_type();
       
  if (dtype->minis.size())
  {
    allocate_mini_objects(dtype->minis.size(), "");
    for (int i=0; i<dtype->minis.size(); i++)
      dtype->minis[i].assign_to(mini_objects+i);     
  }


  if (fp && fp->read_16()==VERSION)
  {
    int old_minis=fp->read_8(), i;
    int old_mini_disk_size=fp->read_32();
      
    if (old_minis==dtype->minis.size())
    {
      g1_mini_object *m=mini_objects;

      for (i=0; i<dtype->minis.size(); i++)
      {
        fp->read_format("fffffffff", 
                        &m->offset.x, &m->offset.y, &m->offset.z,
                        &m->x, &m->y, &m->h,
                        &m->rotation.x, &m->rotation.y, &m->rotation.z);
      }
    }
    else
      fp->seek(fp->tell() + old_mini_disk_size);
  }


  flags = get_type()->obj_flags;

  grab_old();
}


void g1_dynamic_object_class::save(g1_saver_class *fp)
{
  g1_object_class::save(fp);

  fp->write_16(VERSION);

  fp->write_8(num_mini_objects);
  int han=fp->mark_size();

  g1_mini_object *m=mini_objects;
  for (int i=0; i<num_mini_objects; i++)
  {      
    fp->write_format("fffffffff", 
                     &m->offset.x, &m->offset.y, &m->offset.z,
                     &m->x, &m->y, &m->h,
                     &m->rotation.x, &m->rotation.y, &m->rotation.z);
  }

  fp->end_mark_size(han);
}




//////////////////  dynamic object type functions ///////////////////////////////

g1_object_class *g1_dynamic_object_type_class::create_object(g1_object_type type, 
                                                             g1_loader_class *fp)
{
  g1_object_class *o=new g1_dynamic_object_class(type, fp);   
  o->init();
  return o;
}

static void read_vect(i4_3d_vector &v, li_object *o, li_environment *env)
{
  v.x=li_get_float(li_eval(li_car(o,env), env),env);  o=li_cdr(o,env);
  v.y=li_get_float(li_eval(li_car(o,env), env),env);  o=li_cdr(o,env);
  v.z=li_get_float(li_eval(li_car(o,env), env),env);
}


static li_function_type get_function(li_object *o, li_environment *env)
{
  li_object *fun=li_get_fun(li_symbol::get(o,env),env);
  if (!fun)
    li_error(env, "no registered function %O", o);

  return li_function::get(fun,env)->value();
}

li_object *li_def_object(li_object *o, li_environment *env)
{
  li_symbol *sym=li_symbol::get(li_car(o,env),env);
  

  g1_dynamic_object_type_class *type=new g1_dynamic_object_type_class(sym); 


  for (li_object *l=li_cdr(o,env); l; l=li_cdr(l,env))
  {
    li_object *prop=li_car(l,env);
    li_symbol *sym=li_symbol::get(li_car(prop,env),env);
    prop=li_cdr(prop,env);

    if (sym==li_get_symbol("model_name", s_model_name))
    {
      int id=g1_model_list_man.find_handle(li_string::get(li_car(prop,env),env)->value());
      type->model=g1_model_list_man.get_model(id);
    }
    else if (sym==li_get_symbol("mini_object", s_mini_object))
    {
      g1_mini_object_def mo;
      mo.init();
          
      char *name=0;
      for (;prop; prop=li_cdr(prop,env)) // prop = ((offset 0 0 0.1) (center 0 0 0) "gunport_barrel")
      {
        li_object *sub=li_car(prop,env);
        sym=li_symbol::get(li_car(sub,env),env); sub=li_cdr(sub,env);


        if (sym==li_get_symbol("offset", s_offset))
          read_vect(mo.offset, sub, env);
        else if (sym==li_get_symbol("position", s_position))
          read_vect(mo.position, sub, env);
        else if (sym==li_get_symbol("rotation", s_rotation))
          read_vect(mo.rotation, sub, env);
        else if (sym==li_get_symbol("model_name", s_model_name))
        {
          char *n=li_string::get(li_eval(li_car(sub,env), env),env)->value();
          mo.defmodeltype=g1_model_list_man.find_handle(n);
        }
        else li_error(env,"%O should be (offset/position/rotation x y z)",sym);
      }

      type->minis.add(mo);

    }
    else if (sym==li_get_symbol("object_flags", s_object_flags))
    {
      for (;prop; prop=li_cdr(prop,env))
        type->obj_flags |= li_int::get(li_eval(li_car(prop,env),env),env)->value();
    }
    else if (sym==li_get_symbol("type_flags", s_type_flags))
    {
      for (;prop; prop=li_cdr(prop,env))
        type->flags |= li_int::get(li_eval(li_car(prop,env),env),env)->value();
    }
    else
    {
      int found=0;
      for (int i=0; i<G1_F_TOTAL; i++)
        if (sym==s_ofun[i])
        {
          type->funs[i]=get_function(li_car(prop,env),env);
          found=1;
        }

      if (!found)
        li_error(env,"unknown object property %O", sym);
    }      
  }   

  // if functions were not filled in see if we can find defaults
  for (int i=0; i<G1_F_TOTAL; i++)
    if (type->funs[i]==0)
    {
      char buf[200];
      sprintf(buf, "%s_%s", type->name(), g1_ofun_names[i]);
      
      li_symbol *sym=li_find_symbol(buf);
      if (sym)
      {
        li_function *fun=li_function::get(sym->fun(), env);
        if (fun)
          type->funs[i]=li_function::get(fun, env)->value();
      }
    }


  type->flags|=g1_object_definition_class::DELETE_WITH_LEVEL;
 
  return 0;
}



