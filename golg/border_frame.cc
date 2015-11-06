/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "border_frame.hh"
#include "loaders/load.hh"
#include "image/image.hh"
#include "device/keys.hh"
#include "device/kernel.hh"
#include "resources.hh"
#include "time/profile.hh"
#include "window/colorwin.hh"
#include "gui/button.hh"
#include "gui/image_win.hh"
#include "human.hh"
#include "mess_id.hh"
#include "objs/map_piece.hh"
#include "g1_render.hh"
#include "r1_api.hh"
#include "r1_win.hh"
#include "r1_clip.hh"
#include "objs/stank.hh"
#include "options.hh"
#include "app/app.hh"
#include "gui/gradiant.hh"
#include "device/device.hh"
#include "g1_texture_id.hh"
#include "g1_speed.hh"
#include "input.hh"
#include "player.hh"
#include "sound_man.hh"
#include "app/app.hh"
#include "lisp/li_class.hh"
#include "lisp/li_load.hh"
#include "lisp/li_init.hh"
#include "li_objref.hh"
#include "map_man.hh"
#include "map_view.hh"
#include "gui/deco_win.hh"
#include "controller.hh"
#include "device/key_man.hh"
#include "tick_count.hh"
#include "map_view.hh"
#include "demo.hh"
#include "window/win_evt.hh"
#include "image_man.hh"
#include "window/wmanager.hh"


S1_SFX(press_tab_to_switch_to_strategy_mode,"computer_voice/press_tab_to_switch_to_strategy_mode_22khz.wav",S1_STREAMED, 200);
S1_SFX(press_tab_to_switch_to_action_mode, "computer_voice/press_tab_to_switch_to_action_mode_22khz.wav", S1_STREAMED, 200);


class i4_profile_class pf_border_draw("border frame draw");


class g1_amount_display_class : public i4_window_class
{
public:
  int last_amount, last_max;
  i4_image_class *im, *dark_im;
  i4_bool refresh_as_text;

  g1_amount_display_class(int w, int h)
    : i4_window_class(w,h)
  {
    refresh_as_text=i4_T;
    im=dark_im=0;
    last_amount=0;
    last_max=1;
  }

  g1_amount_display_class(char *image_name=0)
    : i4_window_class(0,0)
  {
    if (image_name)
    {
      char fn[100];
      sprintf(fn, "bitmaps/stank/frame_%s.tga", image_name);
      im=i4_load_image(fn);
      sprintf(fn, "bitmaps/stank/dark_%s.tga", image_name);
      dark_im=i4_load_image(fn);
    }
    else im=dark_im=0;

    last_amount=1;
    last_max=1;
    refresh_as_text=i4_F;

    if (im)
      resize(im->width(), im->height());
    else
      resize(10,5);
  }

  void update(int new_amount, int max_amount=1)
  {
    if (new_amount!=last_amount || last_max!=max_amount)
    {
      last_amount=new_amount;
      last_max=max_amount;
      request_redraw(i4_F);
    }
  }

  void draw(i4_draw_context_class &context)
  {
    if (refresh_as_text)
    {
      local_image->clear(0, context);
      i4_font_class *fnt=i4_current_app->get_style()->font_hint->normal_font;
      fnt->set_color(0x00ff00);
      char str[30];
      sprintf(str, "%d", last_amount);
      fnt->put_string(local_image, width()-fnt->width(str), 
                      height()/2-fnt->height(str)/2,
                      str, context);
                      
    }
    else
    {
      int midx=width()*last_amount/last_max;
      if (midx>=width())
        midx=width()-1;

      if (im)
        im->put_part(local_image, 0,0,  0,0, midx, height()-1, context);
      else
        local_image->bar(0,0, midx, height()-1, 0xff0000, context);

      if (dark_im)
        dark_im->put_part(local_image, midx,0, midx, 0, width()-1, height()-1, context);
      else
        local_image->bar(midx, 0, width()-1, height()-1, 0, context);
    }
  }

