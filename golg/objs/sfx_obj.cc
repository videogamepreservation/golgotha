/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "objs/sfx_obj.hh"
#include "music/stream.hh"
#include "file/get_filename.hh"
#include "editor/editor.hh"
#include "gui/create_dialog.hh"
#include "window/colorwin.hh"
#include "gui/text_input.hh"
#include "object_definer.hh"
#include "objs/model_draw.hh"
#include "sound_man.hh"
#include "g1_speed.hh"
#include "map_man.hh"
#include "app/app.hh"
#include "editor/editor.hh"
#include "editor/e_res.hh"
#include "lisp/li_dialog.hh"
#include "saver.hh"

g1_object_type g1_sfx_obj_type;

class g1_sfx_obj_edit_class : public i4_color_window_class
{ 
public:
  w32 selected_objects[G1_MAX_OBJECTS];
  int t_sel;

  i4_str *filename;
  i4_text_input_class *ti_filename,
    *ti_max_volume,
    *ti_max_hearable,
    *ti_restart,
    *ti_random_restart,
    *ti_current_delay;

  enum { 
    SET_SONG_CANCEL,
    SET_SONG_OK,
    OK, CANCEL,
    BROWSE };


  g1_sfx_obj_edit_class(i4_graphical_style_class *style)

    : i4_color_window_class(400, 200, style->color_hint->neutral(), style)
  {
    t_sel=g1_get_map()->make_selected_objects_list(selected_objects, G1_MAX_OBJECTS);

    g1_sfx_obj_class *sfx;
    if (t_sel && (sfx=g1_sfx_obj_class::cast(g1_global_id.get(selected_objects[0]))))
    {
      if (sfx->filename)
        filename=new i4_str(*sfx->filename);
      else
        filename=new i4_str(i4_string_man.get(0));

      i4_create_dialog(g1_ges("sfx_obj_dialog"), this, style,
                       &ti_filename, filename,  this, BROWSE,
                       I4_SOUND_VOLUME_LEVELS-1, &ti_max_volume, sfx->max_volume,
                       &ti_max_hearable, sfx->max_hearable_distance,
                       G1_HZ, &ti_restart, sfx->restart_delay,
                       &ti_random_restart, sfx->random_restart_delay,
                       &ti_current_delay, sfx->current_delay,
                       this, OK,
                       this, CANCEL);
    }
  }

  void receive_event(i4_event *ev)
  {
    i4_color_window_class::receive_event(ev);

    if (ev->type()==i4_event::USER_MESSAGE)
    {
      CAST_PTR(uev, i4_user_message_event_class, ev);
      switch (uev->sub_type)
      {
        case BROWSE :
        {
          i4_create_file_open_dialog(style,
                                     g1_ges("get_song_title"),
                                     g1_ges("get_song_start_dir"),
                                     g1_ges("get_song_file_mask"),
                                     g1_ges("get_song_mask_name"),
                                     this,
                                     SET_SONG_OK,
                                     SET_SONG_CANCEL);
        } break;

        case SET_SONG_OK :
        {
          CAST_PTR(fev, i4_file_open_message_class, ev);
          ti_filename->change_text(*fev->filename);
          if (filename)
            delete filename;

          filename=i4_relative_path(*fev->filename);
        } break;

        case OK :
        {
          int v=ti_max_volume->get_number();
          if (v<0 || v>=I4_SOUND_VOLUME_LEVELS)
          {
            g1_editor_instance.create_modal(500, 100, "bad_volume_title");
            i4_create_dialog(g1_ges("bad_volume_dialog"), 
                             g1_editor_instance.modal_window.get(),
                             style, v, I4_SOUND_VOLUME_LEVELS,
                             &g1_editor_instance, G1_EDITOR_MODAL_BOX_CANCEL);
          }
          else
          {
            int max_hear=ti_max_hearable->get_number();
            int restart=ti_restart->get_number();
            int random_restart=ti_random_restart->get_number();
            int cur_delay=ti_current_delay->get_number();
            g1_sfx_obj_class *sfx;

            for (int i=0; i<t_sel; i++)
              if (g1_global_id.check_id(selected_objects[i]))
              {
                sfx=g1_sfx_obj_class::cast(g1_global_id.get(selected_objects[i]));
                sfx->set_filename(*ti_filename->get_edit_string());             
                sfx->max_volume=v;
                sfx->max_hearable_distance=max_hear;
                sfx->restart_delay=restart;
                sfx->random_restart_delay=random_restart;
                sfx->current_delay=cur_delay;
              }
            
          }

          li_call("object_changed");

        } break;

        case CANCEL :
          li_call("object_changed");
          break;

      }
    }

  }

};

