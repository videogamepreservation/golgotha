/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "cwin_man.hh"
#include "resources.hh"
#include "controller.hh"
#include "border_frame.hh"
#include "map_view.hh"
#include "player.hh"
#include "image/image.hh"
#include "window/win_evt.hh"
#include "device/kernel.hh"
#include "window/style.hh"
#include "gui/deco_win.hh"
#include "map.hh"
#include "r1_win.hh"
#include "g1_render.hh"
#include "objs/stank.hh"
#include "saver.hh"
#include "math/pi.hh"
#include "device/key_man.hh"
#include "input.hh"
#include "map_man.hh"
#include "lisp/lisp.hh"

g1_cwin_man_class *g1_cwin_man=0;



void g1_change_key_context(w8 view_mode)
{
  switch (view_mode)
  {
    case G1_EDIT_MODE :
      i4_key_man.set_context("editor"); break;

    case G1_STRATEGY_MODE :
      i4_key_man.set_context("strategy"); break;

    case G1_ACTION_MODE :
      i4_key_man.set_context("action"); break;

    case G1_FOLLOW_MODE :
      i4_key_man.set_context("follow"); break;

    case G1_WATCH_MODE :
      i4_key_man.set_context("strategy"); break;
  }
}




void g1_cwin_man_class::save_views(g1_saver_class *fp)
{
  fp->mark_section("Game view setting V1");
  view_state.save(fp);
}

void g1_cwin_man_class::load_views(g1_loader_class *fp)
{
  if (fp && !fp->goto_section("Game view setting V1"))
    fp=0;     // load defaults if not in file

  view_state.load(fp);
}


void g1_cwin_man_class::init(i4_parent_window_class *_parent,
                             i4_graphical_style_class *_style,
                             i4_image_class *_root_image,
                             i4_display_class *display,
                             i4_window_manager_class *wm)
{
  style=_style;
  parent=_parent;
  root_image=_root_image;
  pal=_root_image->get_pal(); 
}

g1_cwin_man_class::g1_cwin_man_class()
{
  parent=0;
  root_image=0;
  style=0;
}

void g1_cwin_man_class::create_views()

{    
  if (!g1_border.get())
  {
    g1_strategy_screen=new g1_strategy_screen_class();

    g1_border = new g1_border_frame_class(); 
    //    g1_current_controller->view=view_state;
    parent->add_child(0,0,g1_border.get());

  }
}


void g1_cwin_man_class::destroy_views()
{
  if (g1_border.get())
  {
    i4_kernel.delete_handler(g1_strategy_screen.get());
    i4_kernel.delete_handler(g1_border.get());
  }
  
}

void g1_cwin_man_class::receive_event(i4_event *ev)
{  
//   if (view.get())
//     g1_input.receive_event(ev);
}


void g1_cwin_man_class::map_changed()
{
  g1_player_type p=g1_default_player;
  g1_radar_recalculate_backgrounds();
}

