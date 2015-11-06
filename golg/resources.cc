/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "resources.hh"
#include "window/cursor.hh"
#include "memory/malloc.hh"
#include "string/string.hh"
#include "network/login.hh"
#include "string/string.hh"
#include "loaders/load.hh"
#include "r1_api.hh"

#include "sound/sound.hh"

g1_resource_class g1_resources;

float f_get(char *rname)
{
  i4_const_str::iterator s=i4gets(rname).begin();
  return s.read_float();
}


void g1_resource_class::g_float(i4_float &f, char *res_name)
{
  i4_const_str st=use->get(res_name);
  if (st.null())
    i4_error("Constant missing %s\n",res_name);
  i4_const_str::iterator i=st.begin();
  f=i.read_float();
}

void g1_resource_class::g_cursor(i4_cursor_class *&c, char *res_name, int big, int color)
{
  i4_const_str st=use->get(res_name);
  if (st.null())
    i4_error("Constant missing from res/constants.res %s\n",res_name);

  i4_const_str::iterator i=st.begin();
  i4_str *fmt=i.read_string();

  w32 tcolor=i.read_number();
  int hx=i.read_number();  if (!big) hx/=2;
  int hy=i.read_number();  if (!big) hy/=2;

  i4_str *fn=fmt->sprintf(100,big ? "big" : "small", color ? 16 : 2);
  delete fmt;

  i4_image_class *im=i4_load_image(*fn);
  if (!im)
  {
    char buf[100];
    i4_os_string(*fn, buf, 100);
    i4_error("failed to load image for cursor in constants.res : %s",buf);
  }

  delete fn;

  c=new i4_cursor_class(im, tcolor, hx, hy);

  delete im;
}
  
void g1_resource_class::g_w32(w32 &x, char *res_name)
{
  i4_const_str st=use->get(res_name);
  if (st.null())
    i4_error("Constant missing %s\n",res_name);
  i4_const_str::iterator i=st.begin();
  x=i.read_number();
}

void g1_resource_class::g_sw32(sw32 &x, char *res_name)
{
  i4_const_str st=use->get(res_name);
  if (st.null())
    i4_error("Constant missing %s\n",res_name);
  i4_const_str::iterator i=st.begin();
  x=i.read_number();
}

void g1_resource_class::g_w16(w16 &x, char *res_name)
{
  i4_const_str st=use->get(res_name);
  if (st.null())
    i4_error("Constant missing %s\n",res_name);
  i4_const_str::iterator i=st.begin();
  x=i.read_number();
}

void g1_resource_class::g_sw16(sw16 &x, char *res_name)
{
  i4_const_str st=use->get(res_name);
  if (st.null())
    i4_error("Constant missing %s\n",res_name);
  i4_const_str::iterator i=st.begin();
  x=i.read_number();
}

void g1_resource_class::g_point(i4_3d_point_class &p, char *res_name)
{
  i4_const_str *s,*x=use->get_array(res_name);
  
  s=x;
  i4_const_str::iterator i=x->begin();
  p.x=i.read_float();

  x++;
  i=x->begin();
  p.y=i.read_float();

  x++;
  i=x->begin();
  p.z=i.read_float();

  i4_free(s);
}

void g1_resource_class::g_rect(rect &r, char *res_name)
{
 i4_const_str *s,*x=use->get_array(res_name);
  
  s=x;
  i4_const_str::iterator i=x->begin();
  r.x1=i.read_number();

  x++;
  i=x->begin();
  r.y1=i.read_number();

  x++;
  i=x->begin();
  r.x2=i.read_number();

  x++;
  i=x->begin();
  r.y2=i.read_number();

  i4_free(s);
}

void g1_resource_class::cleanup()
{
  int i;
  for (i=0; i<G1_TOTAL_CURSORS; i++)
  {
    if (big_cursors[i])      
      delete big_cursors[i];
    
    if (small_cursors[i])      
      delete small_cursors[i];
  }


  for (i=0; i<t_images; i++)
    if (images[i])
    {
      delete images[i];
      images[i]=0;
    }

  i4_free(images);

  i4_free(build_context_help);  
  t_build_context_helps=0;

  if (username) delete username;
  username=0;
}