  ~g1_amount_display_class()
  {
    if (im) delete im;
    if (dark_im) delete dark_im;
  }

  char *name() { return "amount_display"; }
};

i4_event_handler_reference_class<g1_border_frame_class> g1_border;
i4_event_handler_reference_class<g1_strategy_screen_class> g1_strategy_screen;

static i4_event_handler_reference_class<g1_amount_display_class>  g1_lives, 
     g1_chain, g1_missiles, g1_main, g1_health, g1_money;


class g1_object_stats_window_class : public i4_window_class
{
public:
  const char *object_name;
  i4_image_class *logo;

  char *name() { return "g1_object_stats"; }
  i4_graphical_style_class *style;

  g1_object_stats_window_class(const char *object_name, 
                               i4_image_class *logo, 
                               i4_graphical_style_class *style)
    : object_name(object_name), logo(logo), 
      style(style),
      i4_window_class(200,170)
  {
    if (logo) 
      resize(logo->width(), logo->height());
  }

  void draw(i4_draw_context_class &context)
  {
    if (logo)
      logo->put_image(local_image,0,0, context);
    else
      i4_gradiant_bar(local_image, 1,1, width()-2, height()-2, 0x7f0000, 0, context);
    

    g1_object_defaults_struct *odef=g1_get_object_defaults(object_name, i4_F);
    if (odef)
    {
      i4_font_class *fnt=style->font_hint->normal_font;

      char *fmt="Name\nCost\nArmor\nSpeed\nDamage\nRange\nRate";

      int x=-20,y=10,w=97;

      fnt->set_color(0);
      fnt->put_paragraph(local_image, x+1,y+1, i4_const_str(fmt), context,0,
                         i4_font_class::RIGHT, w);
      fnt->set_color(0xffff00);
      fnt->put_paragraph(local_image, x,y, i4_const_str(fmt), context,0,
                         i4_font_class::RIGHT, w);

      char fmt2[256];
			
			sprintf(fmt2, "%s\n$%d\n", odef->object_name, odef->cost);

			if (!odef->health)
			  strcat(fmt2, "special\n");
			else
			  sprintf(fmt2+strlen(fmt2), "%d hp\n", odef->health);

			sprintf(fmt2+strlen(fmt2), "%3.3g mph\n",
			  odef->speed * 100.0 * G1_HZ * 3600.0 / 5280.0);

      int wtype=0, damage=-1;
      if (odef->fire_type && odef->fire_type!=li_nil)
        wtype=g1_get_object_type(odef->fire_type);
      
      if (wtype)
        damage=g1_object_type_array[wtype]->get_damage_map()->default_damage;

			if (damage == -1)
			  strcat(fmt2, "special\n"); // damage
			else
			  sprintf(fmt2+strlen(fmt2), "%d hp\n", damage);

			sprintf(fmt2+strlen(fmt2), "%4.4g ft\n%3.3g shots/s",
			  odef->detection_range * 100.0, (double)G1_HZ/(1.0+odef->fire_delay));

      i4_const_str *st=new i4_const_str(fmt2);

      fnt->set_color(0);                                 
      fnt->put_paragraph(local_image, x+w+6+1,y+1, *st, context);
      fnt->set_color(0xffffff);                                 
      fnt->put_paragraph(local_image, x+w+6,y, *st, context);
      delete st;         
    }
  }
      
};



class g1_build_but : public i4_button_class
{
public:
  i4_image_class *norm_im, *act_im, *press_im;

  const char *object_name;
  i4_graphical_style_class *style;

  g1_build_but(const char *object_name,
               i4_event_reaction_class *press,
               i4_event_reaction_class *active,
               i4_graphical_style_class *style,
               i4_image_class *_norm_im,
               i4_image_class *_act_im,
               i4_image_class *_press_im)