class g1_sfx_obj_def_class : public g1_object_definer<g1_sfx_obj_class> 
{
public:
  g1_sfx_obj_def_class(char *_name) 
    : g1_object_definer<g1_sfx_obj_class>(_name,
                                          g1_object_definition_class::EDITOR_SELECTABLE
                                          ) {}

  virtual void init()
  {
    g1_sfx_obj_type = type;
    g1_object_definition_class::init();
  }

  i4_window_class *create_edit_dialog()
                                      
  {    
    return new g1_sfx_obj_edit_class(i4_current_app->get_style());
  }

} g1_sfx_obj_def("sfx_obj");


g1_sfx_obj_class::g1_sfx_obj_class(g1_object_type id, 
                                   g1_loader_class *fp)
  : g1_object_class(id, fp)
{
  draw_params.setup("speaker");

  if (fp && fp->check_version(DATA_VERSION))
  {
    filename=fp->read_counted_str();
    max_volume=fp->read_16();
    max_hearable_distance=fp->read_32();
    restart_delay=fp->read_32();
    random_restart_delay=fp->read_32();
    current_delay=fp->read_32();
    fp->end_version(I4_LF);
  }
  else
  {
    filename=0;
    max_volume=I4_SOUND_VOLUME_LEVELS-1;
    max_hearable_distance=10000;
    restart_delay=0;
    random_restart_delay=0;
    current_delay=0;
  }

  stream=0;
  dist_from_camera_sqrd=100000;
}

g1_sfx_obj_class::~g1_sfx_obj_class()
{
  if (stream)
    delete stream;

  if (filename)
    delete filename;
}


void g1_sfx_obj_class::save(g1_saver_class *fp)
{
  g1_object_class::save(fp);

  fp->start_version(DATA_VERSION);
  
  if (filename)
    fp->write_counted_str(*filename);
  else fp->write_16(0);

  fp->write_16(max_volume);
  fp->write_32(max_hearable_distance);
  fp->write_32(restart_delay);
  fp->write_32(random_restart_delay);
  fp->write_32(current_delay);

  fp->end_version();
}


void g1_sfx_obj_class::draw(g1_draw_context_class *context)
{
  g1_editor_model_draw(this, draw_params, context);
}

void g1_sfx_obj_class::think()
{
}

// adds self to sound_manager
i4_bool g1_sfx_obj_class::occupy_location()
{
  g1_sound_man.add_sfx_to_list(this);
  return g1_object_class::occupy_location();
}

// remove self from sound_manager
void g1_sfx_obj_class::unoccupy_location()
{
  g1_object_class::unoccupy_location();
  g1_sound_man.remove_sfx_from_list(this);
}


void g1_sfx_obj_class::set_filename(const i4_const_str &fname)
{
  if (filename)
    delete filename;

  if (fname.null())
    filename=0;
  else filename=new i4_str(fname);

}

i4_str *g1_sfx_obj_class::get_context_string()
{
  return i4gets("sfx_obj_fmt").sprintf(120,filename, max_volume, 
                                   max_hearable_distance,
                                   current_delay);
}
