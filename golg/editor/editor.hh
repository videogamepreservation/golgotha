/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_EDITOR_HH
#define G1_EDITOR_HH

#include "editor/contedit.hh"
#include "math/num_type.hh"
#include "device/event.hh"
#include "device/device.hh"
#include "string/string.hh"
#include "editor/pmenu.hh"
#include "objs/model_id.hh"
#include "map.hh"
#include "mess_id.hh"
#include "cwin_man.hh"
#include "editor/dialogs/scene.hh"
#include "math/vector.hh"
#include "controller.hh"
#include "editor/dialogs/debug_win.hh"
#include "map_man.hh"

class r1_render_window_class;
class i4_pull_menu_class;
class g1_editor_key_item;
class r1_render_api_class;
class g1_object_class;
class i4_image_class;
class g1_movie_flow_class;
class g1_time_line;
class i4_text_input_class;
class g1_path_window_class;

class g1_editor_class : public g1_cwin_man_class
{
  struct undo_info
  {
    int head, tail, redo_tail, max, allow;
    undo_info() 
    { 
      allow=1;
      head=tail=redo_tail=0; 
      max=32;
    }
  } undo;

  void show_undo_state();
  void save_undo_info(w32 sections, i4_const_str &fname);


  i4_window_class **tool_window;
  g1_time_line *time_line;

  int t_views;
  int vert_noise_amount;
  enum { MAX_VIEWS=4};
  g1_controller_edit_class *views[MAX_VIEWS];
  r1_render_window_class *view_wins[MAX_VIEWS];

  i4_bool use_view_state;
  g1_view_state_class view_states[MAX_VIEWS];

  void set_edit_mode(i4_bool yes_no);

  i4_bool need_save, can_undo, can_redo, selection, paste_buffer_valid, edit_mode,
    profile_view, debug_view;

  i4_image_class *pick_act, *pick_pass;
  
  g1_path_window_class *path_window;

  i4_parent_window_class *objects_window;

  i4_event_handler_reference_class<i4_parent_window_class> radar_parent;
  void create_radar();

  i4_string_manager_class e_strs;

  g1_map_class *create_default_map(int w, int h,
                                   const i4_const_str &name);

  i4_pull_menu_class *menu;
  g1_editor_key_item *command_keys[G1_EDITOR_LAST];
  void save();


  virtual void init(i4_parent_window_class *parent,
                    i4_graphical_style_class *style,
                    i4_image_class *root_window,
                    i4_display_class *display,
                    i4_window_manager_class *wm);


  virtual void uninit();

  // this will load an image an convert it to be a compatable bit depth with the frame buffer
  i4_image_class *load_compatiable_image(const i4_const_str &fname);

  void create_objects_window();
  void get_map_cell_from_type(w32 type, g1_map_cell_class &ct);
  void close_windows();

  struct window_info 
  {    
    sw32 x,y;   
    window_info() { x=0; y=0; }
    void save(i4_window_class *w) { x=w->x(); y=w->y(); }
  };

  window_info wi_scene, wi_objects;
  
  g1_scene_editor_class scene_edit;

  void merge_terrain();
  i4_bool merge_terrain_ok(i4_user_message_event_class *ev);

  void open_file();
  void open_dll();

  void open_file_ok(i4_user_message_event_class *ev);
  void open_dll_ok(i4_user_message_event_class *ev);

  void open_saveas();
  void open_saveas_ok(i4_user_message_event_class *ev);

  void open_resize_level_window();
  void resize_level();

  void open_new_level_window();  

  void new_level_from_dialog();

  struct { i4_text_input_class *w, *h; int orient; } resize_dialog;
  struct { i4_text_input_class *amount; } terrain_noise_dialog;
  struct { i4_text_input_class *w, *h, *name; } new_dialog;


  struct { i4_text_input_class *name; } sky_dialog;
  void create_sky_window();
  void set_sky();

  void load_height_bitmap();
  void save_height_bitmap();
  void load_height_bitmap_ok(i4_event *ev);
  void save_height_bitmap_ok(i4_event *ev);

public:
  struct movement_info
  {
    i4_bool pan_up, pan_down, 
      pan_left, pan_right, 
      pan_forward, pan_backward,
      rotate_left, rotate_right,
      rotate_up, rotate_down;
  } movement;


  // used for dialog boxes that need to be completed
  i4_event_handler_reference_class<i4_parent_window_class> modal_window;    

  void close_modal();
  i4_parent_window_class *create_modal(w32 w, w32 h, char *title_res);

  void add_undo(w32 sections);
  void do_undo();
  void do_redo();

  i4_bool have_map() { return g1_map_is_loaded(); }
  g1_map_class *get_map() { return g1_get_map(); }

  i4_image_class *delete_icon;

  
  virtual void create_views();

  virtual void destroy_views();

  void redraw_all();
  const i4_const_str &get_editor_string(char *res_name) { return e_strs.get(res_name); }
  i4_const_str *get_editor_array(char *res_name) { return e_strs.get_array(res_name); }

  // the pointer 'window' is passed via reference because it will be set to 0,
  // when the tool window is replaced (at a later time)
  void set_tool_window(i4_window_class **window);

  void changed();
  void do_command(w16 command_id);
  void select_all_verts();

  g1_editor_class();

  virtual void receive_event(i4_event *ev);


  void major_mode_change()
  {
    if (edit_mode)
      for (w32 i=0; i<t_views; i++)
        views[i]->setup_mode();
  }

  i4_bool in_editor() { return edit_mode; }
  void flatten_terrain();

  void smooth_terrain();
  void smooth_terrain_ok();

  void noise_terrain();
  void noise_terrain_ok();

  int get_current_splines(i4_spline_class **buffer, int buf_size);

  void mark_selected_verts_for_undo_save();
  void unmark_all_selected_verts_for_undo_save();

  virtual void save_views(g1_saver_class *fp);
  virtual void load_views(g1_loader_class *fp);


  g1_controller_edit_class *get_current_view();
};

extern g1_editor_class g1_editor_instance;


#endif
