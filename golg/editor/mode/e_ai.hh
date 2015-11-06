/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#error

#ifndef _E_AI_HH__
#define _E_AI_HH__


#include "editor/mode/e_mode.hh"

class g1_critical_graph_class;
class g1_ai_mode : public g1_mode_handler
{
  w32 sel_color, norm_color;
  i4_bool no_more_move_undos;

public:
  g1_critical_graph_class *get_graph();
  virtual state current_state();
  virtual void post_draw(i4_draw_context_class &context);
  g1_ai_mode(g1_controller_edit_class *c);

  void select_objects_in_area(sw32 x1, sw32 y1, sw32 x2, sw32 y2, select_modifier mod);
  i4_bool select_object(sw32 mx, sw32 my, i4_float &ox, i4_float &oy, i4_float &oz,
                        select_modifier mod);

  void move_selected(i4_float xc, i4_float yc, i4_float zc, 
                             sw32 mouse_x, sw32 mouse_y);

  void mouse_down();
  void delete_selected();
};




class g1_ai_params : public g1_mode_creator
{
public:
  char *name() { return "AI"; }

  enum minor_mode_type
  {
    ROTATE,
    ZOOM,
    MOVE,
    SELECT,
    CREATE,
    RECALC
  };

  i4_bool set_minor_mode(w8 m);

  void create_buttons(i4_parent_window_class *containter);

  g1_mode_handler *create_mode_handler(g1_controller_edit_class *c)
  { return new g1_ai_mode(c); }

} ;

extern g1_ai_params g1_e_ai;



#endif
