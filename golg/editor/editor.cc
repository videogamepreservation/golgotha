/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "editor/editor.hh"
#include "controller.hh"
#include "map.hh"
#include "tile.hh"
#include "window/win_evt.hh"
#include "string/string.hh"
#include "draw_context.hh"
#include "font/font.hh"
#include "window/style.hh"
#include "saver.hh"
#include "editor/pmenu.hh"
#include "menu/pull.hh"
#include "resources.hh"
#include "editor/dialogs/obj_win.hh"
#include "math/pi.hh"
#include "mess_id.hh"
#include "loaders/load.hh"
#include "editor/edit_id.hh"
#include "editor/contedit.hh"
#include "editor/e_state.hh"
#include "editor/dialogs/e_time.hh"
#include "m_flow.hh"
#include "editor/dialogs/scene.hh"
#include "light.hh"
#include "math/spline.hh"
#include "file/get_filename.hh"

#include "time/gui_prof.hh"
#include "border_frame.hh"
#include "remove_man.hh"
#include "status/status.hh"
#include "status/gui_stat.hh"
#include "editor/dialogs/tile_picker.hh"
// #include "editor/dialogs/path_win.hh"
#include "editor/dialogs/debug_win.hh"
#include "r1_api.hh"
#include "g1_render.hh"
#include "gui/create_dialog.hh"
#include "gui/text_input.hh"
#include "r1_win.hh"
// #include "critical_graph.hh"
#include "app/app.hh"

#include "editor/mode/e_tile.hh"
#include "editor/mode/e_object.hh"
#include "dll/dll_man.hh"
#include "window/wmanager.hh"
#include "editor/e_res.hh"
#include "lisp/lisp.hh"
#include "lisp/li_init.hh"
#include "gui/li_pull_menu.hh"
#include "player.hh"

#include "level_load.hh"
#include "map_vert.hh"
#include "lisp/li_dialog.hh"
#include "map_view.hh"
#include "main/main.hh"

g1_editor_class g1_editor_instance;


int g1_controller_edit_class::get_current_splines(i4_spline_class **buffer, int buf_size)
{
  return g1_editor_instance.get_current_splines(buffer, buf_size);
}

void g1_editor_class::create_radar()
{
  if (!radar_parent.get())
  {
    g1_map_class *map=g1_get_map();
    i4_parent_window_class *mv=g1_create_radar_view(map->width(), map->height(),
                                                    G1_RADAR_CLICK_HOLDS_VIEW |
                                                    G1_RADAR_DRAW_ALL_PATHS);



    i4_parent_window_class *p;
    int y=i4_current_app->get_window_manager()->height()-mv->height()-30;
    p=i4_current_app->get_style()->create_mp_window(0, y,
                                                    mv->width(), mv->height(),
                                                    g1_ges("radar_title"));
    p->add_child(0,0, mv);
    radar_parent=p;
  }  
}

int g1_editor_class::get_current_splines(i4_spline_class **buffer, int buf_size)
{
  if (!have_map() || !get_map()->current_movie || !get_map()->current_movie->t_cut_scenes)
    return 0;

  g1_cut_scene_class *s=get_map()->current_movie->current();

  if (s && buf_size>g1_cut_scene_class::T_PATHS)
  {
    for (int i=0; i<g1_cut_scene_class::T_PATHS; i++)
      buffer[i]=&s->paths[i];

    return g1_cut_scene_class::T_PATHS;
  }

  return 0;
}