    : i4_button_class(0, 0, style, press, 0, active, 0),
      object_name(object_name),
      style(style)
  {
    char image_name[256];       
    sprintf(image_name, "bitmaps/build/build_%s.tga", object_name);
    
    i4_image_class *button_image = i4_load_image(image_name);
    norm_im=_norm_im->copy();
    act_im=_act_im->copy();
    press_im=_press_im->copy();

    i4_draw_context_class context(0,0,_norm_im->width()-1,_norm_im->height()-1);

    if (button_image)
    {

      int wid=button_image->width()-1, hi=button_image->height()-1;
      button_image->put_image(norm_im,  0, 0, context);
      button_image->put_image(act_im,   0, 0, context);
      button_image->put_image(press_im, 0, 0, context);
      delete button_image;
    }
    
    resize(_norm_im->width(), _norm_im->height());    
  }

  ~g1_build_but()
  {
    if (norm_im)
    {
      delete norm_im;
      norm_im = 0;
    }
    
    if (act_im)
    {
      delete act_im;
      act_im = 0;
    }
    
    if (press_im)
    {
      delete press_im;
      press_im = 0;
    }
  }

  char *name() { return "g1_build_but"; }
  
  virtual void parent_draw(i4_draw_context_class &context)
  {
    if (pressed)
      press_im->put_image(local_image, 0,0, context);
    else if (active)
      act_im->put_image(local_image, 0,0, context);
    else
      norm_im->put_image(local_image, 0,0, context);

    i4_parent_window_class::parent_draw(context);    // skip button draw
  }

  void do_idle()
  {
    if (context_help_window.get()) return ;

    i4_str *fn=i4gets("object_logo_fmt").sprintf(100,object_name);
    i4_file_class *fp=i4_open(*fn);
    delete fn;

    i4_image_class *im=0;
    if (fp)
    {
      im=i4_load_image(fp);
      delete fp;
    }
   
    context_help_window=new g1_object_stats_window_class(object_name, im, style);
    i4_current_app->get_root_window()->add_child(x(), y()+height()+5, context_help_window.get());
  }
  
};

void g1_add_build_buttons(i4_parent_window_class *parent, i4_event_handler_class *send_to,
                          int build_first, int active_first)
{

}


void g1_border_frame_class::relocate(i4_parent_window_class *w, char *loc,
                                     int dx, int dy)
{
  i4_window_class *wins[6]={g1_lives.get(), g1_money.get(), g1_main.get(),
                            g1_missiles.get(), g1_chain.get(), g1_health.get() };

  int i=0;
  for (li_object *o=li_get_value(loc); o; o=li_cdr(o,0), i++)
  {
    li_object *v=li_car(o,0);
    int x=li_get_int(li_first(v,0),0), y=li_get_int(li_second(v,0),0);

    if (wins[i])
    {
      if (wins[i]->get_parent())
        wins[i]->get_parent()->remove_child(wins[i]);
    
      w->add_child(x+dx,y+dy, wins[i]);
    }   
  }




}

int g1_border_frame_class::border_x()
{
  return width()/2-frame->width()/2;
}

int g1_border_frame_class::border_y()
{
  return height()-frame->height();
}

g1_border_frame_class::g1_border_frame_class()
  : i4_color_window_class(i4_current_app->get_window_manager()->width(),
                          i4_current_app->get_window_manager()->height(), 0,
                          i4_current_app->get_style())
{
  frame=i4_load_image("bitmaps/stank/status_bar.jpg");
  refresh=REFRESH_ALL;

  mouse_grabbed=i4_F;
  strategy_window=0;
  shrink=0;


  g1_money=new g1_amount_display_class(63,13);
  g1_health=new g1_amount_display_class("reactive");
  g1_lives=new g1_amount_display_class("strategy_lives");
  g1_chain=new g1_amount_display_class("minigun");
  g1_missiles=new g1_amount_display_class("guided");
  g1_main=new g1_amount_display_class("120mm");


  set_cursor(0);


  // create the main 3d view window
  r1_render_window_class *rwin;
  r1_expand_type expand=(r1_expand_type) g1_resources.render_window_expand_mode;

  int cw=width(), ch=height()-frame->height();
  rwin=g1_render.r_api->create_render_window(cw,ch, expand);

  g1_object_controller_class *view=new g1_object_controller_class(cw,ch, style);

  g1_current_controller=view;
  
  rwin->add_child(0,0, view);

  add_child(0, 0, rwin);
  controller_window=rwin;
  strategy_on_top=i4_F;

  view->view.view_mode=G1_ACTION_MODE;
  i4_key_man.set_context("action");

 
  relocate(this, "action_mode_locations", border_x(), border_y());
}


