/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "lisp/li_init.hh"
#include "m1_info.hh"
#include "max_object.hh"
#include "render.hh"


class m1_translate_state_class : public m1_utility_state_class
{
public:
  virtual i4_bool mouse_drag()
  {
    g1_quad_object_class *obj=m1_info.obj;
    
    if (obj)
    {
      // get translation
      i4_3d_vector point,ray;
      i4_float px,py;
      
      px = (i4_float(last_x())-center_x())/(center_x()*scale_x());
      py = (i4_float(last_y())-center_y())/(center_y()*scale_y());
      m1_render_window->transform.inverse_transform(i4_3d_vector(px,py,1.0), point);
      
      px = (i4_float(mouse_x())-center_x())/(center_x()*scale_x());
      py = (i4_float(mouse_y())-center_y())/(center_y()*scale_y());
      m1_render_window->transform.inverse_transform(i4_3d_vector(px,py,1.0), ray);
      ray -= point;
      
      g1_vert_class *src_vert = obj->get_verts(m1_info.current_animation, m1_info.current_frame);
      
      for (int i=0; i<obj->num_vertex; i++)
      {
        if (i==m1_info.preselect_point)
          src_vert[i].v += ray;
      }
      return i4_T;
    }
    return i4_F;
  }

};

static m1_translate_state_class translate;

li_object *m1_translate(li_object *o, li_environment *env)
{
  if (m1_info.obj && m1_info.preselect_point)
    m1_render_window->set_state(&translate);

  return 0;
}

li_automatic_add_function(m1_translate, "translate_point");
