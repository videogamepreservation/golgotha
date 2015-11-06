/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "m1_info.hh"
#include "lisp/li_types.hh"
#include "file/file.hh"
#include "make_tlist.hh"
#include "gui/text_scroll.hh"
#include "r1_api.hh"
#include "tmanage.hh"
#include "max_object.hh"
#include "tupdate.hh"
#include "st_edit.hh"
#include "render.hh"

m1_info_struct m1_info;
extern i4_text_scroll_window_class *m1_warn_window;

const i4_const_str m1_info_struct::current_filename()
{
  if (m1_info.models.size()==0)
    return i4_const_str("");
  else
    return *m1_info.models[m1_info.current_model];
}

void m1_info_struct::set_current_filename(const i4_const_str &str)
{
  if (m1_info.models.size()==0)
  {
    m1_info.models.add();
    m1_info.current_model=0;
  }
  else
    delete m1_info.models[m1_info.current_model];

  m1_info.models[m1_info.current_model]=new i4_str(str);
}


void m1_info_struct::get_texture_list(i4_array<i4_str *> &array, i4_bool for_all)
{
  int i;

  if (for_all)
  {
    // add any textures from res files
    if (res_files.size())
    {
      i4_file_class *fp[500];

    
      for (i=0; i<res_files.size(); i++)
        fp[i]=i4_open(*res_files[i]);

      int tmp_int;
      i4_array<i4_str *> tmp_array(0,32);
      g1_get_load_info(fp, res_files.size(), array, tmp_array, tmp_int, i4_T);
      
      for (i=0; i<tmp_array.size(); i++)
        delete tmp_array[i];
    }

    for (i=0; i<textures.size(); i++)
      array.add(new i4_str(*textures[i]));

    for (i=0; i<models.size(); i++)
      g1_make_texture_list_for_model(*models[i], array);
  }
  else
    g1_make_texture_list_for_model(*models[current_model], array);
}


void m1_info_struct::init(int argc, i4_const_str *argv)
{
  for (int i=1; i<argc; i++)
  {
    if (argv[i]=="-update")
      update_mode=1;
    else
    {
      i4_filename_struct fn;
      i4_split_path(argv[i],fn);

      if (fn.extension)
      {
        if (strcmp(fn.extension,"scm")==0)
          res_files.add(new i4_str(argv[i]));
        else if (strcmp(fn.extension,"tga")==0)
          textures.add(new i4_str(argv[i]));
        else if (strcmp(fn.extension, "gmod")==0 || strcmp(fn.extension,"crk")==0)
          models.add(new i4_str(argv[i]));
      }
    }
  }
}

void m1_info_struct::uninit()
{
  int i;
  for (i=0; i<res_files.size(); i++) delete res_files[i];
  res_files.uninit();

  for (i=0; i<textures.size(); i++) delete textures[i];
  textures.uninit();

  for (i=0; i<models.size(); i++) delete models[i];
  models.uninit();
}


void m1_info_struct::texture_list_changed()
{
  if (update_mode)
    return ;

  r1_texture_manager_class *tman=r_api->get_tmanager();
  
  if (!obj)
    return;

  if (tman)
  {
    int i;
    tman->reset();
    textures_loaded=i4_F;

    i4_status_class *stat=i4_create_status("Updating textures");

    int nq=m1_info.obj->num_quad;
    for (i=0; i<nq; i++)
      m1_update_texture(*m1_info.obj->texture_names[i], i4_F, stat, 
                        i/(float)nq, (i+1)/(float)nq);

    if (stat) delete stat;
    

    for (i=0; i<m1_info.obj->num_quad; i++)    
      m1_info.obj->quad[i].material_ref=tman->register_texture(*m1_info.obj->texture_names[i],
                                                               m1_info.current_filename());

    m1_st_edit->edit_poly_changed();
    m1_render_window->request_redraw(i4_F);
  }
}