void g1_editor_class::create_views()
{
  if (edit_mode)
  {
    sw32 x1=0,
      x2=parent->width()-1-g1_edit_state.tools_width(),
      y1=style->font_hint->normal_font->largest_height()+8;

    int y2=parent->height()-1;
    if (time_line) y2-=time_line->height();

    w32 div;
    for (div=1; div*div<t_views; div++);

    sw32 x=x1, y=y1, w=(x2-x1+1)/div, h=(y2-y1+1)/div;


    for (w32 i=0; i<t_views; i++)
    {
      r1_expand_type expand=(r1_expand_type) g1_resources.render_window_expand_mode;

      view_wins[i]=g1_render.r_api->create_render_window(w,h, expand);
                                                                         
      views[i]=new g1_controller_edit_class(view_wins[i]->render_area_width(),
                                            view_wins[i]->render_area_height(), style);

      view_wins[i]->add_child(0,0,views[i]);
      parent->add_child_front(x,y, view_wins[i]);

      if (i==1)
      {
        x=x1;
        y+=h;
      }
      else x+=w;

      views[i]->view=view_states[i];
      views[i]->view.suggest_camera_mode(G1_EDIT_MODE);
    }
    
    g1_current_controller=views[0];
  }
  else
    g1_cwin_man_class::create_views();
}

void g1_editor_class::destroy_views()
{
  if (edit_mode)
  {
    for (w32 i=0; i<t_views; i++)
    {
      views[i]->hide_focus();
      views[i]->view=view_states[i];

      delete view_wins[i];
      views[i]=0;
      view_wins[i]=0;
    }      


  } else g1_cwin_man_class::destroy_views();
}
  



void g1_editor_class::close_windows()
{
  scene_edit.hide();

  for (g1_mode_creator *mc=g1_mode_creator::first; mc; mc=mc->next)
    mc->cleanup();

  if (radar_parent.get())
    i4_kernel.delete_handler(radar_parent.get());
}




void g1_editor_class::set_edit_mode(i4_bool yes_no)
{
  if (yes_no)
  {
    // remove play save name so game doesn't try to load movie start
    i4_unlink(i4gets("play_savename"));
    i4_user_message_event_class movie_stop(G1_STOP_MOVIE);  // stop game movie if there was one
    i4_kernel.send_event(i4_current_app, &movie_stop);

    changed();

    g1_cwin_man_class::destroy_views();   // kill the normal game views
    edit_mode=i4_T;

    g1_change_key_context(G1_EDIT_MODE);

    if (menu) delete menu;
    menu=li_create_pull_menu("scheme/menu.scm");
    menu->show(parent, 0,0);
    
    //time_line=new g1_time_line(parent, style, &g1_edit_state);

    t_views=4;

    int th=parent->height();
    if (time_line) th-=time_line->height();
    g1_edit_state.show_tools(parent, th);
    
    create_views();    // create sup'd up edit controllers

    g1_change_key_context(G1_EDIT_MODE);

    li_call("Pause");  // stop the game from running while the editor is going

  } else
  {
    menu->hide();
    delete menu;
    menu=0;
    
    close_windows();

    destroy_views();

    edit_mode=i4_F;


    if (time_line)
      delete time_line;

    g1_edit_state.hide_tools();
    
    g1_cwin_man_class::create_views();   // create the normal game views

    

    li_call("Pause");     // start the game running again
  }
}


g1_editor_class::g1_editor_class()
{
  memset(&movement, 0, sizeof(movement));

  for (int i=0; i<MAX_VIEWS; i++)
    view_states[i].suggest_camera_mode(G1_EDIT_MODE);

  vert_noise_amount=3;
  use_view_state=i4_F;

  delete_icon=0;

  tool_window=0;
  time_line=0;

  menu=0;

  edit_mode=i4_F;
  need_save=i4_F;
  can_undo=i4_F;
  can_redo=i4_F;
  selection=i4_F;
  profile_view=i4_F;
  paste_buffer_valid=i4_F;

  objects_window=0;
  views[0]=0;

  modal_window=0;

  g1_cwin_man=this;
}

void g1_editor_class::changed()
{
  need_save=i4_T;
}


void g1_editor_key_item::action()
{
  g1_editor_instance.do_command(command_id);
}


