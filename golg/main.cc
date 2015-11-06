/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "time/profile.hh"
#include "window/wmanager.hh"
#include "map.hh"
#include "tile.hh"
#include "app/app.hh"
#include "controller.hh"
#include "input.hh"
#include "objs/map_piece.hh"
#include "player.hh"
#include "math/random.hh"
#include "resources.hh"
#include "menu.hh"
#include "border_frame.hh"
#include "sound/sfx_id.hh"
#include "remove_man.hh"
#include "saver.hh"
#include "mess_id.hh"
#include "sound_man.hh"
#include "time/time.hh"
#include "time/timedev.hh"
#include "g1_speed.hh"
#include "cwin_man.hh"
#include "m_flow.hh"
#include "g1_render.hh"
#include "objs/defaults.hh"
#include "net/startup.hh"       // g1 stuff for start network menus
#include "network/net_prot.hh"  // so we can choose a protocol when we start
#include "r1_api.hh"
#include "g1_texture_id.hh"
#include "statistics.hh"
#include "map_man.hh"
#include "tmanage.hh"
#include "level_load.hh"
#include "lisp/lisp.hh"
#include "make_tlist.hh"
#include "gui/image_win.hh"
#include "font/anti_prop.hh"
#include "lisp/li_init.hh"
#include "objs/vehic_sounds.hh"
#include "tick_count.hh"
#include "map_view.hh"
#include "demo.hh"
#include "image_man.hh"

#include <string.h>

i4_profile_class pf_calc_model("calc model");
i4_profile_class pf_calc_model_1("calc model 1");
i4_profile_class pf_calc_model_2("calc model 2");
i4_profile_class pf_calc_model_3("calc model 3");
i4_profile_class pf_calc_model_4("calc model 4");
i4_profile_class pf_calc_model_5("calc model 5");
i4_profile_class pf_calc_model_6("calc model 6");
i4_profile_class pf_calc_model_7("calc model 7");
i4_profile_class pf_calc_model_8("calc model 8");
i4_profile_class pf_calc_model_9("calc model 9");


extern i4_grow_heap_class *g1_object_heap;

int G1_HZ=10;

static li_symbol_ref s_deterministic("deterministic");

static char first_level[80];
extern int last_draw_tick;
extern int frame_locked_mode;

class golgotha_app : public i4_application_class
{
private:
  i4_window_class *main_menu;
  i4_bool playing_movie;
  w32 _max_memory;

  w32 argc;
  i4_const_str *argv;
  i4_bool need_post_play_load;
  i4_bool start_in_editor;

  struct redraw_later_struct
  {
    i4_bool waiting;
    i4_time_device_class::id id;

    redraw_later_struct() { waiting=i4_F; }    
  } redraw_later;


  virtual w32 max_memory() { return _max_memory; }


  i4_net_protocol *protocol;

public:  

  void handle_no_displays()
  {
#ifdef _WINDOWS
    i4_error("Golgotha requires DirectX5, or a 3dfx card\n"
             "Make sure you have DX5 (and are in 16bit color mode)"
             "Or that you have the current version Glide (for 3dfx) installed");
#else
    i4_error("Golgotha requires a 3dfx card\n");
#endif
  }
  
  void pre_play_save();
  void post_play_load();

  golgotha_app(w32 argc, i4_const_str *argv)
    : argc(argc), argv(argv)
  {
    start_in_editor=i4_F;
    need_post_play_load=i4_F;
    playing_movie=i4_F;
    main_menu=0;
    protocol=0;
    _max_memory = 10*1024*1024;
  }

  void refresh()
  {
    g1_render.main_draw=i4_T;

    if (g1_render.r_api)
      g1_render.r_api->modify_features(R1_LOCK_CHEAT,g1_resources.lock_cheat);

    i4_application_class::refresh();

    g1_render.main_draw=i4_F;
  }

  void grab_time();



  void map_changed() 
  {     
    li_call("redraw");

    if (redraw_later.waiting)
      i4_time_dev.cancel_event(redraw_later.id);

    redraw_later.waiting=i4_T;
    i4_user_message_event_class m(G1_REDRAW_LATER);
    redraw_later.id=i4_time_dev.request_event(this, &m, 500);
  }

  void init();
  
  virtual i4_bool idle() { return i4_F; }