void g1_border_frame_class::reparent(i4_image_class *draw_area, i4_parent_window_class *parent)
{
  i4_parent_window_class::reparent(draw_area, parent);
  if (parent)
    set_strategy_on_top(i4_F);

  if (g1_current_controller.get())
    g1_current_controller->view.view_mode=G1_ACTION_MODE;

  i4_key_man.set_context("action");
}

void g1_border_frame_class::set_strategy_on_top(i4_bool v)
{
  strategy_on_top=v;
  
  if (!v && !mouse_grabbed)
  {
    //    i4_current_app->get_display()->set_mouse_raw_mode(i4_T);
    mouse_grabbed=i4_T;
  }

  if (parent)
  {
    i4_window_request_key_grab_class kgrab(this);
    i4_kernel.send_event(parent, &kgrab);
  }
}


void g1_border_frame_class::receive_event(i4_event *ev)
{
  CAST_PTR(uev, i4_user_message_event_class, ev);
  if (ev->type()==i4_event::USER_MESSAGE)
  {
    int t=uev->sub_type;

    if (t==OPTIONS)
    {
//      g1_sound_man.que_narative(g1_purchase_button_click_wav);
      if (!g1_options_window.get())
      {
        g1_options_window=new g1_option_window(style);
        add_child(-g1_options_window->width(), 0, g1_options_window.get());
      }
      else
      {
        i4_user_message_event_class uev(g1_option_window::SLIDE_AWAY);
        i4_kernel.send_event(g1_options_window.get(), &uev);
      }
    }
  }
  else if (ev->type()==i4_event::MOUSE_MOVE)
  {
    if (!strategy_on_top)
    {
      CAST_PTR(mev,i4_mouse_move_event_class,ev);
      i4_float heh;        
      heh = (((sw32)mev->y - (sw32)mev->ly)*0.01);
      g1_human->mouse_look_increment_y += heh;
    
      heh = (((sw32)mev->x - (sw32)mev->lx)*0.01);
      g1_human->mouse_look_increment_x += heh;        
    }

    i4_parent_window_class::receive_event(ev);
  }
  else if (ev->type()==i4_event::KEY_PRESS || ev->type()==i4_event::WINDOW_MESSAGE)        
    g1_input.receive_event(ev);
  else 
    i4_parent_window_class::receive_event(ev);

}

void g1_border_frame_class::draw(i4_draw_context_class &context)
{
  if (g1_render.main_draw)
    i4_parent_window_class::draw(context);
}


void g1_border_frame_class::parent_draw(i4_draw_context_class &context)
{
  pf_border_draw.start();


  i4_color_window_class::parent_draw(context);

  if (!g1_map_is_loaded())
    return ;


  if ((refresh & REFRESH_FRAME) || !undrawn_area.empty())
  {
    if (frame)
      frame->put_image(local_image, border_x(), border_y(), context);
    
    refresh |= REFRESH_ALL;
  }  

   
  i4_window_class::draw(context);
  refresh=0;

  pf_border_draw.stop();
}


static int first_start=1;
static int first_strategy=1;


