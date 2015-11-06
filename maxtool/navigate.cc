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
#include "math/pi.hh"
#include "app/app.hh"
#include "st_edit.hh"

class m1_navigate_state_class : public m1_utility_state_class
{
public:
  enum 
  // Grab Masks for functions
  {
    moving    = m1_utility_window_class::LEFT,
    zooming   = m1_utility_window_class::RIGHT,
    panning   = m1_utility_window_class::MIDDLE
  };
  
  virtual i4_bool mouse_down()
  {
    if (buttons() & moving)
    {
      if (!i4_current_app->get_window_manager()->shift_pressed() && m1_info.preselect_poly>=0)
        li_call("select_none");
      m1_render_window->select_poly(m1_info.preselect_poly);
      m1_st_edit->select_point(m1_info.preselect_point);

      return i4_T;
    }

    return i4_F;
  }

  virtual i4_bool mouse_drag()
  {
    //{{{ Handle Moving
    if (buttons() & moving)
    {
      i4_float theta = m1_render_window->theta + 0.01*(mouse_x() - last_x());
      i4_float phi = m1_render_window->phi - 0.01*(mouse_y() - last_y());
      
      if (theta<0.0)
        theta += i4_pi()*2;
      else if (theta>i4_pi()*2)
        theta -= i4_pi()*2;
      
      if (phi<0.0)
        phi += i4_pi()*2;
      else if (phi>i4_pi()*2)
        phi -= i4_pi()*2;
      
      m1_render_window->theta = theta;
      m1_render_window->phi = phi;
      m1_render_window->recalc_view();

      return i4_T;
    }
    //}}}

    //{{{ Handle Zooming
    if (buttons() & zooming)
    {
      m1_render_window->dist *= exp(i4_float(mouse_y() - last_y())/height() * log(4));
      
      m1_render_window->recalc_view();
      return i4_T;
    }
    //}}}

    //{{{ Handle Panning
    if (buttons() & zooming)
    {
      i4_float p_x = i4_float(mouse_x() - last_x())/width();
      i4_float p_y = i4_float(mouse_y() - last_y())/height();
      
      m1_render_window->pan(p_x,p_y);
      m1_render_window->recalc_view();
      return i4_T;
    }
    //}}}

    return i4_F;
  }
};

m1_navigate_state_class navigate;
m1_utility_state_class *m1_default_state = &navigate;

li_object *m1_navigate(li_object *o, li_environment *env)
//{{{
{
  m1_render_window->set_state(&navigate);
  return 0;
}
//}}}

li_automatic_add_function(m1_navigate, "navigate");

//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}