  void calc_model()
  {
    int fixed=li_get_value(s_deterministic.get())==li_true_sym;

    if (g1_map_is_loaded())
    {
      pf_calc_model.start();

      g1_render.r_api->get_tmanager()->next_frame();



      


      i4_time_class now;
      sw32 md;
      int ticks_simulated=0;
      do
      {
        md=now.milli_diff(g1_get_map()->tick_time);
        if (fixed || md>0 || frame_locked_mode)
        {
          if (!g1_resources.paused)
          {
            pf_calc_model_1.start();
            g1_player_man.think();  // thinks player/team type thoughts
            pf_calc_model_1.stop();

            g1_map_class *map=g1_get_map();
            map->think_objects();

            g1_demo_tick();

            if ((g1_tick_counter&7)==0)
              g1_radar_update();


            g1_remove_man.process_requests();
            ticks_simulated++;
          }
          else
            g1_get_map()->tick_time.add_milli((1000/G1_HZ));

          if (g1_current_controller.get())
            g1_current_controller->update_camera();
          
        }       
      } while (!frame_locked_mode && !fixed && md>0 && ticks_simulated<10);
      
      if (ticks_simulated>=10) // simulation can't catch up with display
	g1_get_map()->tick_time.get();      // catch up the time

      if (g1_current_controller.get() &&
          g1_current_controller->view.get_view_mode()!=G1_EDIT_MODE)
        g1_current_controller->request_redraw(i4_F);


      g1_sound_man.poll(i4_T);             // update sound effects (play next narative sfx)



      
      if (playing_movie)
      {
        if (!g1_get_map()->advance_movie_with_time())
        {
          playing_movie=i4_F;

          
          if (need_post_play_load)
          {
            if (g1_resources.paused)
              li_call("Pause");
            post_play_load();
          }
          else if (g1_current_controller.get())
            g1_current_controller->view.suggest_camera_mode(G1_ACTION_MODE);
        }

        li_call("redraw");
      }
      else 
        g1_input.reset_esc();

      pf_calc_model.stop();
    }

    g1_input.acknowledge();          // acknowledge that the keys have been examined    
  }

  void return_to_game();
  void start_new_game();
  void network_menu(); 
  void server_menu();
  void help_screen(char *screen_name, int next_id);
  void plot_screen();
  void client_wait_menu();
  void do_main_menu();
  void do_options();

  void receive_event(i4_event *ev);
  void uninit()
  {
    i4_kernel.unrequest_events(this,
                               i4_device_class::FLAG_DO_COMMAND |
                               i4_device_class::FLAG_END_COMMAND);
      
    if (redraw_later.waiting)
    {
      i4_time_dev.cancel_event(redraw_later.id);
      redraw_later.waiting=i4_F;
    }

    if (g1_map_is_loaded())
      g1_destroy_map();

    g1_global_id.init();

    g1_unload_object_defaults();

    g1_resources.cleanup();
    g1_unload_images();


    g1_cwin_man->destroy_views();
    g1_cwin_man->uninit();


    r1_destroy_api(g1_render.r_api);    
    g1_render.r_api=0;

    g1_input.uninit();

    g1_uninitialize_loaded_objects();

    i4_application_class::uninit();
  }

  void choice_first_level();

  char *name() { return "golgotha_app"; }
} ;

golgotha_app *g1_app=0;


li_object *g1_pause(li_object *o, li_environment *env)
{
  g1_stop_sound_averages();

  g1_resources.paused=!g1_resources.paused;
  if (g1_map_is_loaded())
    g1_get_map()->tick_time.get();
  return 0;
}

li_object *g1_set_default_level(li_object *o, li_environment *env)
{
  strcpy(first_level, li_get_string(li_eval(li_car(o,env),env),env));
  return 0;
}


void golgotha_app::pre_play_save()
{
  i4_file_class *out=i4_open(i4gets("play_savename"), I4_WRITE);
  if (out)
  {
    g1_saver_class *save=new g1_saver_class(out);

    g1_get_map()->save(save, G1_MAP_ALL);

    if (save->begin_data_write())
      g1_get_map()->save(save, G1_MAP_ALL);

    delete save;

    need_post_play_load=i4_T;
  }
}

void golgotha_app::post_play_load()
{
  i4_str *old_name=new i4_str(g1_get_map()->get_filename());
  if (g1_load_level(i4gets("play_savename"), 0))
    g1_get_map()->set_filename(*old_name);

  delete old_name; 

  need_post_play_load=i4_F;
}


void golgotha_app::do_options()
{ 
//   if (main_menu)
//     delete main_menu;

//   main_menu=new g1_option_window(wm->width(), wm->height(), wm->get_style(), this);
//   wm->add_child(0,0,main_menu);
}