void g1_border_frame_class::update()   // check for changes in the game
{
  if (first_start)  
  {
    first_start=0;
    press_tab_to_switch_to_strategy_mode.play();
  }


  if (!g1_map_is_loaded())
    return ;
  
  g1_player_info_class *p=g1_player_man.get_local();

  
  g1_money->update(p->money());
  g1_lives->update(p->num_stank_lives(), 5);


  g1_player_piece_class *stank=p->get_commander();
  if (stank)
  {  
    g1_main->update(stank->ammo[0].amount,  stank->ammo[0].ammo_type->max_amount);
    g1_missiles->update(stank->ammo[1].amount, stank->ammo[1].ammo_type->max_amount);
    g1_chain->update(stank->ammo[2].amount, stank->ammo[2].ammo_type->max_amount);
    g1_health->update(stank->health, stank->ammo[3].ammo_type->max_amount);
  }

  
  if (g1_current_controller.get() && strategy_on_top)
  {
    // so that sound will work...
    i4_transform_class transform;
    g1_current_controller->view.calc_transform(transform);
  }

}


g1_border_frame_class::~g1_border_frame_class()
{
  if (frame)
  {
    delete frame;
    frame = 0;
  }
  
  if (mouse_grabbed)
  {
    //    i4_current_app->get_display()->set_mouse_raw_mode(i4_F);
    mouse_grabbed=i4_F;
  }
}


 



g1_help_screen_class::g1_help_screen_class(w16 w, w16 h, 
                                           i4_graphical_style_class *style, 
                                           const i4_const_str &image_name,
                                           w32 mess_id_to_send)
  : i4_parent_window_class(w,h),
    mess_id_to_send(mess_id_to_send)

{
  help=i4_load_image(image_name);  

}


void g1_help_screen_class::parent_draw(i4_draw_context_class &context)
{
  if (help)
    help->put_image(local_image, 0,0, context);
}

void g1_help_screen_class::receive_event(i4_event *ev)
{
  if (ev->type()==i4_event::MOUSE_BUTTON_DOWN || ev->type()==i4_event::KEY_PRESS)
  {
    i4_user_message_event_class u(mess_id_to_send);
    i4_kernel.send_event(i4_current_app, &u);    
  }
}

g1_help_screen_class::~g1_help_screen_class()
{
  if (help) 
    delete help;
}


w32 g1_get_upgrade_color(int upgrade_level)  // used for font colors & border edges
{
  upgrade_level++;
  li_object *o=li_get_value("upgrade_colors");
  while (li_cdr(o,0) && upgrade_level)
  {
    upgrade_level--;
    o=li_cdr(o,0);
  }
  
  return li_get_int(li_car(o,0),0);
}


li_object *g1_redraw(li_object *o, li_environment *env)
{
  if (g1_current_controller.get())
      g1_current_controller->request_redraw(i4_F);
  return 0;
}

li_object *g1_redraw_all(li_object *o, li_environment *env)
{
  if (g1_current_controller.get())
      g1_current_controller->request_redraw(i4_F);
  return 0;
}

li_automatic_add_function(g1_redraw, "redraw");
li_automatic_add_function(g1_redraw_all, "redraw_all");


li_object *g1_strategy_on_top(li_object *o, li_environment *env)
{
  if (first_strategy)
  {
    first_strategy=0;
    press_tab_to_switch_to_action_mode.play();
    g1_current_controller->scroll_message(i4gets("switch_to_action"));
  }


  if (!g1_border.get() || g1_border->strategy_on_top)
    return 0;

  if (g1_border->mouse_grabbed)
  {
    //    i4_current_app->get_display()->set_mouse_raw_mode(i4_F);
    g1_border->mouse_grabbed=i4_F;
  }
    
  g1_border->add_child(0,0, g1_strategy_screen.get());

  g1_border->remove_child(g1_border->controller_window);
  g1_strategy_screen->add_child(0,0, g1_border->controller_window);

  g1_demo_script_add("(strategy_on_top)");
  g1_border->set_strategy_on_top(i4_T);

  g1_border->resize_controller(0);

  g1_current_controller->view.view_mode=G1_STRATEGY_MODE;
  i4_key_man.set_context("strategy");


  g1_border->relocate(g1_strategy_screen.get(), "strategy_mode_locations", 
                      g1_strategy_screen->border_x(),
                      g1_strategy_screen->border_y());

  return 0;
}


