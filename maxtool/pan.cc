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

class m1_pan_state_class : public m1_utility_state_class
{
public:
  virtual i4_bool mouse_drag()
  {
    i4_float p_x = i4_float(last_x() - mouse_x())/width();
    i4_float p_y = i4_float(last_y() - mouse_y())/height();
    
    m1_render_window->pan(p_x,p_y);
    m1_render_window->recalc_view();

    return i4_T;
  }

  virtual i4_bool mouse_up()
  {
    m1_render_window->restore_state();
    return i4_T;
  }
};

static m1_pan_state_class pan;

li_object *m1_pan(li_object *o, li_environment *env)
//{{{
{
  m1_render_window->set_state(&pan);
  return 0;
}
//}}}

li_automatic_add_function(m1_pan, "pan");

//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}
