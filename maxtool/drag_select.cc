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
#include "st_edit.hh"
#include "app/app.hh"

class m1_drag_select_class : public m1_utility_state_class
{
public:
  i4_float ax,ay;
  i4_float px,py;

  virtual i4_bool mouse_down()
  {
    ax = mouse_x();
    ay = mouse_y();

    return i4_F;
  }

  virtual i4_bool mouse_up()
  {
    g1_quad_object_class *obj = m1_info.obj;
    g1_vert_class *src_vert = obj->get_verts(m1_info.current_animation, m1_info.current_frame);
    r1_vert v;
    i4_float tmp;

    px = mouse_x();
    py = mouse_y();

    if (ax>px) { tmp=ax; ax=px; px=tmp; }
    if (ay>py) { tmp=ay; ay=py; py=tmp; }
    
    if (!i4_current_app->get_window_manager()->shift_pressed())
      li_call("select_none");

    for (int i=0; i<obj->num_quad; i++)
    {
      g1_quad_class *q = &obj->quad[i];
      i4_bool out=i4_F;
      for (int j=0; j<q->num_verts() && !out; j++)
      {
        m1_render_window->project_point(src_vert[q->vertex_ref[j]].v, v);
        if (v.px<ax || v.px>px || v.py<ay || v.py>py)
          out = i4_T;
      }
      if (!out)
        q->set_flags(g1_quad_class::SELECTED);
    }
    m1_render_window->restore_state();
    m1_render_window->request_redraw(i4_F);
    m1_st_edit->edit_poly_changed();

    return i4_T;
  }

  virtual i4_bool mouse_drag()
  {
    return i4_F;
  }
};

static m1_drag_select_class drag_select;

li_object *m1_drag_select(li_object *o, li_environment *env)
{
  if (m1_info.obj)
    m1_render_window->set_state(&drag_select);

  return 0;
}

li_automatic_add_function(m1_drag_select, "drag_select");