void g1_editor_class::save()
{
  if (have_map())
  {
    get_map()->recalc_static_stuff();

    i4_bool restore_edit_mode=i4_T;
    
    g1_edit_state.hide_focus();
    get_map()->recalc_static_stuff();

    i4_file_class *out=i4_open(get_map()->get_filename(), I4_WRITE);
    if (out)
    {
      g1_saver_class *save=new g1_saver_class(out);

      get_map()->save(save, G1_MAP_ALL);

      if (save->begin_data_write())
        get_map()->save(save, G1_MAP_ALL);

      delete save;
    }


    g1_edit_state.show_focus();

    need_save=i4_F;
  }
}

void g1_editor_class::do_command(w16 command_id)
{
  switch (command_id)
  {
    case G1_EDITOR_UNDO :
      do_undo();
      break;

    case G1_EDITOR_REDO :
      do_redo();
      break;
      
    case G1_EDITOR_NEW :
      open_new_level_window();
      break;

    case G1_EDITOR_SAVE :
    {
      save();      
    } break;

    case G1_EDITOR_SAVEAS :
    {
      open_saveas();
    } break;
      
    case G1_EDITOR_LOAD :
      open_file();
      break;

    case G1_EDITOR_OPEN_DLL :
      open_dll();
      break;

    case G1_EDITOR_EXIT :
    {
      i4_user_message_event_class ex(G1_QUIT);
      i4_kernel.send_event(i4_current_app, &ex);
    } break;

    case G1_EDITOR_TOGGLE :
      set_edit_mode((i4_bool)!edit_mode);
      break;


    case G1_EDITOR_TILE_PICKER :
      if (g1_map_is_loaded())
        g1_e_tile.open_picker();
      break;


    case G1_EDITOR_WINDOW_RADAR :
      if (g1_map_is_loaded())
      {
        if (radar_parent.get())
          i4_current_app->get_style()->close_mp_window(radar_parent.get());
        else
          create_radar();
      }
      break;

    case G1_EDITOR_WINDOW_OBJECTS :
      if (g1_map_is_loaded())
        li_call("toggle_object_picker");

      break;


    
//     case G1_EDITOR_AI_WINDOW:
//     {
//       if (!path_window)
//       {
//         enum { MAX_NUM=40 };

//         get_map()->get_critical_graph()->expand_critical_graph();

//         i4_image_class *img[MAX_NUM];

//         i4_const_str::iterator i1=get_editor_string("path_start").begin();
//         img[0] = g1_edit_state.get_icon(i1.read_number());

//         i1=get_editor_string("path_dest").begin();
//         img[1] = g1_edit_state.get_icon(i1.read_number());

//         i1=get_editor_string("path_critical").begin();
//         img[2] = g1_edit_state.get_icon(i1.read_number());

//         path_window = new g1_path_window_class(get_map(), img);

//         // Create MPWindow
//         i4_parent_window_class *mpw;
//         i4_user_message_event_class *close=
//           new i4_user_message_event_class(G1_EDITOR_AI_WINDOW_CLOSED);
//         mpw=style->
//           create_mp_window(-1,-1,
//                            path_window->width()+40,
//                            path_window->height(),
//                            e_strs.get("ai_window_title"),
//                            new i4_event_reaction_class(this, close));
//         mpw->add_child(40,0, path_window);
        
//         // Create Toolbar
//         i4_const_str *nums;
//         i4_const_str *help_names[MAX_NUM];

//         nums = get_editor_array("path_tool_win_icons");
//         if (nums)
//         {
//           int n=0;
//           for (i4_const_str* p=nums;  !p->null();  n++)
//           {
//             i4_const_str::iterator i1=p->begin();
//             img[n] = g1_edit_state.get_icon(i1.read_number());
//             ++p;

//             help_names[n] = p;
//             ++p;
//           }

//           g1_path_tool_window_class *path_tool = 
//             new g1_path_tool_window_class(style, path_window, n, img, help_names );
//           mpw->add_child(0,0, path_tool);
//           mpw->resize_to_fit_children();

//           i4_free(nums);
//         }
//       }
//     } break;


    case G1_EDITOR_RESIZE_MAP :
      open_resize_level_window();
      break;

    case G1_EDITOR_WINDOW_SCENES :
      if (have_map() && get_map()->get_current_movie())
      {
        scene_edit.show(parent, get_map()->get_current_movie(), style);
      }
      break;

    case G1_EDITOR_SELECT_ALL_VERTS :
      select_all_verts();
      break;

    case G1_EDITOR_1_VIEW:
    {
      destroy_views();
      t_views=1;
      create_views();
    } break;

    case G1_EDITOR_TICK_MAP:
    {
      if (edit_mode && have_map())
      {
        get_map()->think_objects();
        g1_remove_man.process_requests();
        li_call("redraw");
      }
    } break;

    case G1_EDITOR_PROFILE:
    {
      sw32 x=0, y=0;
      i4_user_message_event_class *ue=new i4_user_message_event_class(G1_EDITOR_PROFILE_CLOSED);
      i4_event_reaction_class *prof_closed;
      prof_closed=new i4_event_reaction_class(this, ue);
      i4_profile_watch(style, parent, x,y, 200,280,!profile_view, prof_closed);
      profile_view=!profile_view;
    } break;

    case G1_EDITOR_DEBUG:
    {
      if (debug_view) {
	g1_debug_close(style);
	debug_view = 0;
      }
      else {
	i4_user_message_event_class *ue=
	  new i4_user_message_event_class(G1_EDITOR_DEBUG_CLOSED);
	i4_event_reaction_class *debug_closed=
	  new i4_event_reaction_class(this, ue);
	g1_debug_open(style, parent, g1_ges("debug_title"), 3, debug_closed);
	debug_view=1;
      }
    } break;

    case G1_EDITOR_4_VIEWS:
    {
      destroy_views();
      t_views=4;
      create_views();
    } break;

    case G1_EDITOR_RECALC_LIGHT :
//       if (map)
//         calc_map_lighting(0,0, get_map()->width(), get_map()->height());
      break;

    case G1_EDITOR_MERGE_TERRAIN :
      merge_terrain();
      break;

    case G1_EDITOR_FLATTEN_TERRAIN :
      flatten_terrain();
      break;

    case G1_EDITOR_SMOOTH_TERRAIN :
      smooth_terrain();
      break;

    case G1_EDITOR_NOISE_TERRAIN :
      noise_terrain();
      break;

    case G1_EDITOR_LOAD_HEIGHT_MAP :
      load_height_bitmap();
      break;

    case G1_EDITOR_SAVE_HEIGHT_MAP :
      save_height_bitmap();
      break;

    case G1_EDITOR_FOG_ALL :
      break;
      
    case G1_EDITOR_FOG_NONE :
      break;

    case G1_EDITOR_SET_SKY :
      create_sky_window();      // defined in editor/sky.cc
      break;

    case G1_EDITOR_SNAP_CENTER :
      g1_edit_state.snap=g1_edit_state_class::SNAP_CENTER;
      break;

    case G1_EDITOR_NO_SNAP :
      g1_edit_state.snap=g1_edit_state_class::NO_SNAP;
      break;

    case G1_EDITOR_SNAP_ORIGIN :
      g1_edit_state.snap=g1_edit_state_class::SNAP_ORIGIN;
      break;


  }
}