void golgotha_app::network_menu()
{ 
  if (main_menu)
    delete main_menu;

  main_menu=new g1_startup_window(wm->width(), wm->height(), wm->get_style(), protocol);
  wm->add_child(0,0,main_menu);
}

void golgotha_app::help_screen(char *screen_name, int next_id)
{
  if (main_menu)
    delete main_menu;

  main_menu=new g1_help_screen_class(wm->width(), wm->height(), wm->get_style(),
                                     i4gets(screen_name), next_id);

  wm->add_child(0,0,main_menu);
}

void golgotha_app::plot_screen()
{
  if (main_menu)
    delete main_menu;

  main_menu=new g1_help_screen_class(wm->width(), wm->height(), wm->get_style(),
                                     i4gets("plot_screen"), G1_MAIN_MENU);

  wm->add_child(0,0,main_menu);
}

void golgotha_app::server_menu()
{ 
  if (main_menu)
    delete main_menu;


  main_menu=new g1_server_start_window(wm->width(), wm->height(), 
                                       wm->get_style(), protocol);
  wm->add_child(0,0,main_menu);
}

void golgotha_app::client_wait_menu()
{ 
  if (main_menu)
    delete main_menu;

  main_menu=new g1_client_wait_window(wm->width(), wm->height(), 
                                      wm->get_style(),  protocol);
  wm->add_child(0,0,main_menu);
}



void golgotha_app::do_main_menu()
{

  sw32 ids[]={ G1_START_NEW_GAME,
               G1_NO_MESSAGE,
               G1_NO_MESSAGE,

               G1_NETWORK_MENU,
               G1_OPTIONS,
               G1_QUIT,
               -1 };

  if (main_menu)
    delete main_menu;

  main_menu=new g1_main_menu_class(wm->width(),
                                   wm->height(),
                                   this,
                                   ids,
                                   wm->get_style());
                                   
                                   
  wm->add_child(0,0,main_menu);
}

li_symbol_ref fo_sym("File/Open");

void check_fo()
{
  li_symbol *org=fo_sym.get();
  li_symbol *news=li_get_symbol("File/Open");
  
  if (org!=news)
    i4_warning("symbols off!");
}

void golgotha_app::receive_event(i4_event *ev)
{
  switch (ev->type())
  {
    case i4_event::DO_COMMAND :
    {
      fo_sym.get();

      g1_cwin_man->receive_event(ev);      
      char *cmd=((i4_do_command_event_class *)ev)->command;
      li_symbol *s=li_get_symbol(cmd);
      if (li_get_fun(s,0))
        li_call(s); 

      g1_input.receive_event(ev);
    } break;

    case i4_event::END_COMMAND :
    {
      g1_cwin_man->receive_event(ev);
      char cmd[200];
      sprintf(cmd, "-%s",((i4_do_command_event_class *)ev)->command);
      li_symbol *s=li_get_symbol(cmd);
      if (li_get_fun(s,0))
        li_call(cmd);
      g1_input.receive_event(ev);
    } break;
      
    case i4_event::USER_MESSAGE :
    {
      CAST_PTR(uev,i4_user_message_event_class,ev);
      switch (uev->sub_type)
      {
       case G1_MAIN_MENU :
          do_main_menu();
          break;

        case G1_NETWORK_MENU :
          network_menu();  
          break;
        case G1_SERVER_MENU :
          server_menu();
          break;
        case G1_CLIENT_JOINED_MENU :
          client_wait_menu();
          break;

        case G1_YOU_LOSE :
          help_screen("youlose_screen", G1_QUIT);
          break;
          
        case G1_YOU_WIN :
          help_screen("youwin_screen", G1_QUIT);
          break;         

        case G1_HELP_SCREEN :
          help_screen("help_screen", G1_RETURN_TO_GAME);
          break;

        case G1_START_HELP_SCREEN :
          help_screen("help_screen", G1_START_NEW_GAME);
          break;

        case G1_PLOT_SCREEN :
          plot_screen();
          break;

        case G1_START_NEW_GAME :
          start_new_game(); 
          break;

        case G1_RETURN_TO_GAME :
          return_to_game();
          break;

        case G1_OPTIONS :
          do_options();
          break;

        case G1_QUIT :
          quit();
          break;
        case G1_ESCAPE :
          quit();
          break;

        case G1_MAP_CHANGED :
          map_changed();
          break;

        case G1_GET_ROOT_IMAGE :
        {
          CAST_PTR(get, g1_get_root_image_event, ev);
          get->result=display->get_screen();
        } break;


        case G1_REDRAW_LATER :
        {
          redraw_later.waiting=i4_F;
          li_call("redraw_all");
        } break;

        case G1_PLAY_MOVIE :
          if (g1_map_is_loaded())
          {
            pre_play_save();

            if (g1_get_map()->start_movie())
            {
              playing_movie=i4_T;            
              g1_resources.paused=i4_F;
            }

          } break;

        case G1_STOP_MOVIE :
        {
          if (playing_movie)
          {
            if (g1_map_is_loaded())
            {
              g1_get_map()->stop_movie();
              
              post_play_load();
            }

            playing_movie=i4_F;

            if (g1_current_controller.get() &&
                g1_current_controller->view.get_view_mode()==G1_CAMERA_MODE)
              g1_current_controller->view.suggest_camera_mode(G1_ACTION_MODE);
          }
         
        } break;

        case G1_GET_CURRENT_MOVIE :
        {
          if (g1_map_is_loaded())
          {
            CAST_PTR(mev, g1_get_current_movie_event, ev);
            mev->mflow=g1_get_map()->current_movie;
          }

        } break;

        case G1_INTERLACE_PIXEL :
        {
          g1_cwin_man->destroy_views();
          g1_resources.render_window_expand_mode = R1_COPY_1x1_SCANLINE_SKIP;
          g1_cwin_man->create_views();            
        } break;

        case G1_DOUBLE_PIXEL :
        {
          g1_cwin_man->destroy_views();
          g1_resources.render_window_expand_mode = R1_COPY_2x2;
          g1_cwin_man->create_views();            
        } break;

        case G1_NORMAL_PIXEL :
        {
          g1_cwin_man->destroy_views();
          g1_resources.render_window_expand_mode = R1_COPY_1x1;
          g1_cwin_man->create_views();            
        } break;

      }
    } break;

    default:
      i4_application_class::receive_event(ev);
  }
}

