/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_PICK_WIN_HH
#define G1_PICK_WIN_HH

#include "window/window.hh"
#include "g1_limits.hh"
#include "menu/menuitem.hh"
#include "time/time.hh"

class i4_image_class;
class g1_draw_context_class;

class g1_3d_pick_window : public i4_menu_item_class
{
 
protected:
  i4_bool need_flip_update;



  sw32 last_mx, last_my;

  i4_bool grabl, grabr;
  i4_image_class *act,*pass;
  i4_time_class start;
  i4_event_reaction_class *reaction;
public:
  struct camera_struct
  {
    i4_float theta, phi, zrot;

    i4_float view_dist,
      center_x,
      center_y,
      center_z; 
    
    i4_bool stopped;

    void init() 
    { 
      stopped=i4_F; 
      center_x=0;
      center_y=0;
      center_z=0;
      view_dist=1;
      theta=0;
      phi=0;
      zrot=0;
    }

  } camera;

  g1_3d_pick_window(w16 w, w16 h,
                    i4_image_class *active_back,
                    i4_image_class *passive_back,
                    camera_struct &camera,
                    i4_event_reaction_class *reaction)

    : i4_menu_item_class(0,0,w,h),
      camera(camera),
      act(active_back),
      pass(passive_back),
      reaction(reaction)
  {
    grabl=i4_F;
    grabr=i4_F;
    need_flip_update=i4_T;
  }

  void request_redraw(i4_bool for_a_child)
  {
    need_flip_update=i4_T; 
    i4_menu_item_class::request_redraw(for_a_child);
  }

  ~g1_3d_pick_window()
  {
    if (reaction)
      delete reaction;
  }

  virtual i4_bool selected() { return i4_F; }

  void setup_bg_color();

  virtual void do_activate() 
  {
    i4_menu_item_class::do_activate();
    start.get();
  }

  virtual void do_deactivate()
  {
    i4_menu_item_class::do_deactivate();
  }

  virtual void parent_draw(i4_draw_context_class &context);
  virtual void draw_object(g1_draw_context_class *context) = 0;

  virtual void receive_event(i4_event *ev);

  char *name() { return "object window"; }
} ;


#endif