void g1_editor_class::close_modal()
{
  if (modal_window.get())
  {
    style->close_mp_window(modal_window.get());
    modal_window=0;
  }
}



char *cmd_2_enum[]=
{
  "File/New",                    // G1_EDITOR_NEW,
  "File/Save",                   // G1_EDITOR_SAVE,
  "File/Save As",                // G1_EDITOR_SAVEAS,
  "File/Open",                   // G1_EDITOR_LOAD,
  "",                            // G1_EDITOR_MERGE_TERRAIN,
  "File/Exit",                   // G1_EDITOR_EXIT,

  "Edit/Undo",                   // G1_EDITOR_UNDO,
  "Edit/Redo",                   // G1_EDITOR_REDO,
  "",                            // G1_EDITOR_CUT,
  "",                            // G1_EDITOR_COPY,
  "",                            // G1_EDITOR_PASTE,
  "Edit/Toggle Menu",            // G1_EDITOR_TOGGLE,
  "Edit/No Snap",                // G1_EDITOR_NO_SNAP,
  "Edit/Snap Cell Center",       // G1_EDITOR_SNAP_CENTER,
  "Edit/Snap Cell Origin",       // G1_EDITOR_SNAP_ORIGIN,


  "Tools/Objects",               // G1_EDITOR_WINDOW_OBJECTS,
  "Tools/Scenes",                // G1_EDITOR_WINDOW_SCENES,
  "Tools/Radar",                 // G1_EDITOR_WINDOW_RADAR,
  "View/1 View",                 // G1_EDITOR_1_VIEW,
  "View/4 Views",                // G1_EDITOR_4_VIEWS,
  "Tools/Tiles",                 // G1_EDITOR_TILE_PICKER,

  "",                            // G1_EDITOR_RECALC_LIGHT,
  "Terrain/Select All",          // G1_EDITOR_SELECT_ALL_VERTS,
  "Map/Resize",                  // G1_EDITOR_RESIZE_MAP,
  "Map/Change Sky",              // G1_EDITOR_SET_SKY,

  "Map/Fog Out",                 // G1_EDITOR_FOG_ALL,
  "Map/Unfog",                   // G1_EDITOR_FOG_NONE,

  "Map/Simulate Tick",           // G1_EDITOR_TICK_MAP,
  "Tools/Profile",               // G1_EDITOR_PROFILE,
  "Tools/Debug",                 // G1_EDITOR_DEBUG,
  "Tools/AI",                    // G1_EDITOR_AI_WINDOW,

  "Terrain/Load Image Heightmap",// G1_EDITOR_LOAD_HEIGHT_MAP,
  "Terrain/Save Image Heightmap",// G1_EDITOR_SAVE_HEIGHT_MAP,

  "Terrain/Flatten Selected",    // G1_EDITOR_FLATTEN_TERRAIN,
  "Terrain/Smooth Selected",     // G1_EDITOR_SMOOTH_TERRAIN,
  "Terrain/Add Noise to Selected",  // G1_EDITOR_NOISE_TERRAIN,

  "File/Open DLL",               // G1_EDITOR_OPEN_DLL,
  0};

  

