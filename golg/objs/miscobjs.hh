/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_MISCOBJS_HH
#define G1_MISCOBJS_HH

#include "g1_object.hh"
#include "lisp/li_types.hh"
#include "objs/structure_death.hh"

g1_object_type g1_create_deco_object(char *name);

class g1_deco_definition_class;

class g1_deco_type_manager_class : public i4_init_class
{
  i4_array<g1_deco_definition_class *> deco_objs;
public:
  g1_deco_type_manager_class() : deco_objs(0,32) {}
  void uninit();

  const char *find_name(const char *name);
  void add_type(g1_deco_definition_class *def)
  {
    int i;
    for (i=0; i<deco_objs.size() && deco_objs[i]; i++) ;

    if (i==deco_objs.size())
      deco_objs.add(def);
    else
      deco_objs[i] = def;
  }
  
  void remove_type(g1_deco_definition_class *def)
  {
    for (int i=0; i<deco_objs.size(); i++)
      if (deco_objs[i]==def)
        deco_objs[i]=0;
  }
};

extern g1_deco_type_manager_class g1_deco_type_manager;

class g1_deco_object_class : public g1_object_class
{
protected:
  g1_structure_death_class death;
public:
  sw16               health;         // the health its got left
  w8 collision_type;

  enum { DATA_VERSION=2 };
  const char *model_name;

  static g1_deco_object_class *cast(g1_object_class *o) 
  { 
    if (o->get_type()->get_flag(g1_object_definition_class::TO_DECO_OBJECT))
      return (g1_deco_object_class *)o;
    else return 0;
  }

  g1_deco_object_class(g1_object_type id, g1_loader_class *fp);

  virtual i4_bool check_collision(const i4_3d_vector &start, 
                          i4_3d_vector &ray);
  
  void save(g1_saver_class *fp);
    
//   void occupy_location()
//   {
//     h=g1_get_map()->terrain_height(x,y);
//     lh=h;
//     g1_object_class::occupy_location();
//   }


  virtual i4_bool occupy_location()
  {
    if (occupancy_radius()<0.6)
      return occupy_location_corners();
    else
      return occupy_location_model(draw_params);
  }

  virtual void think();

  virtual void damage(g1_object_class *who_is_hurting,
                      int how_much_hurt, i4_3d_vector damage_dir);
};

#endif