void g1_resource_class::load()
{
  int i;

  i4_string_manager_class constants;
  constants.init();
  constants.load(i4gets("constants"));
  
  paused=i4_F;
 
  username=i4_get_user_name();

  use=&constants;

  draw_flags=DRAW_ALL;

  char *cursors[]={"default_cursor",
                   "select_cursor", "move_cursor",
                  "target_cursor", "follow_cursor", "x_cursor" };

  for (i=0; i<G1_TOTAL_CURSORS; i++)
  {
    g_cursor(big_cursors[i], cursors[i], 1, 1);
    g_cursor(small_cursors[i], cursors[i],  0, 0);
  }
  
  g_float(gravity,                                 "gravity");
  g_float(damping_friction,                        "damping_friction");
  damping_fraction = 1.0/damping_friction;
  ln_damping_friction = exp(0.1*damping_friction);

  g_float(recoil_factor,                           "recoil_factor");
  g_float(recoil_gravity,                          "recoil_gravity");

  g_float(sink_rate,                               "sink_rate");
  g_sw16(water_damage,                             "water_damage");

  g_float(player_turn_speed,                       "player_turn_speed");
  g_float(player_accel,                            "player_accel");

  g_rect(health_window,                            "health_window");
  g_rect(chain_gun_window,                         "chain_gun_window");

  g_rect(main_gun_window,                          "main_gun_window");
  g_rect(missile_window,                           "missile_window");
  g_rect(options_window,                           "options_window");



  g_float(player_max_speed,                        "player_max_speed");
  g_float(player_max_strafe_speed,                 "player_max_strafe_speed");
  g_float(player_stop_friction,                    "player_stop_friction");

  g_float(strafe_turn_speed,                       "strafe_turn_speed");
  g_float(player_roll_speed,                       "player_roll_speed");
  g_float(player_roll_max,                         "player_roll_max");
  g_float(bullet_speed,                            "bullet_speed"); 

  g_float(player_strafe_accel,                     "player_strafe_accel");

  g_float(camera_dist,                             "camera_dist");
  g_float(camera_angle,                            "camera_angle");
  g_float(startegy_camera_angle,                   "startegy_camera_angle");
  g_float(startegy_camera_dist,                    "startegy_camera_dist");
  g_float(strategy_camera_turn_angle,              "strategy_camera_turn_angle");

  g_float(follow_camera_height,                    "follow_camera_height");
  g_float(follow_camera_dist,                      "follow_camera_dist");
  g_float(follow_camera_rotation,                  "follow_camera_rotation");

  g_float(player_fire_slop_angle,                  "player_fire_slop_angle");
  g_float(player_turret_radius,                    "player_turret_radius");
  g_point(player_top_attach,                       "player_top_attach");
  g_point(rocket_tank_top_attach,                  "rocket_tank_top_attach");
  g_point(turret_muzzle_attach,                    "turret_muzzle_attach");
  g_point(turret_top_attach,                       "turret_top_attach");
  g_point(supergun_muzzle_attach,                  "supergun_muzzle_attach");

  g_point(repairer_tip_attach,                     "repairer_tip_attach");
  g_float(repairer_boom_offset,                    "repairer_boom_offset");

  g_float(lod_switch_dist,                         "lod_switch_dist");
  g_float(lod_disappear_dist,                      "lod_disappear_dist");
  g_float(skimpy_details_dist,                     "skimpy_details_dist");

  g_float(view_radii[0],                           "visual_radii_low");
  g_float(view_radii[1],                           "visual_radii_med");
  g_float(view_radii[2],                           "visual_radii_high");

  g_rect(balance,                                  "balance");
  g_rect(build_buttons,                            "build_buttons");

  g_rect(lives,                                    "lives");
  g_rect(mouse_scroll,                             "mouse_scroll");

  g_w16(net_hostname_x,                            "net_hostname_x");
  g_w16(net_hostname_y,                            "net_hostname_y");

  g_w16(net_username_x,                            "net_username_x");
  g_w16(net_username_y,                            "net_username_y");

  g_w16(net_found_x1,                              "net_found_x1");
  g_w16(net_found_x2,                              "net_found_x2");
  g_w16(net_found_y,                               "net_found_y");

  g_w16(net_start_x,                               "net_start_x");
  g_w16(net_start_y,                               "net_start_y");

  g_w16(net_find_port,                             "net_find_port");
  g_w16(net_udp_port,                              "net_udp_port");

  g_sw16(compass_x,                                "compass_x");
  g_sw16(compass_y,                                "compass_y");

  g_sw16(stat_pad_start_x,                         "stat_pad_start_x");
  g_sw16(stat_pad_start_y,                         "stat_pad_start_y");
  g_w16(stat_pad_size_x,                           "stat_pad_size_x");
  g_w16(stat_pad_size_y,                           "stat_pad_size_y");
  g_sw16(stat_pad_step_x,                          "stat_pad_step_x");
  g_sw16(stat_pad_step_y,                          "stat_pad_step_y");

  g_w16(small_health_added,                        "small_health_added");
  g_w16(large_health_added,                        "large_health_added");

  g_w16(small_bullets_added,                       "small_bullets_added");
  g_w16(large_bullets_added,                       "large_bullets_added");

  g_w16(small_chain_added,                         "small_chain_added");
  g_w16(large_chain_added,                         "large_chain_added");

  g_w16(small_missiles_added,                       "small_missiles_added");
  g_w16(large_missiles_added,                       "large_missiles_added");

  g_w16(small_money_added,                          "small_money_added");
  g_w16(large_money_added,                          "large_money_added");



  g_w32(supertank_machine_gun_damage,              "supertank_machine_gun_damage");
  g_w32(supertank_turret_damage,                   "supertank_turret_damage");
  g_w32(supertank_guided_missile_damage,           "supertank_guided_missile_damage");

  g_w32(action_strategy_ticks_to_switch,           "action_strategy_ticks_to_switch");

  g_w32(lock_cheat,                                "lock_cheat");

  i4_const_str *im_array=constants.get_array("images");
  t_images=0;
  for (i=0; !im_array[i].null(); i++)
    t_images++;

  images=(i4_image_class **)i4_malloc(sizeof(i4_image_class **) * t_images, "image array");

  for (i=0; i<t_images && !im_array[i].null(); i++)
  {
    images[i]=i4_load_image(im_array[i]);
    if (!images[i])
    {
      char im_name[100];
      i4_error("image missing %s", i4_os_string(im_array[i], im_name, sizeof(im_name)));
    }
  }
  i4_free(im_array);


  use=0;   // make sure this isn't used any more
  constants.uninit();


  build_context_help=i4_string_man.get_array("build_context_help");
  if (!build_context_help)
    i4_error("no build_context_help in res/g1.res");
  
  t_build_context_helps=0;
  for (i4_const_str *str=build_context_help; !str->null(); str++)
    t_build_context_helps++;


  render_window_expand_mode=R1_COPY_1x1;
}