// these ID's are defined in mess_id.hh
void g1_editor_class::receive_event(i4_event *ev)
{
  if (ev->type()==i4_event::USER_MESSAGE)
  {
    CAST_PTR(uev, i4_user_message_event_class, ev);
    switch (uev->sub_type)
    {            
      case G1_EDITOR_TERRAIN_NOISE_OK :
        noise_terrain_ok();
        break;


      case G1_EDITOR_SET_SKY_OK :
        set_sky();                // defined in editor/sky.cc
        break;

      case G1_TMERGE_FILE_OPEN_OK :
        merge_terrain_ok(uev);
        break;

      case G1_FILE_OPEN_OK :
        open_file_ok(uev);
        break;

      case G1_OPEN_DLL_OK :
        open_dll_ok(uev);
        break;


      case G1_FILE_SAVEAS_OK :
        open_saveas_ok(uev);
        break;

      case G1_EDITOR_LOAD_TERRAIN_HEIGHT_OK :
        load_height_bitmap_ok(ev);
        break;

      case G1_EDITOR_SAVE_TERRAIN_HEIGHT_OK :
        save_height_bitmap_ok(ev);
        break;
          
      case G1_EDITOR_RESIZE_PLACE_LT :
      case G1_EDITOR_RESIZE_PLACE_CT :
      case G1_EDITOR_RESIZE_PLACE_RT :
      case G1_EDITOR_RESIZE_PLACE_LC :
      case G1_EDITOR_RESIZE_PLACE_CC :
      case G1_EDITOR_RESIZE_PLACE_RC :
      case G1_EDITOR_RESIZE_PLACE_BL :
      case G1_EDITOR_RESIZE_PLACE_BC :
      case G1_EDITOR_RESIZE_PLACE_BR :
        resize_dialog.orient=uev->sub_type-G1_EDITOR_RESIZE_PLACE_LT;
        break;


      case G1_EDITOR_RESIZE_MAP_OK :
        resize_level();
        break;

      case G1_EDITOR_NEW_OK :
        new_level_from_dialog();
        break;
        

      case G1_EDITOR_MODAL_BOX_CANCEL :
        close_modal();
        break;
      
      case G1_EDITOR_MODAL_CLOSED :
        modal_window=0;
        break;

        
      case G1_EDITOR_AI_WINDOW_CLOSED :
        path_window=0;
        break;
        

      case G1_EDITOR_PROFILE_CLOSED :
      {
        profile_view=0;
      } break;

      case G1_EDITOR_DEBUG_CLOSED :
      {
        debug_view=0;
      } break;
    }
  }
  else
  {      
    g1_cwin_man_class::receive_event(ev);
  }

}