void golgotha_app::choice_first_level()
{

  for (int i=1; i<argc; i++)
    if (argv[i]=="-f")
    {
      i4_os_string(*(argv+i+1), first_level, 80);
      i++;
    }
    else if (argv[i]==i4_const_str("-edit"))
    {
      start_in_editor=1;
    }
    else if (argv[i]=="-eval")
    {
      i++;
      char buf[1000];
      i4_os_string(argv[i], buf, 1000);
      char *s=buf;
      li_eval(li_get_expression(s,0), 0);
    }
    else if (argv[i]=="-frame_lock")
      frame_locked_mode=1;
}



static i4_event_handler_reference_class<i4_parent_window_class> loading_window;

void golgotha_app::init()
{
  sw32 i;

  for (i=0; i<argc; i++)
  {
    if (argv[i] == "-max_memory")
    {
      i++;
      i4_const_str::iterator p(argv[i].begin());
      _max_memory = (w32)p.read_number();
    }
  }


  memory_init();


  resource_init("resource.res",0);
  strcpy(first_level, "test.level");
  li_add_function("set_default_level", g1_set_default_level);

  li_load("scheme/start.scm");

  i4_image_class *im=i4_load_image("bitmaps/comic1.jpg");

  display_init();  

  g1_sound_man.poll(i4_T);             // update sound effects (play next narative sfx)
  g1_sound_man.loop_current_song=i4_T;


  wm->set_background_color(0);



  if (im)
  {
    loading_window=new i4_image_window_class(im, i4_T, i4_F);
    wm->add_child(wm->width()/2-loading_window->width()/2,
                  wm->height()/2-loading_window->height()/2,
                  loading_window.get());
  }
  refresh();     // show something right away

  char *font_fname=li_get_string(li_get_value("font"),0);
  i4_image_class *fim=i4_load_image(font_fname);
  if (!fim) i4_error("image load failed : %s", font_fname);  
  i4_current_app->get_style()->font_hint->normal_font=new i4_anti_proportional_font_class(fim);  
  delete fim;




  choice_first_level();
  
  g1_render.r_api = r1_create_api(display);
  
  if (!g1_render.r_api)
    i4_error("Could not initialize a rendering device");  


  g1_input.init();  

  g1_resources.load();
  li_load("scheme/preferences.scm");
  
  i4_bool movie_time = i4_F;

  for (i=0; i<argc; i++)
  {
    if (argv[i] == i4gets("movie_option"))
      movie_time = i4_T;        
  }

  if (g1_render.r_api->get_render_device_flags() & R1_SOFTWARE)
  {
    //setup a timer and decide whether or not to do double pixel
    //default is high res mode
    int processor_speed = i4_get_clocks_per_second();
  
    g1_resources.render_window_expand_mode = R1_COPY_1x1;
    /*
    if (processor_speed > 170000000)
	    g1_resources.render_window_expand_mode = R1_COPY_1x1;
    else
    if (processor_speed > 140000000)
      g1_resources.render_window_expand_mode = R1_COPY_1x1_SCANLINE_SKIP;
    else    
      g1_resources.render_window_expand_mode = R1_COPY_2x2;

    g1_resources.radius_mode = g1_resource_class::VIEW_LOW;
    */

    if (movie_time)
      g1_resources.radius_mode = g1_resource_class::VIEW_FAR;

  }
  else
  {
    //hardware rasterizer, view distance = far by default
    g1_resources.radius_mode = g1_resource_class::VIEW_FAR;
  }

  s1_load();
  g1_load_images();


  g1_player_man.init_colors(&display->get_screen()->get_pal()->source, g1_render.r_api);

  protocol=i4_get_first_protocol();

  i4_graphical_style_class *style=wm->get_style();
  

  /*
  style->color_hint->button.active.bright=(135<<8)|64;
  style->color_hint->button.active.medium=(114<<8)|49;
  style->color_hint->button.active.dark=(88<<8)|38;

  style->color_hint->button.passive.bright=(120<<8)|64;
  style->color_hint->button.passive.medium=(100<<8)|49;
  style->color_hint->button.passive.dark=(70<<8)|38;
  */

  if (!g1_cwin_man) 
    g1_cwin_man=new g1_cwin_man_class;      

  g1_cwin_man->init(wm, style, display->get_screen(), display, wm);
  li_add_function("Pause", g1_pause);

//   main_menu=new g1_help_screen_class(wm->width(), wm->height(), wm->get_style(),
//                                      i4gets("startup_screen"), G1_PLOT_SCREEN);

  main_menu=0;

//   i4_const_str *tlist=i4_string_man.get_array("texture_array");
//   g1_load_texture_list(tlist);
//   i4_free(tlist);


  start_new_game();
  if (start_in_editor)
    g1_cwin_man->set_edit_mode(i4_T);


  i4_kernel.request_events(this,
                           i4_device_class::FLAG_DO_COMMAND |
                           i4_device_class::FLAG_END_COMMAND);
      
  //  do_main_menu();


  // start game with startup screens
//   if (!movie_time)
//   {
//     help_screen("startup_screen", G1_START_HELP_SCREEN);
//   }
//   else
//   {
//     start_new_game();
//   }

}
  

