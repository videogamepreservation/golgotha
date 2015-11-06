/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "lisp/lisp.hh"
#include "lisp/li_init.hh"
#include "app/app.hh"
#include "lisp/li_dialog.hh"
#include "lisp/li_class.hh"
#include "window/style.hh"
#include "player.hh"
#include "gui/button.hh"
#include "gui/image_win.hh"
#include "loaders/load.hh"
#include "window/wmanager.hh"
#include "objs/stank.hh"
#include "map_man.hh"
#include "objs/stank.hh"
#include "border_frame.hh"

i4_event_handler_reference_class<i4_parent_window_class> g1_cheat_window;
static int team_editing;

enum { CHEAT_MONEY,
       CHEAT_STANK_INVINSIBILITY,
       CHEAT_TROOP_INVINSIBILITY,
       CHEAT_NUKE };

static char *cheats[]={"bitmaps/cheats/mo_money.jpg",
                       "bitmaps/cheats/invulnerable.jpg",
                       "bitmaps/cheats/group_invulnerable.jpg",
                       "bitmaps/cheats/nuke.jpg", 0};


class cheat_handler_class : public i4_event_handler_class
{
public:
  void receive_event(i4_event *ev)
  {
    if (!g1_map_is_loaded())
      return;
    
    CAST_PTR(uev, i4_user_message_event_class, ev);
    switch(uev->sub_type)
    {
      case CHEAT_MONEY :
        g1_player_man.get_local()->money()+=5000;
        break;
        
      case CHEAT_STANK_INVINSIBILITY :
      {
        g1_player_piece_class *stank=g1_player_man.get_local()->get_commander();      
        if (stank)
          stank->stank_flags ^= g1_player_piece_class::ST_GODMODE;
      } break;
      
      case CHEAT_NUKE :
      {
        g1_player_piece_class *stank=g1_player_man.get_local()->get_commander();
        if (stank)
        {
          for (int i=0; i<g1_player_piece_class::MAX_WEAPONS-1; i++)
          {
            int mx=stank->ammo[i].ammo_type->max_amount;
            stank->ammo[i].amount=mx;
          }
        }
        
      } break;
      
    }
    
  }
  
  char *name() { return "cheat_handler"; }
} cheat_handler;

li_object *g1_cheat_menu(li_object *o, li_environment *env)
{  
  if (g1_cheat_window.get())
    i4_kernel.delete_handler(g1_cheat_window.get());
  else
  {
    i4_graphical_style_class *s=i4_current_app->get_style();
    i4_color_window_class *cw=new i4_color_window_class(0,0,0,s);

    int id=0;
    for (char **a=cheats; *a; a++, id++)
    {
      i4_image_class *im=i4_load_image(*a);
      if (im)
      {
        i4_button_class *b=new i4_button_class(0, new i4_image_window_class(im, i4_T), s,
                                               new i4_event_reaction_class(&cheat_handler, id));
        b->set_popup(i4_T);
        cw->add_child(0,0, b);
      }
    }
    cw->arrange_right_down();
    cw->resize_to_fit_children();
    i4_window_manager_class *wm=i4_current_app->get_window_manager();
    wm->add_child(wm->width()-cw->width()-1,
                  wm->height()-cw->height()-1, cw);
    g1_cheat_window=cw;
  }

  return 0;
}


li_automatic_add_function(g1_cheat_menu, "Cheat Menu");



li_object *g1_upgrade_level_0(li_object *o, li_environment *env)
{
  g1_player_piece_class *p=g1_player_man.get_local()->get_commander();
  if (p && g1_border.get())
  {
    g1_player_man.get_local()->supertank_upgrade_level=0;
    p->find_weapons();
  }

  return 0;
}



li_object *g1_upgrade_level_1(li_object *o, li_environment *env)
{
  g1_player_piece_class *p=g1_player_man.get_local()->get_commander();
  if (p && g1_border.get())
  {
    g1_player_man.get_local()->supertank_upgrade_level=1;
    p->find_weapons();
  }

  return 0;
}



li_object *g1_upgrade_level_2(li_object *o, li_environment *env)
{
  g1_player_piece_class *p=g1_player_man.get_local()->get_commander();
  if (p && g1_border.get())
  {   
    g1_player_man.get_local()->supertank_upgrade_level=2;     
    p->find_weapons();
  }

  return 0;
}


li_automatic_add_function(g1_upgrade_level_0, "upgrade_level_0");
li_automatic_add_function(g1_upgrade_level_1, "upgrade_level_1");
li_automatic_add_function(g1_upgrade_level_2, "upgrade_level_2");