i4_image_class *g1_editor_class::load_compatiable_image(const i4_const_str &fname)
{
  i4_image_class *im=i4_load_image(fname);
  if (im)
  {
    i4_draw_context_class c(0,0, im->width()-1, im->height()-1);

    i4_image_class *i=g1_render.r_api->create_compatible_image(im->width(), im->height());
    if (!i)
    {
      delete im;
      return 0;
    }

    im->put_image(i, 0,0, c);
    delete im;
    return i;     
  }
  return 0;
}


li_object *g1_old_pull_menu_command(li_object *o, li_environment *env)
{
  char *cmd_name=li_symbol::get(env->current_function(),env)->name()->value();
  
  int id=0;
  for (char **cmd=cmd_2_enum; *cmd; cmd++, id++)
    if (strcmp(*cmd, cmd_name)==0)
      g1_editor_instance.do_command(id);
  
  return 0;
}

void g1_editor_class::redraw_all()
{
  for (int i=0; i<t_views; i++)
    if (views[i])
      views[i]->request_redraw(i4_F);
}

li_object *g1_editor_redraw_all(li_object *o, li_environment *env)
{
  g1_editor_instance.redraw_all();
  return 0;
}

void g1_editor_class::init(i4_parent_window_class *_parent,
                           i4_graphical_style_class *_style,
                           i4_image_class *_root_image,
                           i4_display_class *display,
                           i4_window_manager_class *wm)

{ 
  li_add_function("redraw_all", g1_editor_redraw_all);

  for (char **cmd=cmd_2_enum; *cmd; cmd++)
    li_add_function(*cmd, g1_old_pull_menu_command);  
  
  g1_cwin_man_class::init(_parent, _style, _root_image, display, wm);

  e_strs.load(i4gets("editor_strs"));


  pick_act=load_compatiable_image(e_strs.get("pick_act"));
  pick_pass=load_compatiable_image(e_strs.get("pick_pass"));

  views[0]=0;

  menu=0;

  delete_icon=i4_load_image(e_strs.get("delete_icon"));  


  i4_bool determantistic=i4_F;
  for (int i=0; i<i4_global_argc; i++)
    if (i4_global_argv[i]=="-deterministic")
      determantistic=i4_T;

  if (!determantistic)
    i4_init_gui_status(wm, display);

}