void golgotha_app::return_to_game()
{
  if (!g1_map_is_loaded())
    start_new_game();
  else if (main_menu)
  {
    delete main_menu;
    main_menu=0;
  }    
}

void golgotha_app::start_new_game()
{
  if (main_menu)
  {
    delete main_menu;
    main_menu=0;
  }

  g1_cwin_man->destroy_views();
  
  int no_demo=1;
  for (int i=1; i<argc; i++)
  {
    if (argv[i]=="-no_demo")
      no_demo=1;
    if (argv[i]=="-demo")
      li_call("play_demo");
    else if (argv[i]=="-df")
    {
      i++;
      char fname[100];
      i4_os_string(argv[i], fname,256);
      li_call("play_demo", li_make_list(new li_string(fname), 0));            
    }
  }

  // demo verision for sampler CD, play demo if not started already
  if (!g1_playing_demo() && !no_demo)
    li_call("play_demo");

  if (!g1_playing_demo())
    if (!g1_load_level(first_level))
      i4_error("could not load level %s", first_level);


  if (loading_window.get())
    i4_kernel.delete_handler(loading_window.get());
  
  g1_cwin_man->create_views();

  if (g1_map_is_loaded() && g1_get_map()->start_movie())
  {
    playing_movie=i4_T;            
    g1_resources.paused=i4_F;
    
    if (g1_current_controller.get())
      g1_current_controller->view.suggest_camera_mode(G1_CAMERA_MODE);

  }

  g1_sound_man.loop_current_song=i4_F;
  g1_sound_man.next_song();
}

void i4_main(w32 argc, i4_const_str *argv)
{  
  golgotha_app app(argc, argv);
  g1_app=&app;
  app.run();  
}


