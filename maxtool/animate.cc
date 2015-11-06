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
#include "math/pi.hh"

li_object *m1_frame_add(li_object *o, li_environment *env)
//{{{
{
  m1_poly_object_class *obj = m1_info.obj;
  obj->add_frame(m1_info.current_animation, m1_info.current_frame);
  m1_render_window->request_redraw();
  return 0;
}
//}}}

li_object *m1_frame_remove(li_object *o, li_environment *env)
//{{{
{
  m1_poly_object_class *obj = m1_info.obj;
  obj->remove_frame(m1_info.current_animation, m1_info.current_frame);
  m1_render_window->request_redraw();
  return 0;
}
//}}}

li_object *m1_frame_rewind(li_object *o, li_environment *env)
//{{{
{
  m1_info.current_frame = 0;
  m1_info.time = 0;
  m1_render_window->update_object(m1_info.time);
  m1_st_edit->edit_poly_changed();
  m1_render_window->request_redraw();

  return 0;
}
//}}}

li_object *m1_frame_advance(li_object *o, li_environment *env)
//{{{
{
  m1_poly_object_class *obj = m1_info.obj;

  if (++m1_info.current_frame==obj->animation[m1_info.current_animation].num_frames)
    m1_info.current_frame = 0;

  m1_info.time += 1.0;
  m1_render_window->update_object(m1_info.time);
  m1_st_edit->edit_poly_changed();
  m1_render_window->request_redraw();

  return 0;
}
//}}}

li_object *m1_frame_back(li_object *o, li_environment *env)
//{{{
{
  m1_poly_object_class *obj = m1_info.obj;

  if (--m1_info.current_frame<0)
    m1_info.current_frame = obj->animation[m1_info.current_animation].num_frames - 1;

  m1_info.time -= 1.0;

  m1_render_window->update_object(m1_info.time);
  m1_st_edit->edit_poly_changed();
  m1_render_window->request_redraw();

  return 0;
}
//}}}

li_object *m1_toggle_animation(li_object *o, li_environment *env)
//{{{
{
  m1_render_window->set_animation(!m1_render_window->is_animating());
  m1_st_edit->edit_poly_changed();
  return 0;
}
//}}}

li_automatic_add_function(m1_frame_add, "frame_add");
li_automatic_add_function(m1_frame_remove, "frame_remove");
li_automatic_add_function(m1_frame_rewind, "frame_rewind");
li_automatic_add_function(m1_frame_advance, "frame_advance");
li_automatic_add_function(m1_frame_back, "frame_back");
li_automatic_add_function(m1_toggle_animation, "toggle_animation");

//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}