void g1_editor_class::uninit()
{
  close_modal();

  i4_uninit_gui_status();

  for (g1_mode_creator *mc=g1_mode_creator::first; mc; mc=mc->next)
    mc->cleanup();

  if (debug_view) {
    g1_debug_close(style);
    debug_view = 0;
  }


  if (pick_act)
    delete pick_act;

  if (pick_pass)
    delete pick_pass;  

  delete menu; 
  menu=0;

  delete delete_icon;
   
  g1_edit_state.uninit();  
}

void g1_controller_edit_class::changed()
{
  g1_editor_instance.changed();   // so the user can save
}



void g1_editor_class::set_tool_window(i4_window_class **window)
{
  if (tool_window && tool_window!=window)
  {
    delete *tool_window;
    *tool_window=0;
    tool_window=0;
  }

  if (parent)
  {
    parent->add_child(parent->width()-(*window)->width(),
                      0,
                      (*window));
    tool_window=window;
  } else 
  {
    delete *window;
    window=0;
  }
}


void g1_controller_edit_class::display_edit_win()  // defined in editor.cc
{
  g1_editor_instance.set_tool_window(&edit_win);
}


void g1_controller_edit_class::move_selected_heights(sw32 z_change)
{
  g1_editor_instance.unmark_all_selected_verts_for_undo_save();
  g1_editor_instance.mark_selected_verts_for_undo_save();
  g1_editor_instance.add_undo(G1_MAP_SELECTED_VERTS);


  w32 w=(get_map()->width()+1), h=(get_map()->height()+1),c;
  g1_map_vertex_class *v=get_map()->vertex(0,0);

  i4_status_class *status = i4_create_status(g1_ges("moving vertices"));

  for (sw32 vy=0; vy<h; vy++)
  {
    if (status)
      status->update(vy/(float)h);

    for (sw32 vx=0; vx<w; vx++, v++)
    {
      if (v->is_selected())
      {
        w8 nh;

        if ((sw32)v->height+z_change<0)
          nh=0;
        else if ((sw32)v->height+z_change>255)
          nh=255;
        else
          nh=v->height+z_change;

        get_map()->change_vert_height(vx,vy, nh);
      }
    }
  }

  if (status)
    delete status;

  changed();
  refresh();
}


void g1_controller_edit_class::change_light_direction()
{
  if (g1_map_is_loaded())
  {
    g1_editor_instance.add_undo(G1_MAP_VERTS | G1_MAP_LIGHTS);


    i4_3d_vector one_z(0,0,-1), center, light;
    transform.transform(one_z, light);
    transform.transform(i4_3d_vector(0,0,0), center);
    light-=center;



    changed();
    refresh();
  }  
}

void g1_editor_class::select_all_verts()
{
  add_undo(G1_MAP_VERTS);

  sw32 w=get_map()->width(), h=get_map()->height();

  for (int j=0; j<=h; j++)
    for (int i=0; i<=w; i++)
      get_map()->vertex(i,j)->set_is_selected(i4_T);

  changed();
  li_call("redraw");
}


void g1_editor_class::open_file()
{
  i4_str *start_dir=0;

  if (g1_map_is_loaded() && !g1_get_map()->get_filename().null())
  {
    i4_filename_struct fn;
    i4_split_path(g1_get_map()->get_filename(), fn);

    if (fn.path[0])
      start_dir=new i4_str(fn.path);
  }

  if (!start_dir)
    start_dir=new i4_str(get_editor_string("open_start_dir"));

  i4_create_file_open_dialog(style,
                             get_editor_string("open_title"),
                             *start_dir,
                             get_editor_string("open_file_mask"),
                             get_editor_string("open_mask_name"),
                             this,
                             G1_FILE_OPEN_OK,
                             G1_FILE_OPEN_CANCEL);

  delete start_dir;
}