li_object *g1_strategy_on_bottom(li_object *o, li_environment *env)
{
  if (!g1_border.get() || !g1_border->strategy_on_top)
    return 0;

  g1_demo_script_add("(strategy_on_bottom)");
  i4_window_class *cw=g1_border->controller_window;


  g1_border->remove_child(g1_strategy_screen.get());




  g1_strategy_screen->remove_child(g1_border->controller_window);
  g1_border->add_child(0,0, g1_border->controller_window);

  g1_border->set_strategy_on_top(i4_F);
  g1_border->resize_controller(0);

  g1_current_controller->view.view_mode=G1_ACTION_MODE;
  i4_key_man.set_context("action");

  g1_border->relocate(g1_border.get(), "action_mode_locations", 
                      g1_border->border_x(),
                      g1_border->border_y());


  return 0;
}

li_object *g1_strategy_toggle(li_object *o, li_environment *env)
{
  if (g1_border.get())
    if (g1_border->strategy_on_top)
      g1_strategy_on_bottom(0,0);
    else
      g1_strategy_on_top(0,0);
  return 0;

}

int g1_strategy_screen_class::border_x()
{
  return width()-frame->width();
}

int g1_strategy_screen_class::border_y()
{
  return height()-frame->height();
}


void g1_strategy_screen_class::parent_draw(i4_draw_context_class &context)
{
  i4_color_window_class::parent_draw(context);
  if (!frame)
    return ;


  frame->put_image(local_image, border_x(), border_y(), context);
}

g1_strategy_screen_class::g1_strategy_screen_class()
  : build_buttons(0,16),
    i4_color_window_class(i4_current_app->get_window_manager()->width(),
                          i4_current_app->get_window_manager()->height(),
                          0, 
                          i4_current_app->get_style())
{ 
  shrink=0;
  frame=i4_load_image("bitmaps/options/strategy_layout.jpg");

  i4_window_class *strategy_window;

  int rw=frame->width(), rh=height()-frame->height();

  strategy_window=g1_create_radar_view(rw,
                                       rh,
                                       G1_RADAR_DRAW_UNHIDDEN_PATHS |
                                       G1_RADAR_CLICK_SELECTS_PATH |
                                       G1_RADAR_INTERLACED |
                                       G1_RADAR_USE_ICONS);
  add_child(width() - rw, 0, strategy_window);

  if (g1_map_is_loaded())
    create_build_buttons();


}


void g1_strategy_screen_class::create_build_buttons()
{
  for (int i=0; i<build_buttons.size(); i++)
  {
    remove_child(build_buttons[i]);
    delete build_buttons[i];
  }

  build_buttons.uninit();


  i4_image_class *build_back=i4_load_image("bitmaps/build/build_button.tga");
  i4_image_class *build_active=i4_load_image("bitmaps/build/build_button_highlight.tga");
  i4_image_class *build_selected=i4_load_image("bitmaps/build/build_button_select.tga");

  int x1=g1_resources.build_buttons.x1, y1=g1_resources.build_buttons.y1, 
      x2=g1_resources.build_buttons.x2, y2=g1_resources.build_buttons.y2;
                          
  int x=x1, y=y1;
  int max_line_height=build_back->height();
  int bw=build_back->width();

  for (li_object *o=li_get_value("player_buildable"); o; o=li_cdr(o,0))
  {
    int type=g1_get_object_type(li_symbol::get(li_car(o,0),0));

    if (g1_object_type_array[type] &&
        g1_object_type_array[type]->defaults &&
        g1_object_type_array[type]->defaults->cost)
    {     
      i4_event_reaction_class *re=new i4_event_reaction_class(this, BUILD+type);
      i4_event_reaction_class *cost_re=new i4_event_reaction_class(this, ACTIVE+type);


      g1_build_but *b = new g1_build_but(g1_object_type_array[type]->name(), re, cost_re, 
                                         i4_current_app->get_style(),
                                         build_back, build_active, build_selected);
      build_buttons.add(b);

      b->set_popup(i4_T);        

      if (x+bw > x2)
      {
        x=x1; 
        y+=max_line_height;
      }

      add_child(x,y,b);

      x+=bw;
    }
  }


  // add blank buttons for the rest to even out the screen
  while (x+bw<=x2)
  {
    i4_event_reaction_class *re=new i4_event_reaction_class(this, 0xffff);
    g1_build_but *b = new g1_build_but("no_vehicle", re, 0, 
                                       i4_current_app->get_style(), 
                                       build_back, build_active, build_selected);
    build_buttons.add(b);
    b->set_popup(i4_T);         
    add_child(x,y,b);
    x+=bw;
  }


  delete build_back;
  delete build_active;
  delete build_selected;


}


