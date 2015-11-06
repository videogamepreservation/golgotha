/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "lisp/li_init.hh"
#include "app/app.hh"
#include "file/get_filename.hh"
#include "m1_info.hh"
#include "max_object.hh"
#include "loaders/dir_save.hh"
#include "render.hh"


i4_str *m1_get_dir()
{
  if (m1_info.obj && m1_info.models.size()>m1_info.current_model)
  {
    i4_filename_struct fn;
    i4_split_path(m1_info.current_filename(), fn);
    if (fn.path)
      return new i4_str(fn.path);
    else 
      return new i4_str(".");
  }
  else return new i4_str(".");
}

li_object *m1_save(li_object *o, li_environment *env)
{
  if (m1_info.obj && m1_info.models.size()>m1_info.current_model)
  {
    i4_file_class *fp=i4_open(m1_info.current_filename(), I4_WRITE);
    if (fp)
    {
      m1_info.obj->update(0);
      i4_saver_class *sfp=new i4_saver_class(fp);
      m1_info.obj->save(sfp);
      sfp->begin_data_write();
      m1_info.obj->save(sfp);        
      delete fp;
    }        
  }
  return 0;
}



class m1_saveas_handler_class : public i4_event_handler_class
{
public:
  char *name() { return "saveas"; }
  void receive_event(i4_event *ev)
  {    
    CAST_PTR(f, i4_file_open_message_class, ev);
    if (f->sub_type)
    {
      m1_info.set_current_filename(*f->filename);
      m1_save(0,0);
    }
  }

} m1_saveas_handler;


li_object *m1_saveas(li_object *o, li_environment *env)
{
  if (m1_info.obj && m1_info.models.size()>m1_info.current_model)
  {
    i4_str *sdir=m1_get_dir();
    i4_create_file_save_dialog(i4_current_app->get_style(),
                               m1_info.current_filename(),
                               "Save As..",
                               *sdir,
                               "*.gmod",
                               "Golgotha MODel",
                               &m1_saveas_handler,
                               1,0);
    delete sdir;
  }
  return 0;
}



class m1_open_handler_class : public i4_event_handler_class
{
public:
  char *name() { return "open"; }
  void receive_event(i4_event *ev)
  {    
    CAST_PTR(f, i4_file_open_message_class, ev);
    if (f->sub_type)
    {
      m1_info.set_current_filename(*f->filename);
      m1_render_window->set_object(*m1_info.models[m1_info.current_model]);
    }
  }

} m1_open_handler;


li_object *m1_open(li_object *o, li_environment *env)
{
  if (o)
  {
    char buff[512];

    strcpy(buff, li_get_string(li_eval(li_car(o,env), env),env));
    
    m1_info.set_current_filename(buff);
    m1_render_window->set_object(buff);
  }
  else
  {
    i4_str *sdir=m1_get_dir();
    i4_create_file_open_dialog(i4_current_app->get_style(),
                               "Open..",
                               *sdir,
                               "*.gmod",
                               "Golgotha MODel",
                               &m1_open_handler,
                               1,0);
    delete sdir;
  }
  return 0;
}

li_automatic_add_function(m1_saveas, "saveas_model");
li_automatic_add_function(m1_save, "save_model");
li_automatic_add_function(m1_open, "open_model");