void g1_editor_class::open_dll() 
{
  i4_create_file_open_dialog(style,
                             get_editor_string("open_dll_title"),
                             i4_const_str(i4_dll_dir),
                             get_editor_string("open_dll_file_mask"),
                             get_editor_string("open_dll_mask_name"),
                             this,
                             G1_OPEN_DLL_OK,
                             G1_FILE_OPEN_CANCEL);
}



void g1_editor_class::open_saveas()
{
  if (have_map())
  {
    i4_create_file_save_dialog(style,
                               get_map()->get_filename(),
                               get_editor_string("open_title"),
                               get_editor_string("open_start_dir"),
                               get_editor_string("open_file_mask"),
                               get_editor_string("open_mask_name"),
                               this,
                               G1_FILE_SAVEAS_OK,
                               G1_FILE_OPEN_CANCEL);
    
  }
}


void g1_editor_class::open_file_ok(i4_user_message_event_class *ev)
{
  CAST_PTR(f, i4_file_open_message_class, ev);

  g1_load_level(*f->filename, 1, 0);

}

void g1_editor_class::open_dll_ok(i4_user_message_event_class *ev)
{
  CAST_PTR(f, i4_file_open_message_class, ev);
  if (f->filename)
    i4_dll_man.load(*f->filename);
}

void g1_editor_class::open_saveas_ok(i4_user_message_event_class *ev)
{
  if (have_map())
  {

    CAST_PTR(f, i4_file_open_message_class, ev);

    get_map()->set_filename(*f->filename);

    save();    
  }
}


i4_parent_window_class *g1_editor_class::create_modal(w32 w, w32 h, char *title_res)
{
  close_modal();

  i4_object_message_event_class *closed_modal;
  closed_modal=new i4_object_message_event_class(this, G1_EDITOR_MODAL_CLOSED);


  modal_window=style->create_mp_window(-1, -1, w, h,
                                       g1_ges(title_res), 
                                       new i4_event_reaction_class(this, closed_modal));

  return modal_window.get();
}


void g1_editor_class::mark_selected_verts_for_undo_save()
{
  if (have_map())
  {
    int t=(get_map()->width()+1) * (get_map()->height()+1);
    g1_map_vertex_class *v=get_map()->verts;

    for (int i=0; i<t; i++, v++)
      if (v->is_selected())
        v->set_need_undo(i4_T);
  }
}


void g1_editor_class::unmark_all_selected_verts_for_undo_save()
{
  int t=(get_map()->width()+1) * (get_map()->height()+1);
  g1_map_vertex_class *v=get_map()->verts;

  for (int i=0; i<t; i++, v++)
    v->set_need_undo(i4_F);
}




void g1_editor_class::save_views(g1_saver_class *fp)
{
  g1_cwin_man_class::save_views(fp);

  for (int i=0; i<t_views; i++)
  {
    char section_name[50];
    sprintf(section_name, "editor view %d v1", i);
    fp->mark_section(section_name);

    if (views[i])
      view_states[i]=views[i]->view;

    view_states[i].save(fp);
  }
}

void g1_editor_class::load_views(g1_loader_class *fp)
{
  g1_cwin_man_class::load_views(fp);

  if (!fp)
  {
    for (int i=0; i<MAX_VIEWS; i++)
    {
      if (views[i])
        views[i]->view.load(0);
      else
        view_states[i].load(0);
    }
  }
  else
  {
    for (int i=0; 1; i++)
    {
      char section_name[50];
      sprintf(section_name, "editor view %d v1", i);
      if (!fp->goto_section(section_name))
        return ;

      view_states[i].load(fp);
      if (views[i])
        views[i]->view=view_states[i];
    }
  }
}


const i4_const_str &g1_ges(char *res)
{ 
  return g1_editor_instance.get_editor_string(res); 
}

g1_controller_edit_class *g1_editor_class::get_current_view()
{
  for (int i=0; i<MAX_VIEWS; i++)
    if (views[i]==g1_current_controller.get())
      return views[i];
 
  return 0;  
}