void g1_strategy_screen_class::receive_event(i4_event *ev)
{
  if (ev->type()==i4_event::USER_MESSAGE)
  {
    CAST_PTR(uev, i4_user_message_event_class, ev);
    int t=uev->sub_type;

    if (t>=BUILD && t<ACTIVE)
      g1_human->build_unit((g1_object_type)(t-BUILD));
    else if (t>ACTIVE)
    {
      t-=ACTIVE;
      if (g1_object_type_array[t])
      {
        //        last.cost=g1_object_type_array[t]->defaults->cost;
        //        refresh|=REFRESH_COST;
      }
    }

    if (t==OPTIONS)
    {
      if (!g1_options_window.get())
      {
        g1_options_window=new g1_option_window(i4_current_app->get_style());
        add_child(-g1_options_window->width(), 
                  height()-g1_options_window->height(),
                  g1_options_window.get());
      }
      else
      {
        i4_user_message_event_class uev(g1_option_window::SLIDE_AWAY);
        i4_kernel.send_event(g1_options_window.get(), &uev);
      }
    }

  }
  else i4_parent_window_class::receive_event(ev);
}

enum {xa=20, ya=10 };

void g1_border_frame_class::resize_controller(int shrink_add)
{
  if (!g1_border->strategy_on_top)
  {
    shrink+=shrink_add;

    int cw=width()-shrink*2;
    int ch=height()-frame->height()-shrink*2;

    remove_child(controller_window);
    controller_window->resize(cw,ch);
    add_child(width()/2-cw/2, (height()-frame->height())/2-ch/2, controller_window);
  }
  else 
  {
    int s=g1_strategy_screen->shrink;
    s+=shrink_add;
    g1_strategy_screen->shrink=s;

    int cw=width()-g1_strategy_screen->frame->width()-s*2;
    int ch=height()-s*2;

    
    g1_strategy_screen->remove_child(controller_window);
    controller_window->resize(cw,ch);
    g1_strategy_screen->add_child((width()-g1_strategy_screen->frame->width())/2-cw/2, 
                                  height()/2-ch/2, controller_window);
  }
}


li_object *g1_shrink_screen(li_object *o, li_environment *env)
{
  if (!g1_border.get())
    return 0;


    
  if (!g1_border->strategy_on_top)
  {
    if (g1_border->width()-g1_border->shrink*2>80)
      g1_border->resize_controller(xa);
  }
  else
  {
    if (g1_strategy_screen->width()-g1_strategy_screen->shrink*2>80)
      g1_border->resize_controller(xa);

  }
  return 0;
}




li_object *g1_grow_screen(li_object *o, li_environment *env)
{
  if (!g1_border.get())
    return 0;

  int xa=20, ya=10;
    
  if (!g1_border->strategy_on_top)
  {
    if (g1_border->shrink-xa>=0)
      g1_border->resize_controller(-xa);
  }
  else
  {
    if (g1_strategy_screen->shrink-xa>=0)
      g1_border->resize_controller(-xa);
  }
  return 0;
}


li_automatic_add_function(g1_strategy_on_top, "strategy_on_top");  
li_automatic_add_function(g1_strategy_on_bottom, "strategy_on_bottom");  
li_automatic_add_function(g1_strategy_toggle, "strategy_toggle");  

li_automatic_add_function(g1_shrink_screen, "shrink_screen");
li_automatic_add_function(g1_grow_screen, "grow_screen");


