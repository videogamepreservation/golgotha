/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "g1_object.hh"
#include "object_definer.hh"
#include "lisp/li_class.hh"
#include "controller.hh"
#include "lisp/li_init.hh"
#include "map.hh"

class g1_field_camera_object_class;



class g1_field_camera_object_class : public g1_object_class
{
public:
  g1_field_camera_object_class *next;
  
  g1_field_camera_object_class(g1_object_type id, g1_loader_class *fp)
    : g1_object_class(id, fp)
  {
    draw_params.setup("camera");   
  }
  
  void draw(g1_draw_context_class *context)
  {
    g1_editor_model_draw(this, draw_params, context);
  }
  
  void think() {;}  
  i4_bool occupy_location();  
  void unoccupy_location();

};

static i4_isl_list<g1_field_camera_object_class> g1_camera_list;

g1_object_definer<g1_field_camera_object_class> 
  g1_field_camera_object_def("field_camera", g1_object_definition_class::EDITOR_SELECTABLE);

i4_bool g1_field_camera_object_class::occupy_location()
{
  g1_camera_list.insert(*this);
  return g1_object_class::occupy_location();
}

void g1_field_camera_object_class::unoccupy_location()
{
  g1_camera_list.find_and_unlink(this);
  g1_object_class::unoccupy_location();
}

g1_object_class *g1_find_closest_field_camera(const i4_3d_vector &pos)
{
  i4_isl_list<g1_field_camera_object_class>::iterator i=g1_camera_list.begin();
  
  g1_field_camera_object_class *closest=0;
  float closest_dist=100000000;

  i4_transform_class t;

  
  for (; i!=g1_camera_list.end(); ++i)
  {
    // direction vector from object to camera
    i4_3d_vector v=i4_3d_vector(i->x-pos.x, i->y-pos.y, i->h-pos.z), cam_dir;
    float l=v.length();
    v/=l;                   // normalize the vector

    // get direction camera is facing
    i->calc_world_transform(1, &t);
    t.transform_3x3(i4_3d_vector(1,0,0), cam_dir);
    
    float dproduct=cam_dir.dot(v);

    // is the camera facing the object?
    if (dproduct<0)
    {
      if (l<closest_dist)
      {
        closest=&(*i);
        closest_dist=l;
      }
    }
  }

  return closest;
}
                            
li_string_class_member li_name("name");

g1_object_class *g1_find_named_camera(char *name)
{
  for (g1_field_camera_object_class *f=g1_camera_list.first(); f; f=f->next)
  {
    li_class_context context(f->vars);
    if (strcmp(li_name(), name)==0)
      return f;
  }

  return 0;
}


static li_object *place_camera(li_object *o, li_environment *env)
{
  if (g1_current_controller.get())
  {
    li_call("add_undo", li_make_list(new li_int(G1_MAP_OBJECTS)));

    g1_camera_info_struct *c=g1_current_controller->view.get_camera();
    g1_field_camera_object_class *fc=(g1_field_camera_object_class *)
      g1_create_object(g1_field_camera_object_def.type);
    if (fc)
    {
      fc->theta=c->ground_rotate;
      fc->pitch=c->horizon_rotate;
      fc->roll=0;
      fc->x=c->gx;
      fc->y=c->gy;
      fc->h=c->gz;
      fc->grab_old();


      fc->occupy_location();
    }

    li_call("redraw");
  }

  return 0;
}


li_automatic_add_function(place_camera, "place_camera");
