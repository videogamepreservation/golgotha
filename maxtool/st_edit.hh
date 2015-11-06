/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef ST_EDIT_HH
#define ST_EDIT_HH

#include "window/window.hh"
#include "device/device.hh"
#include "string/string.hh"
#include "math/num_type.hh"

class i4_image_class;
class i4_text_input_class;

class m1_st_edit_window_class : public i4_window_class
{
private:
  enum { LEFT = 1, RIGHT = 2, MIDDLE = 3 };
  int grab;
  int last_x, last_y, last_but;

  i4_image_class *texture;
  int preselect_x, preselect_y;
  int snap_off_x, snap_off_y;

  i4_bool verts_are_selected();
  i4_bool dragging;
  i4_text_input_class *tname_edit;
  void get_point(int poly, int num, int &x, int &y);

  void drag_points(int xc, int yc);

  i4_float twidth() const;
  i4_float theight() const;
public:
  void edit_poly_changed();
  m1_st_edit_window_class(w16 w , w16 h, i4_text_input_class *tname_edit);
  void draw(i4_draw_context_class &context);
  void receive_event(i4_event *ev);
  void change_current_texture(i4_const_str new_name);
  void change_current_verts();
  void select_point(int point);

  char *name() { return "texture coord editor"; }
};

extern i4_event_handler_reference_class<m1_st_edit_window_class> m1_st_edit;


#endif
